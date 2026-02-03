#!/bin/sh
set -eu

# Add optional tool paths here (host-specific).
EXTRA_PATHS="/home/kaz/work/buildroot/buildroot-2025.11/output/host/bin"
PATH="${PATH}:${EXTRA_PATHS}"
export PATH

DEVICE="${1:-}"
PROJECT_NAME="${2:-}"
ROOT_DEV="${3:-/dev/mmcblk0p2}"

if [ -z "${DEVICE}" ] || [ -z "${PROJECT_NAME}" ]; then
  echo "Usage: sudo ./writeimage.sh <sd_device> <project_name> [root device]"
  echo "Example: sudo ./writeimage.sh /dev/sdb minimam_platform /dev/mmcblk0p2"
  exit 1
fi

if [ "$(id -u)" -ne 0 ]; then
  echo "[ERROR] Run as root (use sudo)."
  exit 1
fi

if [ ! -b "${DEVICE}" ]; then
  echo "[ERROR] Block device not found: ${DEVICE}"
  exit 1
fi

partdev() {
  case "$1" in
    *[0-9]) echo "${1}p${2}" ;;
    *) echo "${1}${2}" ;;
  esac
}

unmount_device_partitions() {
  i=0
  while [ "${i}" -lt 5 ]; do
    for part in $(ls "${DEVICE}"* 2>/dev/null | grep -E "^${DEVICE}(p)?[0-9]+$" || true); do
      umount "${part}" 2>/dev/null || true
      if command -v udisksctl >/dev/null 2>&1; then
        udisksctl unmount -b "${part}" >/dev/null 2>&1 || true
      fi
    done
    sleep 1
    i=$((i + 1))
  done
}

BASE_DIR="$(pwd)"
IMG_DIR="${BASE_DIR}/${PROJECT_NAME}/images/linux"
BOOT_BIN="${IMG_DIR}/BOOT.BIN"
IMAGE_UB="${IMG_DIR}/image.ub"
ROOTFS_EXT4="${IMG_DIR}/rootfs.ext4"
BOOT_CMD="${IMG_DIR}/boot.cmd"
BOOT_SCR="${IMG_DIR}/boot.scr"

for f in "${BOOT_BIN}" "${IMAGE_UB}" "${ROOTFS_EXT4}"; do
  if [ ! -f "${f}" ]; then
    echo "[ERROR] Missing file: ${f}"
    exit 1
  fi
done

echo "[INFO] Generating boot.scr (root=${ROOT_DEV})"
cat > "${BOOT_CMD}" <<EOF
echo "Boot from SD with image.ub (FIT)"
setenv bootargs "console=ttyPS0,115200 earlycon root=${ROOT_DEV} rw rootwait rootfstype=ext4"
if fatload mmc 0:1 \${kernel_addr_r} image.ub; then
  echo "Booting image.ub from mmc0:1"
  bootm \${kernel_addr_r}
fi
if fatload mmc 0:1 \${kernel_addr_r} boot/image.ub; then
  echo "Booting boot/image.ub from mmc0:1"
  bootm \${kernel_addr_r}
fi
if fatload mmc 1:1 \${kernel_addr_r} image.ub; then
  echo "Booting image.ub from mmc1:1"
  bootm \${kernel_addr_r}
fi
if fatload mmc 1:1 \${kernel_addr_r} boot/image.ub; then
  echo "Booting boot/image.ub from mmc1:1"
  bootm \${kernel_addr_r}
fi
echo "No bootable image.ub found on SD"
EOF

if command -v mkimage >/dev/null 2>&1; then
  mkimage -A arm -T script -C none -n "SD boot script" -d "${BOOT_CMD}" "${BOOT_SCR}"
elif command -v docker >/dev/null 2>&1; then
  docker run --rm \
    -v "${BASE_DIR}:/workspace" \
    petalinux:2025.2 \
    bash -lc "command -v mkimage >/dev/null 2>&1 || { echo '[ERROR] mkimage not found in petalinux:2025.2 image. Rebuild image with updated Dockerfile.'; exit 127; }; mkimage -A arm -T script -C none -n 'SD boot script' -d '/workspace/${PROJECT_NAME}/images/linux/boot.cmd' '/workspace/${PROJECT_NAME}/images/linux/boot.scr'"
