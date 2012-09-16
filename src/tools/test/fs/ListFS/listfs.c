#include <vfs.h>

static int listfs_node_create (vfs_node *node, vfs_node_type type);
static int listfs_node_remove (vfs_node *node);
static int listfs_node_rename (vfs_node *node, char *name, int len);
static int listfs_node_move (vfs_node *node, vfs_node *current_loc, vfs_node *target_loc);
static int listfs_file_read (vfs_node *node, char *buffer, int len, int offset);
static int listfs_file_write (vfs_node *node, char *buffer, int len, int offset);

struct fs_node_operations listfs_node_operations = {
  .create = listfs_node_create,
  .remove = listfs_node_remove,
  .rename = listfs_node_rename,
  .move = listfs_node_move,
};

struct fs_file_operations listfs_file_operations = {
  .read = listfs_file_read,
  .write = listfs_file_write,
};

extern struct fs_mount_operations listfs_mount_operations;

struct file_system_type listfs_file_system_type = {
  .name = "ListFS",
  .node_operations = &listfs_node_operations,
  .file_operations = &listfs_file_operations,
  .mount_operations = &listfs_mount_operations,
};


static int listfs_node_create (vfs_node *node, vfs_node_type type) {
  return VFS_OPERATION_SUCCESS;  
}

static int listfs_node_remove (vfs_node *node) {
  return VFS_OPERATION_SUCCESS;
}

static int listfs_node_rename (vfs_node *node, char *name, int len) {
  return VFS_OPERATION_SUCCESS;
}

static int listfs_node_move (vfs_node *node, vfs_node *current_loc, vfs_node *target_loc) {
  return VFS_OPERATION_SUCCESS;
}

static int listfs_file_read (vfs_node *node, char *buffer, int len, int offset) {
  return 0;
}

static int listfs_file_write (vfs_node *node, char *buffer, int len, int offset) {
  return 0;
}

