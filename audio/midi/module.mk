# TODO - it would be nice if LPATH could be set by the Makefile that
# includes us, since that has to know our path anyway.
LPATH := audio/midi

LSRC := $(wildcard $(LPATH)/*.cpp)
LPRODUCTS :=

# Common rules
include common.mk
