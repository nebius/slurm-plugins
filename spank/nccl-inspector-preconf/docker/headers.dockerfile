ARG SLURM_VERSION
ARG IMAGE_DAYTIME
# Or arm64
ARG ARCH="amd64"

FROM cr.eu-north1.nebius.cloud/ml-containers/slurm:${SLURM_VERSION}-${IMAGE_DAYTIME}-${ARCH} AS builder

RUN mkdir -p /tmp/include
CMD ["bash", "-lc", "cp -R /usr/include/slurm /tmp/include/"]
