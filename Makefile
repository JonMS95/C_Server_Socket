src_main 	= Source_files/main.c
src_sckt	= Source_files/socket_use.c

obj_main	= Object_files/main.o
obj_sckt	= Object_files/sckt.o

exe_main	= Executable_files/main

shell_dirs	= Shell_files/directories.sh
shell_test	= Shell_files/test.sh

deps_opt_so_org	= /home/jon/Desktop/scripts/C/C_Get_Options/API/Dynamic_libraries/libGetOptions.so
deps_opt_h_org	= /home/jon/Desktop/scripts/C/C_Get_Options/API/Header_files/GetOptions_api.h

deps_opt_so_dst	= Dependency_files/Dynamic_libraries/libGetOptions.so
deps_opt_h_dst	= Dependency_files/Header_files/GetOptions_api.h

deps_log_so_org	= /home/jon/Desktop/scripts/C/C_Severity_Log/Dynamic_libraries/libSeverityLog.so
deps_log_h_org	= /home/jon/Desktop/scripts/C/C_Severity_Log/Source_files/SeverityLog_api.h

deps_log_so_dst	= Dependency_files/Dynamic_libraries/libSeverityLog.so
deps_log_h_dst	= Dependency_files/Header_files/SeverityLog_api.h

all: all_but_test test

all_but_test: directories clean getdeps main.o sckt.o main rm_obj msg

directories:
	@./$(shell_dirs)

getdeps:
	cp $(deps_opt_h_org)  $(deps_opt_h_dst)
	cp $(deps_log_h_org)  $(deps_log_h_dst)
	cp $(deps_opt_so_org) $(deps_opt_so_dst)
	cp $(deps_log_so_org) $(deps_log_so_dst)

main.o: $(src_main)
	gcc -c -g -Wall $(src_main) -o $(obj_main)

sckt.o: $(src_sckt)
	gcc -c -g -Wall $(src_sckt) -o $(obj_sckt)

main:
	gcc -g -Wall $(obj_main) $(obj_sckt) -LDependency_files/Dynamic_libraries -lGetOptions -lSeverityLog -o $(exe_main)

clean:
	rm -rf Object_files/* Executable_files/* Dependency_files/Dynamic_libraries/* Dependency_files/Header_files/*

rm_obj:
	rm -rf Object_files

msg:
	@echo "**************************************************************************************************"
	@echo "Don't forget to set the path:"
	@echo "export LD_LIBRARY_PATH=~/Desktop/scripts/C/C_Socket/Dependency_files/Dynamic_libraries"
	@echo "**************************************************************************************************"

test:
	@./$(shell_test)