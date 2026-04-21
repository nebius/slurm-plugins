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
        golang-1.23 \
        git  \
        nfpm

COPY README.md config/ /usr/src/nccl-inspector-preconf-deb/

# region Build plugin

ARG PLUGIN_NAME

COPY src/ /usr/src/${PLUGIN_NAME}/
WORKDIR /usr/src/${PLUGIN_NAME}

ENV CGO_ENABLED=1
ENV GO111MODULE=on
ENV PATH="$PATH:/usr/lib/go-1.23/bin"

RUN export CGO_CFLAGS="-O3 -fPIC -Wall -Wextra -Wno-unused-parameter -I/usr/include" && \
    export CGO_LDFLAGS="" && \
    go build \
        -buildmode=c-shared \
        -trimpath \
        -ldflags="-s -w" \
        -o ./build/${PLUGIN_NAME}.so \
        . && \
    ls -lah ./build/*.so && \
    mv ./build/*.so /usr/src/nccl-inspector-preconf-deb

# endregion Build plugin

WORKDIR /usr/src/nccl-inspector-preconf-deb

ARG DEB_RELEASE
ARG DEB_ARCH
ARG ARCH

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
