# QEMU fork with ESP32 WiFi emulation, built for the tinyGS ESP32 integration spike.
#
# Pinned source:
#   repo:   https://github.com/Ebiroll/calib
#   commit: 73bee9bab5a78b0aaa584610b1564d533e7879ce (QEMU 9.2.2 base)
#
# Build only the Xtensa system emulator, with SLIRP user-mode networking and no
# GTK/SDL/VNC/curses UI. The resulting image contains the emulator and its runtime
# libraries, not the toolchain or the source tree.

FROM debian:bookworm-slim AS build

ARG QEMU_REPO=https://github.com/Ebiroll/calib
ARG QEMU_COMMIT=73bee9bab5a78b0aaa584610b1564d533e7879ce

RUN apt-get update && apt-get install -y --no-install-recommends \
        ca-certificates \
        git \
        build-essential \
        ninja-build \
        meson \
        pkg-config \
        python3 \
        python3-venv \
        libglib2.0-dev \
        libgcrypt20-dev \
        libpixman-1-dev \
        libslirp-dev \
    && rm -rf /var/lib/apt/lists/*

RUN mkdir -p /src/qemu \
    && cd /src/qemu \
    && git init -q \
    && git remote add origin "$QEMU_REPO" \
    && git fetch --depth 1 origin "$QEMU_COMMIT" \
    && git checkout -q FETCH_HEAD \
    && test "$(git rev-parse HEAD)" = "$QEMU_COMMIT"

# QEMU 9.2 fetches keycodemapdb/dtc through meson subproject wraps when needed;
# no git submodules are required for a headless xtensa-softmmu build.

RUN cd /src/qemu \
    && mkdir build \
    && cd build \
    && ../configure \
        --target-list=xtensa-softmmu \
        --enable-slirp \
        --disable-gtk \
        --disable-sdl \
        --disable-vnc \
        --disable-curses \
        --disable-capstone \
        --disable-docs \
        --disable-tools \
        --disable-fdt \
    && ninja -j"$(nproc)"

FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y --no-install-recommends \
        libglib2.0-0 \
        libgcrypt20 \
        libpixman-1-0 \
        libslirp0 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=build /src/qemu/build/qemu-system-xtensa /usr/local/bin/qemu-system-xtensa
COPY --from=build /src/qemu/pc-bios /usr/local/share/qemu

ENTRYPOINT ["qemu-system-xtensa"]
