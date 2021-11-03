APP = test
APP_SRCS = test.c
APP_CFLAGS += -O3 -g

NB_PE ?= 8


APP_ARCH_CFLAGS ?=  -march=rv32imcxgap9 -mPE=$(NB_PE) -mFC=1
APP_ARCH_LDFLAGS ?=  -march=rv32imcxgap9 -mPE=$(NB_PE) -mFC=1

APP_CFLAGS += -DNB_PE=$(NB_PE)

include $(RULES_DIR)/pmsis_rules.mk

LAUNCHER = TMB_programming

LAUNCHER_SRCS = main.cpp ../src/Gvsoc_Top.cpp
LAUNCHER_CFLAGS += -DIS_STANDALONE_EXEC -I${SYSTEMC_HOME}/include -I${SCML_HOME}/include -I$(GVSOC_SRC_PATH)/engine/include -I${INSTALL_DIR}/include
LAUNCHER_LD_FLAGS += -L${SYSTEMC_HOME}/lib-linux64 -L${SCML_HOME}/lib-linux64 -lsystemc -lscml -L$(GVSOC_PATH) -lpulpvp-debug 

launcher.build: Makefile $(LAUNCHER_SRCS)
	g++ -std=c++14 -O2 -g -o $(LAUNCHER)  $(LAUNCHER_SRCS)  $(LAUNCHER_CFLAGS) $(LAUNCHER_LD_FLAGS)

launcher.run: all run.prepare
	./$(LAUNCHER) $(BUILDDIR)/gvsoc_config.json
