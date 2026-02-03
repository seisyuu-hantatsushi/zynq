#!/bin/sh
set -eu

BASE_DIR="$(pwd)"
PROJECT_NAME="${1:-}"
TARGET="${2:-project}"

if [ -z "${PROJECT_NAME}" ]; then
  echo "Usage: ./menuconfig.sh <project_name> [project|rootfs|kernel|u-boot|busybox]"
  exit 1
fi

PROJECT_DIR="${BASE_DIR}/${PROJECT_NAME}"
if [ ! -d "${PROJECT_DIR}/project-spec" ]; then
  echo "[ERROR] ${PROJECT_DIR} is not a PetaLinux project."
  exit 1
fi

case "${TARGET}" in
  project)
    CONFIG_CMD="petalinux-config"
    ;;
  rootfs|kernel|u-boot|busybox)
    CONFIG_CMD="petalinux-config -c ${TARGET}"
    ;;
  *)
    echo "[ERROR] unsupported target: ${TARGET}"
    echo "Supported: project, rootfs, kernel, u-boot, busybox"
    exit 1
    ;;
esac

docker run --rm -it \
  --privileged --security-opt apparmor=unconfined \
  -e PROJECT_NAME="${PROJECT_NAME}" \
  -e CONFIG_CMD="${CONFIG_CMD}" \
  -e OE_TERMINAL="tmux" \
  -e BB_ENV_PASSTHROUGH_ADDITIONS="OE_TERMINAL" \
  -e TERM="${TERM:-xterm}" \
  -v "${BASE_DIR}:/workspace" \
  petalinux:2025.2 \
  bash -lc '
    source ~/petalinux/settings.sh
    cd "/workspace/${PROJECT_NAME}" || exit 1
    if ! command -v tmux >/dev/null 2>&1; then
      echo "[ERROR] tmux not found in container. Rebuild petalinux:2025.2 image."
      exit 1
    fi
    ${CONFIG_CMD}
  '
