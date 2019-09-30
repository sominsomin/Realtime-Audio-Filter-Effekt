## Makefiles
**NOTE:** To disable compiler optimization flags for debugging with gdb you need to comment out

```Makefile
optimization.flags = -O3 -ffast-math -funroll-loops -fomit-frame-pointer
```

in *"pd-lib-builder/Makefile.pdlibbuilder"*

### make run
Compiles the plugin and runs PD with a test patch

### make debug_run
Compiles the plugin with debuggin flags and runs PD within gdb (with realtime mode disabled).
Also loads the symbol file for the plugin so that gdb properly autocompletes.

## Doxygen

run:
```bash
doxygen Doxyfile
```

in *./fofi~/ to genereate/update docs.*