else
  echo "[ERROR] mkimage not found on host."
  echo "[ERROR] Install u-boot-tools or docker, or generate ${BOOT_SCR} manually."
  exit 1
fi

WORK="/tmp/writeimage-$$"
GEN_ROOT="${WORK}/genimage-root"
GEN_INPUT="${WORK}/genimage-input"
GEN_TMP="${WORK}/genimage-tmp"
GEN_OUT="${WORK}/genimage-out"
GEN_CFG="${WORK}/genimage.cfg"
SD_IMAGE="${GEN_OUT}/sdcard.img"
mkdir -p "${GEN_ROOT}/boot" "${GEN_INPUT}" "${GEN_TMP}" "${GEN_OUT}"
trap 'rm -rf "${WORK}"' EXIT INT TERM

cp -f "${BOOT_BIN}" "${GEN_ROOT}/BOOT.BIN"
cp -f "${IMAGE_UB}" "${GEN_ROOT}/image.ub"
cp -f "${BOOT_SCR}" "${GEN_ROOT}/boot.scr"
cp -f "${BOOT_SCR}" "${GEN_ROOT}/boot/boot.scr"
cp -f "${ROOTFS_EXT4}" "${GEN_INPUT}/rootfs.ext4"

cat > "${GEN_CFG}" <<'EOF'
image boot.vfat {
  vfat {
  }
  size = 512M
}

image sdcard.img {
  hdimage {
    partition-table-type = "dos"
  }
  partition boot {
    partition-type = 0x0C
    bootable = "true"
    image = "boot.vfat"
  }
  partition rootfs {
    partition-type = 0x83
    image = "rootfs.ext4"
  }
}
EOF

echo "[INFO] Generating sdcard image with genimage"
if command -v genimage >/dev/null 2>&1; then
  genimage \
    --config "${GEN_CFG}" \
    --rootpath "${GEN_ROOT}" \
    --tmppath "${GEN_TMP}" \
    --inputpath "${GEN_INPUT}" \
    --outputpath "${GEN_OUT}"
elif command -v docker >/dev/null 2>&1; then
  docker run --rm \
    -v "${WORK}:/work" \
    petalinux:2025.2 \
    bash -lc "command -v genimage >/dev/null 2>&1 || { echo '[ERROR] genimage not found in petalinux:2025.2 image. Rebuild image with updated Dockerfile.'; exit 127; }; genimage --config /work/genimage.cfg --rootpath /work/genimage-root --tmppath /work/genimage-tmp --inputpath /work/genimage-input --outputpath /work/genimage-out"
else
  echo "[ERROR] genimage not found on host and docker is unavailable."
  exit 1
fi

if [ ! -f "${SD_IMAGE}" ]; then
  echo "[ERROR] sd image was not generated: ${SD_IMAGE}"
  exit 1
fi

echo "[INFO] Unmounting existing partitions on ${DEVICE}"
unmount_device_partitions

echo "[INFO] Clearing old signatures on ${DEVICE}"
wipefs -a "${DEVICE}" || true

echo "[INFO] Writing ${SD_IMAGE} to ${DEVICE}"
dd if="${SD_IMAGE}" of="${DEVICE}" bs=4M conv=fsync status=progress
sync
blockdev --rereadpt "${DEVICE}" 2>/dev/null || true
partprobe "${DEVICE}" 2>/dev/null || true
udevadm settle 2>/dev/null || true
unmount_device_partitions

P2="$(partdev "${DEVICE}" 2)"
echo "[INFO] Expanding partition 2 to remaining capacity"
parted -s "${DEVICE}" resizepart 2 100%
blockdev --rereadpt "${DEVICE}" 2>/dev/null || true
partprobe "${DEVICE}" 2>/dev/null || true
udevadm settle 2>/dev/null || true
sleep 1
if [ -b "${P2}" ]; then
  unmount_device_partitions
  e2fsck -fy "${P2}"
  resize2fs "${P2}"
  e2fsck -fy "${P2}"
  CHECK_MNT="${WORK}/check"
  mkdir -p "${CHECK_MNT}"
  mount -o ro "${P2}" "${CHECK_MNT}"
  umount "${CHECK_MNT}"
fi

echo "[INFO] Done."
echo "[INFO] SD image written with genimage (p1: FAT32 boot, p2: ext4 rootfs)."
echo "[INFO] boot.scr root= is set to ${ROOT_DEV}."
