#ifndef _LISTFS_PARSER_H_
#define _LISTFS_PARSER_H_

typedef enum {
  LISTFS_TAG_DIR_HEAD,
  LISTFS_TAG_DIR_TAIL,
  LISTFS_TAG_FILE_HEAD,
  LISTFS_TAG_FILE_TAIL,
  LISTFS_TAG_NAME_HEAD,
  LISTFS_TAG_NAME_TAIL,
  LISTFS_TAG_DATE_HEAD,
  LISTFS_TAG_DATE_TAIL,
  LISTFS_TAG_OFFSET_HEAD,
  LISTFS_TAG_OFFSET_TAIL,
  LISTFS_TAG_SIZE_HEAD,
  LISTFS_TAG_SIZE_TAIL,
  LISTFS_TAG_NONE,
} listfs_tag_id;

struct listfs_parser_element {
  listfs_tag_id id;
  char *tag;
  void (*callback)(char*);
};

void listfs_parser_set_buffer(char *buffer);
void listfs_parser_parse_buffer();
vfs_node* listfs_parser_get_root();

#endif /* _LISTFS_PARSER_H_ */
