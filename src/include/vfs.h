#ifndef _VFS_H_
#define _VFS_H_

#include <type.h>
#include <list.h>
#include <date.h>

/* VFS types */
typedef enum {
  VFS_OPERATION_SUCCESS,
  VFS_OPERATION_FAIL,
  VFS_OPERATION_NOTSUPPORTED,
} vfs_operation_error_code;

typedef enum {
  FILE_TYPE_REGULAR,
  FILE_TYPE_EXE,
} vfs_file_type;

struct _vfs_file {
  vfs_file_type type;
  void *addr;
  int size;
};
typedef struct _vfs_file vfs_file;

struct _vfs_dir {
  struct list_head nodes;
};
typedef struct _vfs_dir vfs_dir;

typedef enum {
  VFS_NODE_TYPE_FILE,
  VFS_NODE_TYPE_DIR,
} vfs_node_type;

struct _vfs_node {
  char name[16];
  date_t date;
  vfs_node_type type;
  struct _vfs_node *parent;
  struct list_head list;
  union {
	vfs_file file;
	vfs_dir dir;
  };
};
typedef struct _vfs_node vfs_node;

/* VFS interface */
int vfs_node_create (vfs_node *, vfs_node_type);
int vfs_node_remove (vfs_node *);
int vfs_node_rename (vfs_node *, char *, int);
int vfs_node_move (vfs_node *, vfs_node *, vfs_node *);
int vfs_file_read (vfs_node *, char *, int, int);
int vfs_file_write (vfs_node *, char *, int, int);


/* concrete FS interface */
struct fs_node_operations {
  int (*create) (vfs_node *, vfs_node_type);
  int (*remove) (vfs_node *);
  int (*rename) (vfs_node *, char *, int);
  int (*move) (vfs_node *, vfs_node *, vfs_node *);
};

struct fs_file_operations {
  int (*read) (vfs_node *, char *, int, int);
  int (*write) (vfs_node *, char *, int, int);
};

struct fs_mount_operations {
  vfs_node* (*file_system_mount)();
  void (*file_system_unmount)();
};

struct file_system_type {
  char name[16];
  struct fs_node_operations* node_operations;
  struct fs_file_operations* file_operations;
  struct fs_mount_operations* mount_operations;
};


/* Registration and Mounting */
int register_file_system(struct file_system_type *type);
int unregister_file_system(struct file_system_type *type);
int mount_file_system(char *name);
int unmount_file_system();

/* Node Management */
vfs_node *vfs_new_node(vfs_node_type type);
void vfs_delete_node(vfs_node *node);
vfs_node *vfs_new_dir();
vfs_node *vfs_new_file();
int vfs_add_node_to_dir(vfs_node *parent, vfs_node *node);
int vfs_remove_node_from_dir(vfs_node *parent, vfs_node *node);
vfs_node* vfs_find_node(char *name);
vfs_node* vfs_find_in_node(vfs_node *node, char *name);

/* Debug */
void vfs_print_file(vfs_node *file);
void vfs_print_dir(vfs_node *dir);
void vfs_print_node_re(vfs_node *node);

#endif /* _VFS_H_ */
