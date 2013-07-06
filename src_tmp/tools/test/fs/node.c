#include <string.h>
#include <vfs.h>
#include <printk.h>
#include <mm.h>
#ifdef __ARCH_X86__
#include <stdio.h>
#include <stdlib.h>
#endif

vfs_node *vfs_new_node(vfs_node_type type) {
  if (VFS_NODE_TYPE_DIR == type)
	return vfs_new_dir();
  else if (VFS_NODE_TYPE_FILE == type)
	return vfs_new_file();
  else
	return NULL;
}

void vfs_delete_node(vfs_node *node) {
  if (VFS_NODE_TYPE_DIR == node->type) {
    struct list_head *current;
	vfs_node *child;
	list_for_each(current, &(node->dir.nodes)) {
	  child = list_entry(current,vfs_node,list);
	  vfs_delete_node(child);
	}
  }

#ifdef __ARCH_X86__
  free(node);
#else
  kfree(node);
#endif

  return;
}


vfs_node *vfs_new_dir() {
  vfs_node *new_node = NULL;

#ifdef __ARCH_X86__
  new_node = (vfs_node *)malloc(sizeof(vfs_node));
#else
  new_node = (vfs_node *)kmalloc(sizeof(vfs_node));
#endif

  memset(new_node, 0, sizeof(vfs_node));
  new_node->type = VFS_NODE_TYPE_DIR;
  INIT_LIST_HEAD(&(new_node->dir.nodes));
  return new_node;
}

vfs_node *vfs_new_file() {
  vfs_node *new_node = NULL;

#ifdef __ARCH_X86__
  new_node = (vfs_node *)malloc(sizeof(vfs_node));
#else
  new_node = (vfs_node *)kmalloc(sizeof(vfs_node));
#endif

  memset(new_node, 0, sizeof(vfs_node));
  new_node->type = VFS_NODE_TYPE_FILE;
  return new_node;
}

int vfs_add_node_to_dir(vfs_node *parent, vfs_node *node) {
  if (VFS_NODE_TYPE_DIR != parent->type)
	return -1;
  
  node->parent = parent;
  list_add_tail(&(node->list), &(parent->dir.nodes));
  return 0;
}

int vfs_remove_node_from_dir(vfs_node *parent, vfs_node *node) {
  node->parent = NULL;
  if (list_empty(&(parent->dir.nodes)))
	return -1;

  list_del(&(node->list));

  return 0;
}

void vfs_print_dir(vfs_node *dir) {
  if (VFS_NODE_TYPE_DIR == dir->type)
	printk(PR_SS_FS, PR_LVL_INF, "DIR: name = %s\n", dir->name);
  else
	printk(PR_SS_FS, PR_LVL_INF, "Node is not a DIR.\n");
}

void vfs_print_file(vfs_node *file) {
  if (VFS_NODE_TYPE_FILE == file->type)
	printk(PR_SS_FS, PR_LVL_INF, "FILE: name = %s, size = %d, addr = 0x%x\n", file->name, file->file.size, (unsigned int)file->file.addr);
  else
	printk(PR_SS_FS, PR_LVL_INF, "Node is not a FILE.\n");
}

static void vfs_print_indent(int indent) {
  while (indent-- > 0)
    printk(PR_SS_FS, PR_LVL_INF, " ");
}

static void _vfs_print_node_re(vfs_node *node, int indent) {
  if (NULL == node)
    return;

  vfs_print_indent(indent);

  if (VFS_NODE_TYPE_FILE == node->type) {
    vfs_print_file(node);
  } else if (VFS_NODE_TYPE_DIR == node->type) {
    struct list_head *current;
	vfs_node *child;
	vfs_print_dir(node);
	list_for_each(current, &(node->dir.nodes)) {
	  child = list_entry(current,vfs_node,list);
	  _vfs_print_node_re(child, (indent+1));
    }
	
  }
}

void vfs_print_node_re(vfs_node *node) {
  _vfs_print_node_re(node, 0);
}
