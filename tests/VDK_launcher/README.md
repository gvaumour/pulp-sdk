
This is an example to launch GVSoC inside a SystemC simulation 

Note that this is different from the experiments with the ddr module that connects to systemC DRAM simulator
"System Simulation with PULP Virtual Platform and SystemC" Zulian et al. 2020 RAPIDO


# Compiling dependencies 

This example requires SystemC v2.3.3 and SCML library v2.6 

* To compile SystemC, you can use the script: get_systemc.sh 
It installs and compile in your $HOME directory by default (you can change it)

* To compiler SCML, you can use the script: install_scml.sh
It installs and compile in your $HOME directory by default (you can change it)


# Building 

Need to export configuration 
````
source ./configs/pulp-open.sh
export GAPY_PY_TARGET=Pulp_control_board@pulp_open.pulp_control_board
````

Then compile the launcher 
The SCML_HOME and SYSTEMC_HOME var env must be defined your the env
````
make clean all launcher.build
````
It will create the SCML_launcher executable that takes the json config file as input 

# Running 

The example connects a gvsoc_top component to a logger to exchange TLM transaction through TLM sockets
```
make launcher.run
```



