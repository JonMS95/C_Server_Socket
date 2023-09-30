src_main 	= Source_files/main.c
src_sckt	= Source_files/socket_use.c
src_fsm		= Source_files/socket_FSM.c

obj_main	= Object_files/main.o
obj_sckt	= Object_files/sckt.o
obj_fsm		= Object_files/fsm.o

exe_main	= Executable_files/main

shell_dirs		= Common_shell_files/directories.sh
shell_sym_links	= Common_shell_files/sym_links.sh
shell_test		= Shell_files/test.sh

p_deps	= config/Dependencies/Header_files
P_deps	= Dependency_files/Header_files
s_deps	= config/Dependencies/Dynamic_libraries
S_deps	= Dependency_files/Dynamic_libraries

# Retrieve the XML selection
SH_FILES_PATH := $(shell xmlstarlet sel -t -v "//$$(xmlstarlet el -a "config.xml" | grep "config/Common_shell_files.*@.*PathToShellFiles")" "config.xml")

all: all_but_test test

all_but_test: clean ln_sh_files directories deps main.o sckt.o fsm.o main rm_obj msg

ln_sh_files:
	ln -sf $(SH_FILES_PATH) Common_shell_files

directories:
	@./$(shell_dirs)

deps:
	@bash $(shell_sym_links) -p $(p_deps) -P $(P_deps) -s $(s_deps) -S $(S_deps)

main.o: $(src_main)
	gcc -c -g -Wall $(src_main) -o $(obj_main)

sckt.o: $(src_sckt)
	gcc -c -g -Wall $(src_sckt) -o $(obj_sckt)

fsm.o: $(src_fsm)
	gcc -c -g -Wall $(src_fsm) -o $(obj_fsm)

main:
	gcc -g -Wall $(obj_main) $(obj_sckt) $(obj_fsm) -LDependency_files/Dynamic_libraries -lGetOptions -lSeverityLog -o $(exe_main)

clean:
	rm -rf Common_shell_files Object_files/* Executable_files/* Dependency_files/Dynamic_libraries/* Dependency_files/Header_files/*

rm_obj:
	rm -rf Object_files

msg:
	@echo "**************************************************************************************************"
	@echo "Don't forget to set the path:"
	@echo "export LD_LIBRARY_PATH=~/Desktop/scripts/C/C_Socket/Dependency_files/Dynamic_libraries"
	@echo "**************************************************************************************************"

test:
	@./$(shell_test)