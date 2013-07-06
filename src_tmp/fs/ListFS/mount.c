#include <vfs.h>
#include <printk.h>
#include "parser.h"

#define LISTFS_IMAGE_LOC 0xC8000000

static vfs_node* listfs_mount();
static void listfs_unmount();

struct fs_mount_operations listfs_mount_operations = {
  .file_system_mount = listfs_mount,
  .file_system_unmount = listfs_unmount,
};

static vfs_node* listfs_mount() {
  char *buffer = (char *)LISTFS_IMAGE_LOC;
  listfs_parser_set_buffer(buffer);
  listfs_parser_parse_buffer();
  return listfs_parser_get_root();
}

static void listfs_unmount() {
  return;
}

