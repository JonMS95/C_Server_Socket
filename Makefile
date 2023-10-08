#####################################################################################################
# Common variables
config_file	:= config.xml

SH_FILES_PATH := $(shell xmlstarlet sel -t -v "config/Common_shell_files/@local_path" $(config_file))
PRJ_DATA_NODE := config/Project_data/
VERSION_MAJOR := "$(shell xmlstarlet sel -t -v "$(PRJ_DATA_NODE)@version_major" $(config_file))"
VERSION_MINOR := "$(shell xmlstarlet sel -t -v "$(PRJ_DATA_NODE)@version_minor" $(config_file))"
VERSION := v$(VERSION_MAJOR)_$(VERSION_MINOR)
VERSION_MODE := "$(shell xmlstarlet sel -t -v "$(PRJ_DATA_NODE)@version_mode" $(config_file))"


shell_dirs			:= Common_shell_files/directories.sh
shell_sym_links		:= Common_shell_files/sym_links.sh
shell_gen_versions 	:= Common_shell_files/gen_version.sh
shell_test			:= Shell_files/test.sh

ifeq ($(VERSION_MODE), "DEBUG")
	DEBUG_INFO := -g -Wall
else
	DEBUG_INFO :=
endif

HEADER_DEPS_DIR			:= Dependency_files/Header_files
SO_DEPS_DIR				:= Dependency_files/Dynamic_libraries

TEST_HEADER_DEPS_DIR	:= Tests/$(HEADER_DEPS_DIR)
TEST_SO_DEPS_DIR		:= Tests/$(SO_DEPS_DIR)
#####################################################################################################

#######################################
# Library variables
src_main 	= Source_files/main.c
src_sckt	= Source_files/socket_use.c
src_fsm		= Source_files/socket_FSM.c

obj_main	= Object_files/main.o
obj_sckt	= Object_files/sckt.o
obj_fsm		= Object_files/fsm.o

exe_main	= Executable_files/main
#######################################

#####################################################################
# Compound rules
exe: clean ln_sh_files directories deps main.o sckt.o fsm.o main

test: test_exe
#####################################################################

##############################################################################################################
# Exe Rules
clean:
	rm -rf Common_shell_files Object_files Dynamic_libraries Dependency_files

ln_sh_files:
	ln -sf $(SH_FILES_PATH) Common_shell_files

directories:
	@./$(shell_dirs)

deps:
	@bash $(shell_sym_links)

main.o: $(src_main)
	gcc $(DEBUG_INFO) -I$(HEADER_DEPS_DIR) -c $(src_main) -o $(obj_main)

sckt.o: $(src_sckt)
	gcc $(DEBUG_INFO) -I$(HEADER_DEPS_DIR) -c $(src_sckt) -o $(obj_sckt)

fsm.o: $(src_fsm)
	gcc $(DEBUG_INFO) -I$(HEADER_DEPS_DIR) -c $(src_fsm) -o $(obj_fsm)

main:
	gcc $(DEBUG_INFO) -I$(HEADER_DEPS_DIR) $(obj_main) $(obj_sckt) $(obj_fsm) -L$(SO_DEPS_DIR) -lGetOptions -lSeverityLog -o $(exe_main)
##############################################################################################################

######################################################################################################################
# Test Rules
test_exe:
	@./$(shell_test)
######################################################################################################################