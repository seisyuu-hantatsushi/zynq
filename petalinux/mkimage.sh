#!/bin/sh
BASE_DIR="$(pwd)"
PROJECT_NAME="$1"
FSBL_ARG="$2"
BIT_ARG="$3"
ROOTFS_DEV="${4:-/dev/mmcblk0p2}"
ROOT_PASSWORD="${5:-rk-zynq}"

if [ -z "${PROJECT_NAME}" ]; then
    echo "need a project name"
    echo "mkimage.sh <project name> [fsbl elf path] [bit path] [root device] [root password]"
    exit 1
fi

PROJECT_DIR="${BASE_DIR}/${PROJECT_NAME}"
if [ ! -d "${PROJECT_DIR}/project-spec" ]; then
    echo "[ERROR] ${PROJECT_DIR} is not a PetaLinux project."
    exit 1
fi

BSP_CONF_HOST="${PROJECT_DIR}/project-spec/meta-user/conf/petalinuxbsp.conf"
ROOT_PASS_HASH="$(python3 - <<PY
import crypt
print(crypt.crypt("${ROOT_PASSWORD}", crypt.mksalt(crypt.METHOD_SHA512)))
PY
)"
ROOT_PASS_HASH_ESCAPED="$(printf "%s" "${ROOT_PASS_HASH}" | sed 's/[$]/\\$/g')"
[ -f "${BSP_CONF_HOST}" ] || touch "${BSP_CONF_HOST}"
sed -i "/^KUIPER_COMPAT_USERADD[[:space:]]*=.*/d" "${BSP_CONF_HOST}"
sed -i "/^KUIPER_COMPAT_SUDOERS[[:space:]]*=.*/d" "${BSP_CONF_HOST}"
sed -i "/^EXTRA_USERS_PARAMS:append[[:space:]]*=.*usermod -[pP].*root;.*/d" "${BSP_CONF_HOST}"
{
    echo 'KUIPER_COMPAT_USERADD = ""'
    echo 'KUIPER_COMPAT_SUDOERS = ""'
    echo "EXTRA_USERS_PARAMS:append = \" usermod -p '${ROOT_PASS_HASH_ESCAPED}' root;\""
} >> "${BSP_CONF_HOST}"

to_container_path() {
    in_path="$1"
    if [ -z "${in_path}" ]; then
        echo ""
        return 0
    fi

    if command -v realpath >/dev/null 2>&1; then
        host_path="$(realpath "${in_path}" 2>/dev/null || true)"
    fi
    if [ -z "${host_path}" ]; then
        case "${in_path}" in
            /*) host_path="${in_path}" ;;
            *) host_path="${BASE_DIR}/${in_path}" ;;
        esac
    fi

    if [ ! -f "${host_path}" ]; then
        echo "[ERROR] file not found: ${in_path}" >&2
        return 1
    fi

    case "${host_path}" in
        "${BASE_DIR}"/*)
            echo "/workspace/${host_path#${BASE_DIR}/}"
            return 0
            ;;
        *)
            echo "[ERROR] path must be under ${BASE_DIR}: ${host_path}" >&2
            return 1
            ;;
    esac
}

if [ -n "${FSBL_ARG}" ]; then
    FSBL_PATH="$(to_container_path "${FSBL_ARG}")" || exit 1
else
    FSBL_PATH="/workspace/${PROJECT_NAME}/images/linux/zynq_fsbl.elf"
fi

if [ -n "${BIT_ARG}" ]; then
    BIT_PATH="$(to_container_path "${BIT_ARG}")" || exit 1
else
    BIT_PATH="/workspace/${PROJECT_NAME}/images/linux/system.bit"
fi

docker run --rm -it \
  --privileged --security-opt apparmor=unconfined \
  -e PROJECT_NAME="${PROJECT_NAME}" \
  -e FSBL_PATH="${FSBL_PATH}" \
  -e BIT_PATH="${BIT_PATH}" \
  -e ROOTFS_DEV="${ROOTFS_DEV}" \
  -e ROOT_PASSWORD="${ROOT_PASSWORD}" \
  -v "${BASE_DIR}:/workspace" \
  petalinux:2025.2 \
  bash -lc '
    source ~/petalinux/settings.sh
    cd "/workspace/${PROJECT_NAME}" || exit 1

    if ! command -v nslookup >/dev/null 2>&1; then
      echo "[ERROR] nslookup not found in container."
      echo "[ERROR] Rebuild petalinux:2025.2 image after installing dnsutils."
      exit 1
    fi

    BOOTARGS="console=ttyPS0,115200 earlycon root=${ROOTFS_DEV} rw rootwait rootfstype=ext4"
    CONFIG_FILE="project-spec/configs/config"
    if [ -f "${CONFIG_FILE}" ]; then
      sed -i "s|^CONFIG_SUBSYSTEM_BOOTARGS_AUTO=.*|CONFIG_SUBSYSTEM_BOOTARGS_AUTO=n|" "${CONFIG_FILE}"
      sed -i "s|^CONFIG_SUBSYSTEM_BOOTARGS_GENERATED=.*|CONFIG_SUBSYSTEM_BOOTARGS_GENERATED=\"${BOOTARGS}\"|" "${CONFIG_FILE}"

      petalinux-config --silentconfig || exit 1
      echo "[INFO] default bootargs root set to ${ROOTFS_DEV}"
      echo "[INFO] root password updated"
    fi

    petalinux-build || exit 1

    if [ ! -f "${FSBL_PATH}" ]; then
      echo "[ERROR] FSBL not found: ${FSBL_PATH}"
      exit 1
    fi

    BOOT_ARGS=(--boot --fsbl "${FSBL_PATH}" --u-boot --force)
    if [ -f "${BIT_PATH}" ]; then
      BOOT_ARGS+=(--fpga "${BIT_PATH}")
    else
      echo "[WARN] bitstream not found: ${BIT_PATH}"
      echo "[WARN] creating BOOT.BIN without --fpga"
    fi

    petalinux-package "${BOOT_ARGS[@]}" || exit 1
    echo "[INFO] generated: $(pwd)/images/linux/BOOT.BIN"
    echo "[INFO] generated: $(pwd)/images/linux/image.ub"
  '
