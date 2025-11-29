# scripts/build.tcl
set here [file dirname [info script]]
set gen  [file normalize "$here/create_project.tcl"]

# 1) Vivadoが吐いたプロジェクト再現Tclを実行

source $gen

# 2) BD wrapperを作って追加し、top設定
set bd_file [get_files design_1.bd]
set wrapper_file [make_wrapper -files $bd_file -top]
add_files -norecurse $wrapper_file
update_compile_order -fileset sources_1
set_property top [file rootname [file tail $wrapper_file]] [get_filesets sources_1]
