#include <vfs.h>
#include <string.h>
#include <date.h>
#include "parser.h"
#include <printk.h>
#ifdef __ARCH_X86__
#include <stdio.h>
#include <stdlib.h>
#endif

static void listfs_parser_tag_dir_head_found (char *buffer);
static void listfs_parser_tag_dir_tail_found (char *buffer);
static void listfs_parser_tag_file_head_found (char *buffer);
static void listfs_parser_tag_file_tail_found (char *buffer);
static void listfs_parser_tag_name_head_found (char *buffer);
static void listfs_parser_tag_name_tail_found (char *buffer);
static void listfs_parser_tag_date_head_found (char *buffer);
static void listfs_parser_tag_date_tail_found (char *buffer);
static void listfs_parser_tag_offset_head_found (char *buffer);
static void listfs_parser_tag_offset_tail_found (char *buffer);
static void listfs_parser_tag_size_head_found (char *buffer);
static void listfs_parser_tag_size_tail_found (char *buffer);

static struct listfs_parser_element elements[] ={
  {LISTFS_TAG_DIR_HEAD, "<dir>", listfs_parser_tag_dir_head_found},
  {LISTFS_TAG_DIR_TAIL, "</dir>", listfs_parser_tag_dir_tail_found},
  {LISTFS_TAG_FILE_HEAD, "<file>", listfs_parser_tag_file_head_found},
  {LISTFS_TAG_FILE_TAIL, "</file>", listfs_parser_tag_file_tail_found},
  {LISTFS_TAG_NAME_HEAD, "<name>", listfs_parser_tag_name_head_found},
  {LISTFS_TAG_NAME_TAIL, "</name>", listfs_parser_tag_name_tail_found},
  {LISTFS_TAG_DATE_HEAD, "<date>", listfs_parser_tag_date_head_found},
  {LISTFS_TAG_DATE_TAIL, "</date>", listfs_parser_tag_date_tail_found},
  {LISTFS_TAG_OFFSET_HEAD, "<offset>", listfs_parser_tag_offset_head_found},
  {LISTFS_TAG_OFFSET_TAIL, "</offset>", listfs_parser_tag_offset_tail_found},
  {LISTFS_TAG_SIZE_HEAD, "<size>", listfs_parser_tag_size_head_found},
  {LISTFS_TAG_SIZE_TAIL, "</size>", listfs_parser_tag_size_tail_found},
};

static char *listfs_parser_buffer = NULL;
static int listfs_parser_buffer_len = 0;
static char saved_line[256];
static vfs_node *root_node = NULL;
static vfs_node *current_node = NULL;

void listfs_parser_set_buffer(char *buffer) {
  /* check parameters */
  if (NULL == buffer)
	return;

  listfs_parser_buffer = buffer;
  listfs_parser_buffer_len = 0;
  memset(saved_line, 0, 256);
  root_node = NULL;
  current_node = NULL;
}

vfs_node* listfs_parser_get_root() {
  return root_node;
}

static listfs_tag_id parse_line(char *line) {
  listfs_tag_id tag_id;
  struct listfs_parser_element *element;
  int i = 0;

/*  printk(PR_SS_FS, PR_LVL_DBG2, "parse_line(): line = %s\n", line); */

  for (i=0; i < (sizeof(elements)/sizeof(struct listfs_parser_element)); i++) {
	element = &elements[i];
	if ((NULL != element) && (strcmp(line, element->tag) == 0) && (NULL != element->callback)) { /* tag found */
	  element->callback(saved_line);
	}
  }

  if ((sizeof(elements)/sizeof(struct listfs_parser_element)) == i) { /* it's a text line */
	memset(saved_line, 0, 256);
	memcpy(saved_line, line, strlen(line));
	tag_id = LISTFS_TAG_NONE;
  } else {
	tag_id = element->id;
  }

  return tag_id;
}

