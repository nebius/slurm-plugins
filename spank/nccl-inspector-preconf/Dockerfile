ARG SLURM_VERSION
ARG IMAGE_DAYTIME
ARG PLUGIN_NAME
FROM cr.eu-north1.nebius.cloud/ml-containers/slurm:${SLURM_VERSION}-${IMAGE_DAYTIME} AS builder

ARG DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN echo 'deb [trusted=yes] https://repo.goreleaser.com/apt/ /' | tee /etc/apt/sources.list.d/goreleaser.list && \
    apt update && \
    apt -y --no-install-recommends install \
        build-essential \
        git  \
        nfpm

COPY README.md config/ /usr/src/nccl-inspector-preconf-deb/

# region Build plugin

ARG PLUGIN_NAME

COPY src/ /usr/src/${PLUGIN_NAME}/
WORKDIR /usr/src/${PLUGIN_NAME}

RUN mkdir -p ./build && \
    gcc \
      -std=gnu99 \
      -shared \
      -Wall -Wextra -Wno-unused-parameter \
      -Wl,-s \
      -fPIC \
      -O3 \
      -I. -I/usr/include \
      -o ./build/${PLUGIN_NAME}.so \
      ./*.c && \
    ls -lah ./build/*.so && \
    mv ./build/*.so /usr/src/nccl-inspector-preconf-deb

# endregion Build plugin

WORKDIR /usr/src/nccl-inspector-preconf-deb

ARG DEB_RELEASE
ARG ARCH
ARG LIB_ARCH

# region Build deb

RUN chmod +x copyright.sh && \
    ./copyright.sh > copyright

RUN mkdir artifacts && \
    nfpm package -p deb -t artifacts && \
    cp ${PLUGIN_NAME}.so ./artifacts/${PLUGIN_NAME}_${SLURM_VERSION}-${DEB_RELEASE}_${ARCH}.so && \
    ls -lah ./artifacts/*

# endregion Build deb

FROM cr.eu-north1.nebius.cloud/soperator/busybox
COPY --from=builder /usr/src/nccl-inspector-preconf-deb/artifacts /artifacts
CMD ["true"]
