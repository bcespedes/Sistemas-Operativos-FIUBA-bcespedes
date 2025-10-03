#include "builtin.h"

int contains_command(char *str, char *command);
void reverse(char *str);

// returns "true (1)" if str equals to line elements
// returns "false (0)" in other case
int
contains_command(char *str, char *command)
{
	for (int i = 0; command[i] != END_STRING; i++) {
		if (str[i] != command[i]) {
			return 0;
		}
	}
	return 1;
}

// returns true if the 'exit' call
// should be performed
//
// (It must not be called from here)
int
exit_shell(char *cmd)
{
	if (contains_command(cmd, "exit") == 1) {
		return 1;
	}
	return 0;
}

// returns true if "chdir" was performed
//  this means that if 'cmd' contains:
// 	1. $ cd directory (change to 'directory')
// 	2. $ cd (change to $HOME)
//  it has to be executed and then return true
//
//  Remember to update the 'prompt' with the
//  	new directory.
//
// Examples:
//  1. cmd = ['c','d', ' ', '/', 'b', 'i', 'n', '\0']
//  2. cmd = ['c','d', '\0']
int
cd(char *cmd)
{
	// Si existe "cd", ejecutamos, si no, salimos
	// Verificar si hay directorio o no
	// Verificar si hay . o ..
	// Si hay y existe, entrar

	// Si es solo cd, ir a $HOME

	if (contains_command(cmd, "cd")) {
		int r;
		char *directory = split_line(cmd, SPACE);  // cmd guarda "cd"
		if (directory[0] == '\0') {
			char *home = getenv("HOME");
			r = chdir(home);
		} else if (strcmp(directory, ".") == 0) {
			return 1;
		} else if (strcmp(directory, "..") == 0) {
			char buf[BUFLEN];
			char *cwd = getcwd(buf, BUFLEN);  // /proc/sys
			reverse(cwd);                     // sys/corp/
			char *go_back = strdup(split_line(cwd, '/'));  // corp/
			reverse(go_back);                              // /proc
			r = chdir(go_back);
			free(go_back);
		}

		r = chdir(directory);

		if (r < -1) {
			perror("Error in cd");
			exit(-1);
		}

		return 1;
	}

	return 0;
}

// returns true if 'pwd' was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
pwd(char *cmd)
{
	if (contains_command(cmd, "pwd") == 1) {
		char buf[BUFLEN] = { 0 };
		char *pwd = getcwd(buf, sizeof(buf));
		printf("%s\n", pwd);
		return 1;
	}

	return 0;
}

// returns true if `history` was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
history(char *cmd)
{
	// Your code here

	return 0;
}

// in-place
void
reverse(char *str)
{
	size_t len = strlen(str);
	char temp;

	for (size_t i = 0; i < len / 2; i++) {
		size_t rev = len - i - 1;
		temp = str[i];
		str[i] = str[rev];
		str[rev] = temp;
	}
}
