# HowToUse

このドキュメントは、以下 4 つのスクリプトの使い方をまとめたものです。

- `mkproject.sh`
- `mkimage.sh`
- `menuconfig.sh`
- `addlayer.sh`
- `writeimage.sh`

前提:

- カレントディレクトリ: `petalinux` ディレクトリ
- Docker イメージ: `petalinux:2025.2`
- `Dockerfile` を更新した場合は再ビルドしてください

```bash
docker build -t petalinux:2025.2 .
```

---

## 1) mkproject.sh

PetaLinux プロジェクト作成 + 必要に応じて HW 記述読み込み + `petalinux-build` を実行します。

## 使い方

```bash
bash mkproject.sh <project_name> [template] [hw-description dir|xsa path]
```

- `project_name`: 必須
- `template`: 省略時 `zynq`
- `hw-description`: 省略可
  - ディレクトリ指定でも XSA ファイル指定でも可

## 例

```bash
# 最小（既存 hw-description がある場合）
bash mkproject.sh minimam_platform

# XSA を指定して作成/設定/ビルド
bash mkproject.sh minimam_platform zynq ../minimam_platform/HW/minimam_platform/minimam_platform.xsa
```

---

## 2) mkimage.sh

`petalinux-build` 実行後、`BOOT.BIN` と `image.ub` を生成します。  
同時に、デフォルト rootfs デバイスを `mmcblk0p2` に設定します。

## 使い方

```bash
bash mkimage.sh <project_name> [fsbl elf path] [bit path] [root device] [root password]
```

- `project_name`: 必須
- `fsbl elf path`: 省略時 `<project>/images/linux/zynq_fsbl.elf`
- `bit path`: 省略時 `<project>/images/linux/system.bit`
- `root device`: 省略時 `/dev/mmcblk0p2`
- `root password`: 省略時 `rk-zynq`

## 例

```bash
# デフォルト値で実行
bash mkimage.sh minimam_platform

# root device を変更
bash mkimage.sh minimam_platform "" "" /dev/mmcblk1p2

# root password を変更
bash mkimage.sh minimam_platform "" "" /dev/mmcblk0p2 my-secret-pass
```

生成物:

- `<project>/images/linux/BOOT.BIN`
- `<project>/images/linux/image.ub`

---

## 3) menuconfig.sh

PetaLinux の menuconfig をコンテナ内で起動します。

## 使い方

```bash
bash menuconfig.sh <project_name> [project|rootfs|kernel|u-boot|busybox]
```

- 第2引数省略時は `project`

## 例

```bash
# project 設定
bash menuconfig.sh minimam_platform

# rootfs 設定
bash menuconfig.sh minimam_platform rootfs

# kernel 設定
bash menuconfig.sh minimam_platform kernel
```

---

## 4) addlayer.sh

任意の Yocto layer をプロジェクトへ追加します。

## 使い方

```bash
bash addlayer.sh <project_name> <layer_path> [layer_path ...]
```

- `<layer_path>/conf/layer.conf` が存在する必要があります
- 既に追加済みのレイヤはスキップされます

## 例

```bash
bash addlayer.sh minimam_platform \
  ../meta-openembedded/meta-oe \
  ../meta-openembedded/meta-python \
  ../meta-openembedded/meta-networking \
  ../meta-adi
```

---

## 推奨フロー

```bash
# 1) プロジェクト作成 + HW取り込み + build
bash mkproject.sh minimam_platform zynq ../path/to/design.xsa

# 2) 追加レイヤ
bash addlayer.sh minimam_platform ../meta-openembedded/meta-oe ../meta-adi

# 3) 必要な設定
bash menuconfig.sh minimam_platform rootfs

# 4) 起動イメージ作成
bash mkimage.sh minimam_platform

# 5) SDカードへ書き込み（デバイス指定に注意）
sudo bash writeimage.sh /dev/sdX minimam_platform
```

---

## 5) writeimage.sh

`genimage` で SD カード用のディスクイメージを生成して書き込みます。

- p1 (FAT32): `BOOT.BIN`, `image.ub`, `boot.scr`
- p2 (ext4): `rootfs.ext4`

## 使い方

```bash
sudo bash writeimage.sh <sd_device> <project_name> [root device]
```

- root 権限が必要です
- 指定デバイス全体を再パーティションするため、デバイス指定に注意してください

## 例

```bash
sudo bash writeimage.sh /dev/sdb minimam_platform
# root を明示する場合
sudo bash writeimage.sh /dev/sdb minimam_platform /dev/mmcblk0p2
```

実行後:

- SD 全体へ genimage 生成の `.img` を書き込み済み
- `boot.scr` を自動生成し、`/boot.scr` と `/boot/boot.scr` に配置済み
- U-Boot の `root=` は通常 `mmcblk0p2`（環境により `mmcblk1p2`）に設定
