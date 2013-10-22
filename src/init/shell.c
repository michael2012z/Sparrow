#include <type.h>
#include <linkage.h>
#include <printk.h>
#include <string.h>
#include "shell.h"
#include <uart.h>

static char *cmd_line_stripe(char *cmd_line, int cmd_line_len) {
  int i = 0;
  char *cmd_p = cmd_line;
  /* stripe leading blankspace*/
  for (i = 0;i < cmd_line_len; i++)
	if (' ' == cmd_line[i])
	  cmd_p++;
	else
	  break;

  /* stripe trailing blankspace */
  for (i = (cmd_line_len - 1); i >= 0; i--)
	if ((' ' == cmd_line[i]) || ('\n' == cmd_line[i]))
	  cmd_line[i] = 0;
	else
	  break;

  return cmd_p;
}

static int cmd_line_parse(char **cmd_p, char **command, char **primary_parameter, char **secondary_parameters) {
  char *p = *cmd_p;
  int i;
  char *accept_array[6] = {*command, 
						   *primary_parameter,
						   secondary_parameters[0],
						   secondary_parameters[1],
						   secondary_parameters[2],
						   secondary_parameters[3]};
  *command = NULL;
  *primary_parameter = NULL;
  for (i = 0; i < 4; i++)
	secondary_parameters[i] = NULL;

  for (i = 0; i < 6; i++) {
	if (0 == *p)
	  return 0;
	if (0 != *p)
	  accept_array[i] = p;
	while(' ' != *p)
	  p++;
	while(' ' == *p)
	  *(p++) = 0;
  }

  return 0 ;
}


static int find_command_id(char *cmd) {
  int ret = -1;
  if (0 == cmd)
	ret = SHELL_COMMAND_ID_NONE;
  else {
	if (0 == strcmp(cmd, "elf"))
	  ret = SHELL_COMMAND_ID_ELF;
	else if (0 == strcmp(cmd, "elfs"))
	  ret = SHELL_COMMAND_ID_ELFS;
	else if (0 == strcmp(cmd, "elfa"))
	  ret = SHELL_COMMAND_ID_ELFA;
	else if (0 == strcmp(cmd, "jiffies"))
	  ret = SHELL_COMMAND_ID_JIFFIES;
	else if (0 == strcmp(cmd, "vruntime"))
	  ret = SHELL_COMMAND_ID_VRUNTIME;
	else if (0 == strcmp(cmd, "help"))
	  ret = SHELL_COMMAND_ID_HELP;
	else
	  ret = -1;
  }
  return ret;
}

int __init kernel_shell(void *unused) {
  /* 1. fetch command line 
   * 2. parse
   * 3. handle different commands
   */
  char ch = 0;
  char cmd_line[256] = {0};
  int i = 0, cmd_id, error;
  char *cmd_p;
  char *command, *primary_parameter, *secondary_parameters[4];
  
  do {
	for (i = 0; i < 256; i++)
	  cmd_line[i] = 0;
	i = 0;
	/* fetch user input, char by char */
	do { /* ENTER indicates the end of a command line */
	  ch = inputc();
	  printk(PR_SS_INI, PR_LVL_DBG5, "%s: char %c was received\n", __func__, ch);
	  i++;
	  if (i >= 256)
		break;
	} while('\n' != ch);
	printk(PR_SS_INI, PR_LVL_DBG5, "%s: received command line length: %d\n", __func__, i);
	if (i >= 256)
	  continue;

	cmd_p = cmd_line_stripe(cmd_line, 256);
	printk(PR_SS_INI, PR_LVL_DBG5, "%s: command line after striping:\n%s\n", __func__, cmd_p);

	error = cmd_line_parse(&cmd_p, &command, &primary_parameter, secondary_parameters);
	printk(PR_SS_INI, PR_LVL_DBG5, "%s: cmd_line_parse = %d\n", __func__, error);

	if (-1 == error) {
	  printu("invalid command\n");
	  continue;
	}

	/* distribute command */
	cmd_id = find_command_id(command);
	printk(PR_SS_INI, PR_LVL_DBG5, "%s: cmd_id = %d\n", __func__, cmd_id);
	switch (cmd_id) {
	case SHELL_COMMAND_ID_NONE:
	  break;
	case SHELL_COMMAND_ID_ELF:
	  handle_cmd_elf(secondary_parameters);
	  break;
	case SHELL_COMMAND_ID_ELFS:
	  handle_cmd_elfs(secondary_parameters);
	  break;
	case SHELL_COMMAND_ID_ELFA:
	  handle_cmd_elfa(secondary_parameters);
	  break;
	case SHELL_COMMAND_ID_JIFFIES:
	  handle_cmd_jiffies(secondary_parameters);
	  break;
	case SHELL_COMMAND_ID_VRUNTIME:
	  handle_cmd_vruntime(secondary_parameters);
	  break;
	case SHELL_COMMAND_ID_HELP:
	  handle_cmd_help(secondary_parameters);
	  break;
	default:
	  printu("command '%s' is not known\n", cmd_p);
	  continue;
	}
  } while(1);
  
  return 0;
}
