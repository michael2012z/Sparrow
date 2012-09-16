#include <type.h>
#include "pid.h"

/* false -- available
 * true  -- allocated
 */
bool pid_array[64];

void initialize_pid() {
  int i;

  for (i = 0; i < 64; i ++) {
	pid_array[i] = false;
  }
}

int allocate_pid() {
  int i = -1;

  for (i = 0; i < 64; i ++)
	if (false == pid_array[i]) {
	  pid_array[i] = true;
	  return i;
	}

  return -1;  
}

void free_pid(int pid) {
  if ((pid >= 0) && (pid < 64))
	pid_array[pid] = false;
}
