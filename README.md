# Makefiles
NOTE: To disabled compiler optimization flags for debugging with gdb you need to comment out

optimization.flags = -O3 -ffast-math -funroll-loops -fomit-frame-pointer

in "pd-lib-builder/Makefile.pdlibbuilder"

## make run
Compiles the plugin and runs PD with a test patch

## make debug_run
Compiles the plugin with debuggin flags and runs PD within gdb. Also lodes the symbol file for the plugin
so that gdb properly autocompletes.

# Doxygen

run:

doxygen Doxyfile

in ./fofi~/ to genereate/update docs.
