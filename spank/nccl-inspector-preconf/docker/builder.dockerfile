# Or `debug`
ARG MODE=release
ARG PLUGIN_NAME

arg BASE_IMAGE
FROM ${BASE_IMAGE} AS builder

ARG PLUGIN_NAME

COPY src/ /usr/src/${PLUGIN_NAME}/
WORKDIR /usr/src/${PLUGIN_NAME}
SHELL ["/bin/bash", "-c"]

ENV CGO_ENABLED=1
ENV GO111MODULE=on

FROM builder AS build-release
ARG PLUGIN_NAME
ENV PLUGIN_NAME=${PLUGIN_NAME}
CMD test -f go.mod && \
    export CGO_CFLAGS="-O3 -DNDEBUG -fPIC -Wall -Wextra -I/usr/include" && \
    export CGO_LDFLAGS="" && \
    go build \
        -buildmode=c-shared \
        -trimpath \
        -ldflags="-s -w" \
        -o ./build/${PLUGIN_NAME}.so \
        .

FROM builder AS build-debug
ARG PLUGIN_NAME
ENV PLUGIN_NAME=${PLUGIN_NAME}
CMD test -f go.mod && \
    export CGO_CFLAGS="-O0 -g -fPIC -Wall -Wextra -I/usr/include" && \
    export CGO_LDFLAGS="" && \
    go build \
        -buildmode=c-shared \
        -gcflags="all=-N -l" \
        -o ./build/${PLUGIN_NAME}.so \
        .

FROM build-${MODE} AS build
ARG PLUGIN_NAME
ENV PLUGIN_NAME=${PLUGIN_NAME}
