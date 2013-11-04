#include <type.h>
#include <linkage.h>
#include <printk.h>
#include <string.h>
#include "shell.h"
#ifndef __ARCH_X86__
#include <uart.h>
#else
#include <stdio.h>
#define printu //
#endif

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
	else if (0 == cmd_line[i])
	  continue;
	else
	  break;

  return cmd_p;
}

static int cmd_line_parse(char **cmd_p, char **command, char **primary_parameter, char **secondary_parameters) {
  char *p = *cmd_p;
  int i;
  char **accept_array[6] = {command, 
						   primary_parameter,
						   &secondary_parameters[0],
						   &secondary_parameters[1],
						   &secondary_parameters[2],
						   &secondary_parameters[3]};

  *command = NULL;
  *primary_parameter = NULL;
  for (i = 0; i < 4; i++)
	secondary_parameters[i] = NULL;

  for (i = 0; i < 6; i++) {
	if (0 == *p)
	  break;
	if (0 != *p)
	  *accept_array[i] = p;
	while ((' ' != *p) && (0 != *p))
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
	else if (0 == strcmp(cmd, "ls"))
	  ret = SHELL_COMMAND_ID_LS;
	else if (0 == strcmp(cmd, "tree"))
	  ret = SHELL_COMMAND_ID_TREE;
	else if (0 == strcmp(cmd, "jiffies"))
	  ret = SHELL_COMMAND_ID_JIFFIES;
	else if (0 == strcmp(cmd, "cat"))
	  ret = SHELL_COMMAND_ID_CAT;
	else if (0 == strcmp(cmd, "kmsg"))
	  ret = SHELL_COMMAND_ID_KMSG;
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

  printk(PR_SS_INI, PR_LVL_DBG5, "%s: kernel shell started\n", __func__);
  printu("\n");
  printu("**********************************************\n");
  printu("         Sparrow OS default shell\n");
  printu("**********************************************\n");
  printu("\n");

  do {
	for (i = 0; i < 256; i++)
	  cmd_line[i] = 0;
	i = 0;

	printu("[sparrow] $: ");
	/* fetch user input, char by char */
	do { /* ENTER indicates the end of a command line */
	  ch = inputc();
	  printk(PR_SS_INI, PR_LVL_DBG5, "%s: char %c was received\n", __func__, ch);
	  cmd_line[i] = ch;
	  i++;
	  printu("%c", ch);
	  if (i >= 256)
		break;
	} while('\n' != ch);
	printk(PR_SS_INI, PR_LVL_DBG5, "%s: received command line: %s\n", __func__, cmd_line);
	printk(PR_SS_INI, PR_LVL_DBG5, "%s: received command line length: %d\n", __func__, i);
	if (i >= 256)
	  continue;

	cmd_p = cmd_line_stripe(cmd_line, 256);
	printk(PR_SS_INI, PR_LVL_DBG5, "%s: command line after striping:\n%s\n", __func__, cmd_p);

	error = cmd_line_parse(&cmd_p, &command, &primary_parameter, secondary_parameters);
	printk(PR_SS_INI, PR_LVL_DBG5, "%s: cmd_line_parse = %d\n", __func__, error);

	printk(PR_SS_INI, PR_LVL_DBG5, "%s: separated command line:\n", __func__);
	printk(PR_SS_INI, PR_LVL_DBG5, "%s: command = %s\n", __func__, command);
	if (primary_parameter)
	  printk(PR_SS_INI, PR_LVL_DBG5, "%s: primary_parameter = %s\n", __func__, primary_parameter);
	if (secondary_parameters[0])
	  printk(PR_SS_INI, PR_LVL_DBG5, "%s: secondary_parameters[0] = %s\n", __func__, secondary_parameters[0]);
	if (secondary_parameters[1])
	  printk(PR_SS_INI, PR_LVL_DBG5, "%s: secondary_parameters[1] = %s\n", __func__, secondary_parameters[1]);
	if (secondary_parameters[2])
	  printk(PR_SS_INI, PR_LVL_DBG5, "%s: secondary_parameters[2] = %s\n", __func__, secondary_parameters[2]);
	if (secondary_parameters[3])
	  printk(PR_SS_INI, PR_LVL_DBG5, "%s: secondary_parameters[3] = %s\n", __func__, secondary_parameters[3]);
	
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
	  handle_cmd_elf(primary_parameter, secondary_parameters);
	  break;
	case SHELL_COMMAND_ID_ELFS:
	  handle_cmd_elfs(primary_parameter, secondary_parameters);
	  break;
	case SHELL_COMMAND_ID_ELFA:
	  handle_cmd_elfa(primary_parameter, secondary_parameters);
	  break;
	case SHELL_COMMAND_ID_LS:
	  handle_cmd_ls(primary_parameter, secondary_parameters);
	  break;
	case SHELL_COMMAND_ID_TREE:
	  handle_cmd_tree(primary_parameter, secondary_parameters);
	  break;
	case SHELL_COMMAND_ID_JIFFIES:
	  handle_cmd_jiffies(primary_parameter, secondary_parameters);
	  break;
	case SHELL_COMMAND_ID_CAT:
	  handle_cmd_cat(primary_parameter, secondary_parameters);
	  break;
	case SHELL_COMMAND_ID_KMSG:
	  handle_cmd_kmsg(primary_parameter, secondary_parameters);
	  break;
	case SHELL_COMMAND_ID_HELP:
	  handle_cmd_help(primary_parameter, secondary_parameters);
	  break;
	default:
	  printu("command '%s' is not known\n", cmd_p);
	  continue;
	}
#ifdef __ARCH_X86__
  } while(0);
#else
  } while(1);
#endif
  
  return 0;
}
