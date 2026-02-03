#!/bin/sh
set -eu

BASE_DIR="$(pwd)"
PROJECT_NAME="${1:-}"
shift || true

if [ -z "${PROJECT_NAME}" ] || [ "$#" -lt 1 ]; then
  echo "Usage: ./addlayer.sh <project_name> <layer_path> [layer_path ...]"
  exit 1
fi

PROJECT_DIR="${BASE_DIR}/${PROJECT_NAME}"
if [ ! -d "${PROJECT_DIR}/project-spec" ]; then
  echo "[ERROR] ${PROJECT_DIR} is not a PetaLinux project."
  exit 1
fi

layer_count=0
LAYER_LIST=""
EXT_MOUNTS_FILE="$(mktemp)"
trap 'rm -f "${EXT_MOUNTS_FILE}"' EXIT INT TERM

for layer_arg in "$@"; do
  if command -v realpath >/dev/null 2>&1; then
    layer_host="$(realpath "${layer_arg}" 2>/dev/null || true)"
  else
    layer_host=""
  fi

  if [ -z "${layer_host}" ]; then
    case "${layer_arg}" in
      /*) layer_host="${layer_arg}" ;;
      *) layer_host="${BASE_DIR}/${layer_arg}" ;;
    esac
  fi

  if [ ! -d "${layer_host}" ]; then
    echo "[ERROR] layer directory not found: ${layer_arg}"
    exit 1
  fi

  if [ ! -f "${layer_host}/conf/layer.conf" ]; then
    echo "[ERROR] not a Yocto layer (conf/layer.conf missing): ${layer_host}"
    exit 1
  fi

  case "${layer_host}" in
    "${BASE_DIR}")
      layer_cont="/workspace"
      ;;
    "${BASE_DIR}"/*)
      layer_cont="/workspace/${layer_host#${BASE_DIR}/}"
      ;;
    *)
      layer_count=$((layer_count + 1))
      layer_cont="/extlayer${layer_count}"
      printf '%s\t%s\n' "${layer_host}" "${layer_cont}" >> "${EXT_MOUNTS_FILE}"
      ;;
  esac

  if [ -z "${LAYER_LIST}" ]; then
    LAYER_LIST="${layer_cont}"
  else
    LAYER_LIST="${LAYER_LIST} ${layer_cont}"
  fi
done

set -- docker run --rm -it \
  --privileged --security-opt apparmor=unconfined \
  -e "PROJECT_NAME=${PROJECT_NAME}" \
  -e "LAYER_LIST=${LAYER_LIST}" \
  -v "${BASE_DIR}:/workspace"

while IFS="$(printf '\t')" read -r host cont; do
  [ -n "${host}" ] || continue
  set -- "$@" -v "${host}:${cont}:ro"
done < "${EXT_MOUNTS_FILE}"

set -- "$@" petalinux:2025.2 bash -lc '
  source ~/petalinux/settings.sh
  cd "/workspace/${PROJECT_NAME}" || exit 1
  export PROOT="$(pwd)"
  export BB_ENV_PASSTHROUGH_ADDITIONS="${BB_ENV_PASSTHROUGH_ADDITIONS:-} PROOT"
  source components/yocto/layers/poky/oe-init-build-env build >/dev/null || exit 1

  for layer in ${LAYER_LIST}; do
    if grep -Fq "${layer}" conf/bblayers.conf 2>/dev/null; then
      echo "[INFO] layer already added: ${layer}"
    else
      echo "[INFO] adding layer: ${layer}"
      bitbake-layers add-layer "${layer}" || exit 1
    fi
  done

  echo "[INFO] current layers:"
  bitbake-layers show-layers
'

"$@"
