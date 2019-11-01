#!/bin/bash

#  the shell script file must start with the above statement 

#  Copyright (C) 2018-2019 Intel Corporation
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.

error() {
    local code="${3:-1}"
    if [[ -n "$2" ]];then
        echo "Error on or near line $1: $2; exiting with status ${code}"    # print string 
    else
        echo "Error on or near line $1; exiting with status ${code}"        #
    fi
    exit "${code}"
}
trap 'error ${LINENO}' ERR

SAMPLES_PATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"            # No need to define the type of the variable
                                                                            # BASH_SOURCE[0]/BASH_SOURCE : get current shell path
                                                                            # SAMPLES_PATH : absolute path of the current program
if [[ -z "${InferenceEngine_DIR}" ]]; then
    printf "\nInferenceEngine_DIR environment variable is not set. Trying to find setupvars.sh to set it. \n"
                                                                    #determine if the environment variable is added
                                                                    #SAMPLES_PATH : ~/openvino/deployment_tools/inference_engine/samples/
    setvars_path=$SAMPLES_PATH/../..                                #setvars_path : ~/openvino/deployment_tools/
    if [ -e "$setvars_path/inference_engine/bin/setvars.sh" ]; then # for Intel Deep Learning Deployment Toolkit package
        setvars_path="$setvars_path/inference_engine/bin/setvars.sh"
    elif [ -e "$setvars_path/../bin/setupvars.sh" ]; then           # for OpenVINO package
        setvars_path="$setvars_path/../bin/setupvars.sh"
    elif [ -e "$setvars_path/../setupvars.sh" ]; then
        setvars_path="$setvars_path/../setupvars.sh"
    else
        printf "Error: setupvars.sh is not found in hardcoded paths. \n\n"
        exit 1
    fi
    if ! source $setvars_path ; then
        printf "Unable to run ./setupvars.sh. Please check its presence. \n\n"
        exit 1
    fi
fi

if ! command -v cmake &>/dev/null; then
    printf "\n\nCMAKE is not installed. It is required to build Inference Engine samples. Please install it. \n\n"
    exit 1
fi

build_dir=$HOME/inference_engine_samples_build    # $HOME : /root/

OS_PATH=$(uname -m)                               #x86_64
NUM_THREADS="-j2"                      

if [ $OS_PATH == "x86_64" ]; then
  OS_PATH="intel64"
  NUM_THREADS="-j8"
fi

if [ -e $build_dir/CMakeCache.txt ]; then
    rm -rf $build_dir/CMakeCache.txt
fi                                         #clear the CMakeCache.txt file
mkdir -p $build_dir
cd $build_dir
cmake -DCMAKE_BUILD_TYPE=Release $SAMPLES_PATH
make $NUM_THREADS


printf "\nBuild completed, you can find binaries for all samples in the $HOME/inference_engine_samples_build/${OS_PATH}/Release subfolder.\n\n"
