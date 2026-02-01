FROM ubuntu:22.04

ARG USER=kaz
ARG UID=1000
ARG GID=1000

ENV DEBIAN_FRONTEND=noninteractive
ENV LANG=en_US.UTF-8
ENV LC_ALL=en_US.UTF-8

RUN apt-get update && apt-get install -y --no-install-recommends \
    xterm autoconf libtool less bc texinfo zlib1g-dev \
    gcc-multilib lsb-release build-essential \
    bash coreutils diffstat chrpath socat cpio \
    gawk wget git rsync \
    xz-utils zstd \
    python3 python3-pexpect python3-pip python3-jinja2 \
    file locales pax iproute2 unzip tar ca-certificates \
    libtinfo6 libncurses5 libncursesw5 libncurses-dev \
    libssl-dev \
    && rm -rf /var/lib/apt/lists/*

RUN locale-gen en_US.UTF-8

RUN groupadd -g ${GID} ${USER} \
 && useradd -m -u ${UID} -g ${GID} -s /bin/bash ${USER}

WORKDIR /home/${USER}

# legacy builder でも確実に動くように COPY --chown を使わない
COPY petalinux-*.run /home/${USER}/
RUN chown ${USER}:${USER} /home/${USER}/petalinux-*.run \
 && chmod +x /home/${USER}/petalinux-*.run

USER ${USER}

RUN /home/${USER}/petalinux-*.run -y -d /home/${USER}/petalinux

RUN echo 'source ~/petalinux/settings.sh' >> ~/.bashrc

WORKDIR /work
