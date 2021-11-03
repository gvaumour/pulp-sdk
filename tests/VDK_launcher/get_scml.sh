#!/usr/bin/env bash

# Copyright (C) 2021 ATOS 
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Author: Gregory Vaumourin (gregory.vaumourin@atos.net) 

set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null && pwd)"
DEST=${HOME}
SYSC=scml-2.6.0
SYSCTARBALL=$SYSC.tgz
BUILDDIR=objdir


getnumprocs() {
	local __retvar=$1;
	local __nprocs=$(cat /proc/cpuinfo | grep processor | wc -l)
	if [[ "$__retvar" ]]; then
		eval $__retvar="'$__nprocs'"
	else
		echo "$__nprocs"
	fi
}

echo "SYSTEMC_HOME ="${SYSTEMC_HOME}
if [ -z "${SYSTEMC_HOME}" ]; then 
	echo "The SYSTEMC_HOME env var is missing" 
fi

# Create the destination folder and download the code into it
mkdir -p $DEST
cd $DEST
tar -xaf $DIR/$SYSCTARBALL
cd $SYSC

#Patch scml to use c++14 standard 
sed -i 's/std\=c++11/std\=c++14/g' ./configure*


# Build the library
mkdir $BUILDDIR
cd $BUILDDIR


../configure --with-systemc=${SYSTEMC_HOME}

getnumprocs NP
NJ=`expr $NP - 1`
make 
#make check -j$NJ
make install

# Add env. variables to ~/.bashrc
echo "export SCML_HOME=$DEST/$SYSC" >> ~/.bashrc

. ~/.bashrc
echo "Done."

