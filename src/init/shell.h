#ifndef _SHELL_H_
#define _SHELL_H_

#define SHELL_MAX_PARAMETERS
#define SHELL_MAX_CMD_LENGTH

#define SHELL_COMMAND_ID_NONE		0 
#define SHELL_COMMAND_ID_ELF 		1
#define SHELL_COMMAND_ID_ELFS		2
#define SHELL_COMMAND_ID_ELFA		3
#define SHELL_COMMAND_ID_LS			4
#define SHELL_COMMAND_ID_TREE		5
#define SHELL_COMMAND_ID_JIFFIES	6
#define SHELL_COMMAND_ID_CAT		7
#define SHELL_COMMAND_ID_HELP		8


int __init kernel_shell(void *unused);

void handle_cmd_elfs(char *primary_parameter, char **secondary_parameters);
void handle_cmd_elf(char *primary_parameter, char **secondary_parameters);
void handle_cmd_elfa(char *primary_parameter, char **secondary_parameters);
void handle_cmd_ls(char *primary_parameter, char **secondary_parameters);
void handle_cmd_tree(char *primary_parameter, char **secondary_parameters);
void handle_cmd_jiffies(char *primary_parameter, char **secondary_parameters);
void handle_cmd_cat(char *primary_parameter, char **secondary_parameters);
void handle_cmd_help(char *primary_parameter, char **secondary_parameters);


#endif /* _SHELL_H_ */

