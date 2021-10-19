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

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null && pwd)"
DEST=${HOME}
SYSC=systemc-2.3.3
SYSCTARBALL=$SYSC.tar.gz
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

# Create the destination folder and download the code into it
mkdir -p $DEST
cd $DEST
wget -nc http://accellera.org/images/downloads/standards/systemc/$SYSCTARBALL
tar -xaf $SYSCTARBALL
rm $SYSCTARBALL


# Build the library
cd $DEST/$SYSC
mkdir $BUILDDIR
cd $BUILDDIR


export SYSTEMC_HOME=$DEST/$SYSC

../configure
getnumprocs NP
NJ=`expr $NP - 1`
make -j$NJ
make check -j$NJ
make install

# Add env. variables to ~/.bashrc
echo "# SystemC home" >> ~/.bashrc
echo "export SYSTEMC_HOME=$DEST/$SYSC" >> ~/.bashrc

# Reload configuration file
. ~/.bashrc
echo "Done."
