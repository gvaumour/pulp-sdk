#!/bin/bash 

SCRIPT_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PULP_DIR=${SCRIPT_DIR}"/../.."

source ${PULP_DIR}"/configs/pulp-open.sh"
export GAPY_PY_TARGET=Pulp_control_board@pulp_open.pulp_control_board

