# TODO - it would be nice if LPATH could be set by the Makefile that
# includes us, since that has to know our path anyway.
LPATH := tools/disasm

LSRC := $(wildcard $(LPATH)/*.cpp)
LPRODUCTS := disasm

disasm_OBJ = \
	misc/Args.o \
	filesys/FileSystem.o \
	misc/Console.o \
	misc/Q_strcasecmp.o \
	tools/disasm/Disasm.o \
	tools/fold/Type.o \
	tools/fold/IfNode.o \
	tools/fold/OperatorNodes.o \
	tools/fold/Folder.o \
	tools/fold/CallNodes.o \
	tools/fold/VarNodes.o \
	tools/fold/FuncNodes.o

# ../../filesys/Flex.o
# Common rules
include common.mk
