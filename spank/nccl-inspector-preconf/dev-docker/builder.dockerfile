ARG SLURM_VERSION
ARG IMAGE_DAYTIME
# Or arm64
ARG ARCH="amd64"

ARG PLUGIN_NAME

FROM cr.eu-north1.nebius.cloud/ml-containers/slurm:${SLURM_VERSION}-${IMAGE_DAYTIME}-${ARCH} AS builder

ARG PLUGIN_NAME

COPY src/ /usr/src/${PLUGIN_NAME}/
WORKDIR /usr/src/${PLUGIN_NAME}

SHELL ["/bin/bash", "-c"]

ENV PLUGIN_NAME=${PLUGIN_NAME}
CMD mkdir -p ./build && \
    gcc -std=gnu99 \
        -shared \
        -Wall -Wextra -Wno-unused-parameter \
        -Wl,-s \
        -fPIC \
        -O3 \
        -I. -I/usr/include \
        -o ./build/${PLUGIN_NAME}.so \
        ./*.c
