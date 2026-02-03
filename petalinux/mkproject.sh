#!/bin/sh
BASE_DIR="$(pwd)"
PROJECT_NAME="$1"
PROJECT_TEMPLATE="${2:-zynq}"
HW_DESC_ARG="$3"
HW_DESC_DIR=""
HW_MOUNT_HOST=""
HW_MOUNT_CONT="/hw-description"

if [ -z "$PROJECT_NAME" ]; then
    echo "need a project name"
    echo "mkproject.sh <project name> [template] [hw-description dir|xsa path]"
    exit 1
fi

if [ -n "${HW_DESC_ARG}" ]; then
    if command -v realpath >/dev/null 2>&1; then
        HW_DESC_HOST_PATH="$(realpath "${HW_DESC_ARG}" 2>/dev/null || true)"
    fi
    if [ -z "${HW_DESC_HOST_PATH}" ]; then
        case "${HW_DESC_ARG}" in
            /*) HW_DESC_HOST_PATH="${HW_DESC_ARG}" ;;
            *) HW_DESC_HOST_PATH="${BASE_DIR}/${HW_DESC_ARG}" ;;
        esac
    fi

    if [ -f "${HW_DESC_HOST_PATH}" ]; then
        HW_DESC_HOST_DIR="$(dirname "${HW_DESC_HOST_PATH}")"
    elif [ -d "${HW_DESC_HOST_PATH}" ]; then
        HW_DESC_HOST_DIR="${HW_DESC_HOST_PATH}"
    else
        echo "[ERROR] hw-description path not found: ${HW_DESC_ARG}"
        exit 1
    fi

    case "${HW_DESC_HOST_DIR}" in
        "${BASE_DIR}")
            HW_DESC_DIR="/workspace"
            ;;
        "${BASE_DIR}"/*)
            HW_DESC_DIR="/workspace/${HW_DESC_HOST_DIR#${BASE_DIR}/}"
            ;;
        *)
            HW_MOUNT_HOST="${HW_DESC_HOST_DIR}"
            HW_DESC_DIR="${HW_MOUNT_CONT}"
            ;;
    esac
fi

PROJECT_DIR="${BASE_DIR}/${PROJECT_NAME}"
echo "${PROJECT_DIR}"

if [ -n "${HW_MOUNT_HOST}" ]; then
  docker run --rm -it \
    --privileged --security-opt apparmor=unconfined \
    -e PROJECT_NAME="${PROJECT_NAME}" \
    -e PROJECT_TEMPLATE="${PROJECT_TEMPLATE}" \
    -e HW_DESC_DIR="${HW_DESC_DIR}" \
    -v "${BASE_DIR}:/workspace" \
    -v "${HW_MOUNT_HOST}:${HW_MOUNT_CONT}:ro" \
    petalinux:2025.2 \
    bash -lc '
      source ~/petalinux/settings.sh
      cd /workspace || exit 1

      if [ ! -d "${PROJECT_NAME}/project-spec" ]; then
        if [ -d "${PROJECT_NAME}" ] && [ "$(ls -A "${PROJECT_NAME}" 2>/dev/null)" ]; then
          echo "[ERROR] ${PROJECT_NAME} exists but is not a PetaLinux project (project-spec/ not found)."
          echo "[ERROR] Move or remove the directory, then run again."
          exit 1
        fi
        rm -rf "${PROJECT_NAME}"
        petalinux-create project --template "${PROJECT_TEMPLATE}" --name "${PROJECT_NAME}" || exit 1
      fi

      cd "${PROJECT_NAME}" || exit 1

      if ! command -v nslookup >/dev/null 2>&1; then
        echo "[ERROR] nslookup not found in container."
        echo "[ERROR] Rebuild petalinux:2025.2 image after installing dnsutils."
        exit 1
      fi

      if [ -n "${HW_DESC_DIR}" ]; then
        petalinux-config --get-hw-description="${HW_DESC_DIR}" || exit 1
      fi

      if [ -z "${HW_DESC_DIR}" ] && [ -z "$(ls -A project-spec/hw-description 2>/dev/null)" ]; then
        echo "[ERROR] No XSA/DTS found in $(pwd)/project-spec/hw-description"
        echo "[ERROR] Re-run with [hw-description dir], e.g. ./mkproject.sh ${PROJECT_NAME} ${PROJECT_TEMPLATE} ../vivado"
        exit 1
      fi

      petalinux-build
    '
else
  docker run --rm -it \
    --privileged --security-opt apparmor=unconfined \
    -e PROJECT_NAME="${PROJECT_NAME}" \
    -e PROJECT_TEMPLATE="${PROJECT_TEMPLATE}" \
    -e HW_DESC_DIR="${HW_DESC_DIR}" \
    -v "${BASE_DIR}:/workspace" \
    petalinux:2025.2 \
    bash -lc '
      source ~/petalinux/settings.sh
      cd /workspace || exit 1

      if [ ! -d "${PROJECT_NAME}/project-spec" ]; then
        if [ -d "${PROJECT_NAME}" ] && [ "$(ls -A "${PROJECT_NAME}" 2>/dev/null)" ]; then
          echo "[ERROR] ${PROJECT_NAME} exists but is not a PetaLinux project (project-spec/ not found)."
          echo "[ERROR] Move or remove the directory, then run again."
          exit 1
        fi
        rm -rf "${PROJECT_NAME}"
        petalinux-create project --template "${PROJECT_TEMPLATE}" --name "${PROJECT_NAME}" || exit 1
      fi

      cd "${PROJECT_NAME}" || exit 1

      if ! command -v nslookup >/dev/null 2>&1; then
        echo "[ERROR] nslookup not found in container."
        echo "[ERROR] Rebuild petalinux:2025.2 image after installing dnsutils."
        exit 1
      fi

      if [ -n "${HW_DESC_DIR}" ]; then
        petalinux-config --get-hw-description="${HW_DESC_DIR}" || exit 1
      fi

      if [ -z "${HW_DESC_DIR}" ] && [ -z "$(ls -A project-spec/hw-description 2>/dev/null)" ]; then
        echo "[ERROR] No XSA/DTS found in $(pwd)/project-spec/hw-description"
        echo "[ERROR] Re-run with [hw-description dir], e.g. ./mkproject.sh ${PROJECT_NAME} ${PROJECT_TEMPLATE} ../vivado"
        exit 1
      fi

      petalinux-build
    '
fi
