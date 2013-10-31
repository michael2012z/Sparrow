#include <string.h>
#include <vfs.h>
#include <printk.h>
#include <mm.h>
#ifdef __ARCH_X86__
#include <stdio.h>
#include <stdlib.h>
#endif

extern vfs_node *root_vfs_node;

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

vfs_node* vfs_find_node(char *name) {
  char current_dir[16] = {(char)0};
  char *name_tmp=name, *current_tmp;
  vfs_node* current_node = root_vfs_node;

  printk(PR_SS_FS, PR_LVL_DBG3, "%s: finding node with full name: %s\n", __func__, name);

  if (NULL == name)
    return NULL;

  do {
    current_tmp = current_dir;
    // skip leading "/" or " ":
    while(('/' == *name_tmp) || (' ' == *name_tmp))
      name_tmp++;
    if (0 == *name_tmp) {
      printk(PR_SS_FS, PR_LVL_DBG3, "%s: end of path name each, return\n", __func__);
      break;
    }

    // copy current path name
    while(('/' != *name_tmp) && (0 != *name_tmp))
      *(current_tmp++) = *(name_tmp++); 
    *current_tmp = 0;

    printk(PR_SS_FS, PR_LVL_DBG3, "%s: finding node: %s\n", __func__, current_dir);

    // now current_dir hold current dir name
    current_node = vfs_find_in_node(current_node, current_dir);
    if (NULL == current_node)
      printk(PR_SS_FS, PR_LVL_INF, "%s: node %s was not found\n", __func__, name);
  } while(current_node);
  return current_node;
}

vfs_node* vfs_find_in_node(vfs_node *node, char *name) {
  printk(PR_SS_FS, PR_LVL_DBG3, "%s: finding %s in current node: %s\n", __func__, name, node->name);
  if (VFS_NODE_TYPE_FILE == node->type){  
    printk(PR_SS_FS, PR_LVL_DBG3, "%s: current node is file\n", __func__);
    if(0 == strcmp(node->name, name))
      return node;
    else
      return NULL;
  } else if (VFS_NODE_TYPE_DIR == node->type) {
    struct list_head *current;
    vfs_node *child;
    printk(PR_SS_FS, PR_LVL_DBG3, "%s: current node is dir\n", __func__);
    list_for_each(current, &(node->dir.nodes)) {
      child = list_entry(current,vfs_node,list);
      printk(PR_SS_FS, PR_LVL_DBG3, "%s: checking child: %s\n", __func__, child->name);
      if(0 == strcmp(child->name, name))
	return child;
    }
    return NULL;
  } else
    return NULL;
}

void vfs_print_dir(vfs_node *dir) {
  if (VFS_NODE_TYPE_DIR == dir->type) {
	if (root_vfs_node == dir)
	  printu(" / \n");
	else
	  printu("%s\n", dir->name);
  }
}

void vfs_print_file(vfs_node *file) {
  if (VFS_NODE_TYPE_FILE == file->type)
	printu("%s   size = %d   addr = 0x%x\n", file->name, file->file.size, (unsigned int)file->file.addr);
}

static void vfs_print_indent(int indent) {
  printu("  ");
  if (indent > 0) {
	while ((indent---1) > 0)
	  printu("     ");
	printu(" |-- ");
  }
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

void print_fs_tree() {
  vfs_print_node_re(root_vfs_node);
}

