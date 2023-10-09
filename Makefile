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
src_srv_sckt	= Source_files/ServerSocketUse.c
src_fsm			= Source_files/ServerSocketFSM.c

so_srv_sckt	= Dynamic_libraries/libServerSocket.so

src_main	= Tests/Source_files/main.c

exe_main	= Tests/Executable_files/main

d_test_deps	= config/Tests/Dependencies/
#######################################

#####################################################################
# Compound rules
exe: clean ln_sh_files directories deps srv_sckt.so api

test: clean_test directories test_deps test_main test_rm_obj test_exe
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

srv_sckt.so:
	gcc $(DEBUG_INFO) -I$(HEADER_DEPS_DIR) -fPIC -shared $(src_srv_sckt) $(src_fsm) -o $(so_srv_sckt)

api:
	@bash $(shell_gen_versions)

# Use this one carefully. Non-tagged versions will be impossible to recover if used.
clean_api:
	rm -rf API
##############################################################################################################

######################################################################################################################
# Test Rules
clean_test:
	rm -rf Tests/Dependency_files Tests/Object_files Tests/Executable_files

test_deps:
	@bash $(shell_sym_links) -d $(d_test_deps)

test_main:
	gcc $(DEBUG_INFO) -I$(TEST_HEADER_DEPS_DIR) $(src_main) -L$(TEST_SO_DEPS_DIR) -lGetOptions -lSeverityLog -lServerSocket -o $(exe_main)

test_rm_obj:
	rm -rf Tests/Object_files

test_exe:
	@./$(shell_test)
######################################################################################################################