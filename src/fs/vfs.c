#include <vfs.h>

extern struct file_system_type *registered_file_systems;
extern vfs_node *root_vfs_node;

int vfs_node_create (vfs_node *node, vfs_node_type type) {
  return VFS_OPERATION_NOTSUPPORTED;
}

int vfs_node_remove (vfs_node *node) {
  return VFS_OPERATION_NOTSUPPORTED;
}

int vfs_node_rename (vfs_node *node, char *name, int len) {
  return VFS_OPERATION_NOTSUPPORTED;
}

int vfs_node_move (vfs_node *node, vfs_node *current_loc, vfs_node *target_loc) {
  return VFS_OPERATION_NOTSUPPORTED;
}

int vfs_file_read (vfs_node *node, char *buffer, int len, int offset) {
  if (NULL == registered_file_systems)
	return -1;

  if (NULL == node)
	return -1;

  if (VFS_NODE_TYPE_FILE != node->type)
	return -2;

  return registered_file_systems->file_operations->read(node, buffer, len, offset);

}

int vfs_file_write (vfs_node *node, char *buffer, int len, int offset) {
  if (NULL == registered_file_systems)
	return -1;

  if (NULL == node)
	return -1;

  if (VFS_NODE_TYPE_FILE != node->type)
	return -2;

  return registered_file_systems->file_operations->write(node, buffer, len, offset);
}
