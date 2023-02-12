src_main 	= Source_files/main.c
src_sckt	= Source_files/socket_use.c

exe_main	= Executable_files/main

shell_dirs	= Shell_files/directories.sh
shell_test	= Shell_files/test.sh

# all: directories clean main msg test
all: directories clean main msg test

directories:
	@./$(shell_dirs)

main: $(src_main)
	gcc -g $(src_sckt) $(src_main) -o $(exe_main)

clean:
	rm -rf Executable_files/*

msg:
	@echo "**************************************************************************************";

test:
	@./$(shell_test)