void listfs_parser_parse_buffer() {
  char line[256] = {(char)0};
  char *cursor = listfs_parser_buffer;
  char *line_cursor = line;

  /* parse the first line, get list length. */
  while ('\n' != *cursor) {
	*(line_cursor++) = *(cursor++);
  }
  cursor++; 	/* skip '\n' */

  printk(PR_SS_FS, PR_LVL_DBG2, "first line captured: %s\n", line);

  /* first line was read */
  *(line_cursor-3) = '\0';
  listfs_parser_buffer_len = atoi((line+4));

  printk(PR_SS_FS, PR_LVL_DBG2, "index file length: %d\n", listfs_parser_buffer_len);

  /* scan lines */
  while (cursor < (listfs_parser_buffer + listfs_parser_buffer_len)) {
    line_cursor = line;
    memset(line, 0, 256);
	for(; '\n' != *cursor; cursor++, line_cursor++) {
	  if ((line_cursor - line) >= 256)
		return;
	  *line_cursor = *cursor;
	}
    cursor++; 	/* skip '\n' */
    /* printk(PR_SS_FS, PR_LVL_DBG2, "a line captured: %s\n", line); */
	line_cursor = strim(line);
	parse_line(line_cursor);
  }

}


static void listfs_parser_tag_dir_head_found (char *buffer) {
  /* make a new node
   * set type as dir
   * if root is NULL, set new node as root
   * else, add new node into current node
   * set new node as current node
   */
  vfs_node *new_node = NULL;
  new_node = vfs_new_dir();

  if (NULL == root_node)
	root_node = new_node;
  else
	vfs_add_node_to_dir(current_node, new_node);

  current_node = new_node;
  
  printk(PR_SS_FS, PR_LVL_DBG2, "tag <dir> found, buffer = %s\n", buffer);
  return;
}

static void listfs_parser_tag_dir_tail_found (char *buffer) {
  /* if current node has parrent, set current node to parrent
   * else, do nothing (current node is root)
   */
  if (NULL != current_node->parent)
	current_node = current_node->parent;

  printk(PR_SS_FS, PR_LVL_DBG2, "tag </dir> found, buffer = %s\n", buffer);
}

static void listfs_parser_tag_file_head_found (char *buffer) {
  /* make a new node
   * set type as file
   * add new node into current node
   * set new node as current node
   */
  vfs_node *new_node = NULL;
  new_node = vfs_new_file();
  vfs_add_node_to_dir(current_node, new_node);
  current_node = new_node;

  printk(PR_SS_FS, PR_LVL_DBG2, "tag <file> found, buffer = %s\n", buffer);
  return;
}

static void listfs_parser_tag_file_tail_found (char *buffer) {
  /* set current node to parrent */
  current_node = current_node->parent;
  printk(PR_SS_FS, PR_LVL_DBG2, "tag </file> found, buffer = %s\n", buffer);
}

static void listfs_parser_tag_name_head_found (char *buffer) { 
  printk(PR_SS_FS, PR_LVL_DBG2, "tag <name> found, buffer = %s\n", buffer);
}

static void listfs_parser_tag_name_tail_found (char *buffer) {
  /* set saved_line as the name of current node */
  memcpy(current_node->name, buffer, 15);
  printk(PR_SS_FS, PR_LVL_DBG2, "tag </name> found, buffer = %s\n", buffer);
}

static void listfs_parser_tag_date_head_found (char *buffer) { 
  printk(PR_SS_FS, PR_LVL_DBG2, "tag <date> found, buffer = %s\n", buffer);
}

static void listfs_parser_tag_date_tail_found (char *buffer) {
  /* parse saved_line as date, and set it to current node */
  current_node->date = date_parse_format_iso(buffer);
  printk(PR_SS_FS, PR_LVL_DBG2, "tag </date> found, buffer = %s\n", buffer);
}

static void listfs_parser_tag_offset_head_found (char *buffer) { 
  printk(PR_SS_FS, PR_LVL_DBG2, "tag <offset> found, buffer = %s\n", buffer);
}

static void listfs_parser_tag_offset_tail_found (char *buffer) {
  /* parse saved_line as int, and set it as offset of current node */
  current_node->file.addr = atoi(buffer) + listfs_parser_buffer + listfs_parser_buffer_len;
  printk(PR_SS_FS, PR_LVL_DBG2, "tag </offset> found, buffer = %s\n", buffer);
}

static void listfs_parser_tag_size_head_found (char *buffer) { 
  printk(PR_SS_FS, PR_LVL_DBG2, "tag <size> found, buffer = %s\n", buffer);
}

static void listfs_parser_tag_size_tail_found (char *buffer) {
  /* parse saved_line as int, and set it as size of current node */
  current_node->file.size = atoi(buffer);
  printk(PR_SS_FS, PR_LVL_DBG2, "tag </size> found, buffer = %s\n", buffer);
}
