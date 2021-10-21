#!/bin/bash 

PULP_DIR="/WORK/FastModels_materials/software/RISCV/pulp-sdk"

source ${PULP_DIR}"/configs/pulp-open.sh"
export GAPY_PY_TARGET=Pulp_control_board@pulp_open.pulp_control_board

# JSON config file used for the simulation 
export VDK_CONFIG_FILE=${PULP_DIR}"/tests/VDK_launcher/gvsoc_config.json"

