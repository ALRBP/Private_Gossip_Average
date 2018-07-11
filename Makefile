
.PHONY: clean, mrproper

.SUFFIXES:


DEBUG = no

export AR = ar
export RM = rm -rf
export MK = mkdir
export CD = cd
export CC = g++
export BCFLAGS = -W -Wall -Werror -std=c++17 -Wc++17-compat
export LDFLAGS=
export ARFLAGS =
ifeq ($(DEBUG),yes)
	export PCFLAGS = -g $(BCFLAGS)
else
	export PCFLAGS = $(BCFLAGS) -O3
endif
export CFLAGS = $(PCFLAGS) -lm -lpthread
export BIN = pga-test
export LIB = libpga
export DBUILD = build
export DOUT = output
export LPATH = $(DOUT)
export PRG = ../
PRX = ../


all: $(BIN) $(LIB)

$(BIN): $(PRX)$(DOUT)/$(BIN)

$(LIB): $(PRX)$(DOUT)/$(LIB).so
	
$(PRX)$(DOUT)/$(LIB).so: $(PRX)$(DBUILD)/helpers.o $(PRX)$(DBUILD)/bpba_tree.o $(PRX)$(DBUILD)/local_connector.o $(PRX)$(DBUILD)/local_peer_sampling.o $(PRX)$(DBUILD)/req-pull_peer_sampling.o $(PRX)$(DBUILD)/hierarchical_peer_sampling.o $(PRX)$(DBUILD)/average.o $(PRX)$(DBUILD)/graph.o
	$(CC) $^ -shared -fPIC -o $@ $(CFLAGS)

$(PRX)$(DOUT)/$(BIN): $(PRX)$(DBUILD)/main.o
	$(MAKE) $(LIB)
	$(CC) $^ -fPIC -L $(PRX)$(LPATH) -Wl,-rpath=$(PRX)$(LPATH) -lpga -o $@ $(CFLAGS)

$(PRX)$(DBUILD)/helpers.o: helpers/helpers.c++
	$(CC) -c -fPIC $^ -o $@ $(CFLAGS)

$(PRX)$(DBUILD)/bpba_tree.o: bpba_tree/bpba_tree.c++
	$(CC) -c -fPIC $^ -o $@ $(CFLAGS)

$(PRX)$(DBUILD)/local_connector.o: connector/local/local.c++
	$(CC) -c -fPIC $^ -o $@ $(CFLAGS)

$(PRX)$(DBUILD)/local_peer_sampling.o: peer_sampling/local/local.c++
	$(CC) -c -fPIC $^ -o $@ $(CFLAGS)

$(PRX)$(DBUILD)/req-pull_peer_sampling.o: peer_sampling/req-pull/req-pull.c++
	$(CC) -c -fPIC $^ -o $@ $(CFLAGS)

$(PRX)$(DBUILD)/hierarchical_peer_sampling.o: peer_sampling/hierarchical/hierarchical.c++
	$(CC) -c -fPIC $^ -o $@ $(CFLAGS)

$(PRX)$(DBUILD)/average.o: average/average.c++
	$(CC) -c -fPIC $^ -o $@ $(CFLAGS)

$(PRX)$(DBUILD)/graph.o: graph/graph.c++
	$(CC) -c -fPIC $^ -o $@ $(CFLAGS)

$(PRX)$(DBUILD)/%.o: %.c++
	$(CC) -c $^ -o $@ $(CFLAGS)

clean:
	$(RM) $(PRX)$(DBUILD)/*

mrproper: clean
	$(RM) $(PRX)$(DOUT)/*
