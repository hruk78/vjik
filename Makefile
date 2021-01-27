SRC=makemodel.c
TGT=makemodel

cc=cc
#CFLAGS ?=-O0 -std=c99 -D_BSD_SOURCE -g -g3
CFLAGS ?=-O9 -std=c99 -D_BSD_SOURCE 
XGBOOST_ROOT ?=/home/hruk/xgboost
INCLUDE_DIR=-I$(XGBOOST_ROOT)/include -I$(XGBOOST_ROOT)/dmlc-core/include -I$(XGBOOST_ROOT)/rabit/include
LIB_DIR=-L$(XGBOOST_ROOT)/lib

build: $(TGT)

$(TGT): $(SRC) Makefile
	$(cc) $(CFLAGS) $(INCLUDE_DIR) $(LIB_DIR) -o $(TGT) $(SRC) -lxgboost

run: $(TGT)
	LD_LIBRARY_PATH=$(XGBOOST_ROOT)/lib ./$(TGT)

clean:
	rm -f $(TGT)