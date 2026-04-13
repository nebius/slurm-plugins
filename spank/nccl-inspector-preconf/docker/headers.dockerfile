ARG BASE_IMAGE
FROM ${BASE_IMAGE} AS header-copier

RUN mkdir -p /tmp/include
CMD ["bash", "-lc", "cp -R /usr/include/slurm /tmp/include/"]
