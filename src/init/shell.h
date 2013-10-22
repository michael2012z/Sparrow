#ifndef _SHELL_H_
#define _SHELL_H_

#define SHELL_MAX_PARAMETERS
#define SHELL_MAX_CMD_LENGTH

#define SHELL_COMMAND_ID_NONE		0 
#define SHELL_COMMAND_ID_ELF 		1
#define SHELL_COMMAND_ID_ELFS		2
#define SHELL_COMMAND_ID_ELFA		3
#define SHELL_COMMAND_ID_JIFFIES	4
#define SHELL_COMMAND_ID_VRUNTIME	5
#define SHELL_COMMAND_ID_HELP		6

void handle_cmd_elfs(char **secondary_parameters);
void handle_cmd_elf(char **secondary_parameters);
void handle_cmd_elfa(char **secondary_parameters);
void handle_cmd_jiffies(char **secondary_parameters);
void handle_cmd_vruntime(char **secondary_parameters);
void handle_cmd_help(char **secondary_parameters);


#endif /* _SHELL_H_ */

