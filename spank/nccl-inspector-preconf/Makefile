SHELL = bash

ARCH ?= $(shell arch)

ifeq ($(ARCH),arm64)
ARCH_LIB = aarch64
else ifeq ($(ARCH),aarch64)
ARCH_LIB = aarch64
else
ARCH_LIB = x86_64
endif

TARGET_COMPILATION_MODE ?= release

SLURM_VERSION ?= 25.11.3
DOCKER_IMAGE_DAYTIME ?= 20260324153054

BUILD_DIR := build
SLURM_HEADERS_DIR := src/vendor/slurm

DOCKER_IMAGE := snccliprecon
DOCKER_BUILDER_IMAGE := $(DOCKER_IMAGE)-builder
DOCKER_HEADERS_IMAGE := $(DOCKER_IMAGE)-headers

PLUGIN_NAME := spank_nccl_inspector_preconf
PLUGIN_SO := $(BUILD_DIR)/$(PLUGIN_NAME).so

NODE_WORKER_PREFIX ?= worker-h100
NODE_COUNT_WORKER ?= 2
NODE_COUNT_LOGIN ?= 2
NODE_RANGE_WORKER := $(shell seq 0 $$(($(NODE_COUNT_WORKER) - 1)))
NODE_RANGE_LOGIN := $(shell seq 0 $$(($(NODE_COUNT_LOGIN) - 1)))

all: docker-build

$(BUILD_DIR):
	@mkdir -p $@

$(SLURM_HEADERS_DIR):
	@mkdir -p $@

.PHONY: docker-headers
docker-headers: $(SLURM_HEADERS_DIR)
	@docker build \
		-t $(DOCKER_HEADERS_IMAGE) \
		-f dev-docker/headers.dockerfile \
		--platform linux/$(ARCH) \
		--build-arg ARCH=$(ARCH) \
		--build-arg SLURM_VERSION=$(SLURM_VERSION) \
		--build-arg IMAGE_DAYTIME=$(DOCKER_IMAGE_DAYTIME) \
		.

	@docker run \
		-v '$(shell pwd)/$(SLURM_HEADERS_DIR):/tmp/include/slurm' \
		--rm \
		--platform linux/$(ARCH) \
		$(DOCKER_HEADERS_IMAGE)

.PHONY: docker-build
docker-build: $(BUILD_DIR)
	@docker build \
		-t $(DOCKER_BUILDER_IMAGE) \
		-f dev-docker/builder.dockerfile \
		--platform linux/$(ARCH) \
		--build-arg ARCH=$(ARCH) \
		--build-arg SLURM_VERSION=$(SLURM_VERSION) \
		--build-arg IMAGE_DAYTIME=$(DOCKER_IMAGE_DAYTIME) \
		--build-arg MODE=$(TARGET_COMPILATION_MODE) \
		--build-arg PLUGIN_NAME=$(PLUGIN_NAME) \
		.

	@docker run \
		-v '$(shell pwd)/$(BUILD_DIR):/usr/src/$(PLUGIN_NAME)/build' \
		--rm \
		--platform linux/$(ARCH) \
		$(DOCKER_BUILDER_IMAGE)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

.PHONY: redeploy
redeploy: $(PLUGIN_SO)
	$(foreach \
		i, \
		$(NODE_RANGE_WORKER), \
		kubectl -n soperator cp ./$(PLUGIN_SO) $(NODE_WORKER_PREFIX)-$(i):/usr/lib/$(ARCH_LIB)-linux-gnu/slurm/$(PLUGIN_NAME).so ; \
	)
	$(foreach \
		i, \
		$(NODE_RANGE_LOGIN), \
		kubectl -n soperator cp ./$(PLUGIN_SO) login-$(i):/usr/lib/$(ARCH_LIB)-linux-gnu/slurm/$(PLUGIN_NAME).so ; \
	)

.PHONY: generate-options
generate-options:
	@cd src && \
		go run ./cmd/spank-options-gen > spank_options.c
