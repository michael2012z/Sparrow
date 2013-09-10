#include <vfs.h>
#include <string.h>
#ifdef __ARCH_X86__
#include <stdio.h>
#include <stdlib.h>
#endif

/* All registered file systems should be in a list,
 * but as there is only one kind of FS is supported,
 * so the list is simplified to a pointer.
 * Only one file system can be resigered.
 */
struct file_system_type *registered_file_systems = NULL;
vfs_node *root_vfs_node = NULL;

int register_file_system(struct file_system_type *type) {
  if (NULL == type)
	return -1;
  else {
	registered_file_systems = type;
	return 0;
  }
}

/* This function is not going to be used. Ignore it. */
int unregister_file_system(struct file_system_type *type) {
	return 0;
}

int mount_file_system(char *name) {
  if (NULL == registered_file_systems) {
	return -1;
  } else {
	if (strcmp(name, registered_file_systems->name) == 0) {
	  root_vfs_node = registered_file_systems->mount_operations->file_system_mount();
	} else {
	  return -2;
	}
  }
  vfs_print_node_re(root_vfs_node);
  return 0;
}

int unmount_file_system() {
  if (NULL == registered_file_systems) {
	return -1;
  } else {
	registered_file_systems->mount_operations->file_system_unmount();
	return 0;
  }
}
