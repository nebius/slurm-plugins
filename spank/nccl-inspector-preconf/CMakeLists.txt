cmake_minimum_required(VERSION 3.10)
project(spank_nccl_inspector_preconf C)

option(USE_SYSTEM_SLURM "Use system Slurm headers instead of vendor" OFF)

set(LIBRARY_NAME nccliprecon)

set(SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/snccliprecon.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/snccliprecon_config.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/snccliprecon_enroot.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/snccliprecon_env.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/snccliprecon_file_ops.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/snccliprecon_worker_lock.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/snccliprecon_op_user_init.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/snccliprecon_op_task_init_privileged.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/snccliprecon_op_task_exit.c
)

if (USE_SYSTEM_SLURM)
  find_path(SLURM_INCLUDE_DIR
          NAMES spank.h
          PATHS /usr/include /usr/local/include
          PATH_SUFFIXES slurm
          REQUIRED
  )
  message(STATUS "Using system Slurm headers from: ${SLURM_INCLUDE_DIR}")
  include_directories(${SLURM_INCLUDE_DIR})
else ()
  set(SLURM_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor)
  message(STATUS "Using vendored Slurm headers from: ${SLURM_INCLUDE_DIR}")
  include_directories(${SLURM_INCLUDE_DIR})
endif ()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_library(${LIBRARY_NAME} SHARED ${SOURCES})

set_target_properties(${LIBRARY_NAME} PROPERTIES
        OUTPUT_NAME "spank_nccl_inspector_preconf"
        PREFIX ""
        SUFFIX ".so"
        C_STANDARD 99
        POSITION_INDEPENDENT_CODE ON
)
