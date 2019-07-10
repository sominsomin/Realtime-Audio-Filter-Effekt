# Makefile to build class 'pan~' for Pure Data.
# Needs Makefile.pdlibbuilder as helper makefile for platform-dependent build
# settings and rules.

# library name
lib.name = externals.fofi

# input source file (class name == source file basename)
class.sources = fofi~.c

# all extra files to be included in binary distribution of the library
datafiles =

# include Makefile.pdlibbuilder from submodule directory 'pd-lib-builder'
PDLIBBUILDER_DIR=../pd-lib-builder/
include $(PDLIBBUILDER_DIR)/Makefile.pdlibbuilder

debug_run: clean alldebug
	gdb -ex "add-symbol-file fofi~.pd_linux" --args pd -nrt -stderr -open ./fofi_test.pd

run: all
	pd -open ./fofi_test.pd
