# Hardware configuration for simple hello world
## directory tree
```
repo/
  src/        (HDL)
  constr/     (XDC)
  bd/         (BD,bd.tcl)
  ip/         (custom IP,xci)
  scripts/
    create_project.tcl
    build.tcl
```
## regeneration project
- `vivado -mode batch -source scripts/build.tcl`
  This command creates the “hello_world” project folder.
  Open hello_world/hello_world.xpr in Vivado.

## save project
1. File -> Project -> Write Tcl…
2. Tick the checkboxes for “Copy sources to new project” and “Recreate Block Designs using Tcl”.
3. Set the output destination under /scripts.