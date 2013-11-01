#ifndef _PROCESS_H_
#define _PROCESS_H_

#include <mm.h>
#include <mmap.h>
#include <time.h>
#include <type.h>
#include <sched.h>
#include <ptrace.h>

extern union thread_union init_thread_union;

/* process state */
#define PROCESS_STATE_INVALID		0
#define PROCESS_STATE_READY		1
#define PROCESS_STATE_RUNNING		2
#define PROCESS_STATE_WAITING		3
#define PROCESS_STATE_DEAD		4

#define PROCESS_WAITING_TYPE_INVALID		0
#define PROCESS_WAITING_TYPE_INPUT		1
#define PROCESS_WAITING_TYPE_TIME		2
#define PROCESS_WAITING_TYPE_THREAD		3

struct sched_entity {
  /* state date */
  int state;
  int waiting_type;
  int blocking_pid; /* the process I'm waiting for */
  int blocked_pid;  /* the process waiting for me */
  /* timeing data */
  u64 wake_up_jiffy; /* the target jiffy that task should be waken up */
  u64 sum_exec_runtime;
  u64 vruntime;
  u64 continuous_ticks;
  u64 accumulated_ticks;
  /* list chain */
  struct list_head queue_entry;
};

struct task_struct {
  void *stack;
  unsigned int flags;
  struct pt_regs regs;
  bool kernel_thread;
  int priority;
  struct mm_struct mm;
  int pid;
  struct timespec start_time;
  char *elf_file_name;
  char *parameters[4];
  struct sched_entity sched_en;
};

enum sched_enqueue_flag {
  sched_enqueue_flag_new,
  sched_enqueue_flag_timeout,
  sched_enqueue_flag_sleep
};

enum sched_dequeue_flag {
  sched_dequeue_flag_exit,
  sched_dequeue_flag_chosen,
  sched_dequeue_flag_wakeup
};

/* scheduler interface */
struct sched_class {
  void (*init) ();
  void (*enqueue_task) (struct task_struct *p, enum sched_enqueue_flag flag);
  void (*dequeue_task) (struct task_struct *p);
  int (*need_to_reschedule) (struct task_struct *p);
  void (*check_preempt_curr) (struct task_struct *p);
  struct task_struct * (*pick_next_task) ();
  void (*task_tick) (struct task_struct *p);
  struct task_struct * (*find_task_by_pid) (int pid);
  void (*dump) ();
};


struct cpu_context_save {
	__u32	r4;
	__u32	r5;
	__u32	r6;
	__u32	r7;
	__u32	r8;
	__u32	r9;
	__u32	sl;
	__u32	fp;
	__u32	sp;
	__u32	pc;
	__u32	extra[2];		/* Xscale 'acc' register, etc */
};

struct thread_info {
	struct task_struct	*task;		/* main task structure */
	__u32			cpu_domain;	/* cpu domain */
	unsigned long		tp_value;
	struct cpu_context_save	cpu_context;	/* cpu context */
};


#define task_thread_info(task)	((struct thread_info *)(task)->stack)
#define task_stack_page(task)	((task)->stack)
#define task_pt_regs(p) \
	((struct pt_regs *)(THREAD_START_SP + task_stack_page(p)) - 1)


union thread_union {
	struct thread_info thread_info;
	unsigned long stack[THREAD_SIZE/sizeof(long)];
};


void process_test();
void initialize_process();
int create_kernel_thread(int (*fn)(void *));
int create_user_thread(int (*fn)(char *), char *elf_file_name, char **parameters);
int create_process(struct file *filep);
int execute_binary(struct task_struct *task, struct file *filep);
void destroy_process(struct task_struct *task);
void run_kernel_process(char *init_filename);

void destroy_user_thread(struct task_struct *task);
void process_cleaner_init();
int process_cleaner(void *unused);


bool is_scheduler_ready();
void update_task_on_tick();
bool check_should_schedule();
void enqueue_task(struct task_struct *task, enum sched_enqueue_flag flag);
void dequeue_task(struct task_struct *task);
void schedule();
void schedule_initialize();
struct task_struct * find_task_by_pid(int pid);

void cpu_idle();
void arm_health_check(void);
unsigned int arm_calc_kernel_domain();
int arm_kernel_execve(char *filename, char *const argv[], char *const envp[]);
void print_regs (struct pt_regs* regs);
void cpu_v6_switch_mm(unsigned long, int);
void arm_create_kernel_thread(int (*fn)(void *), void *arg, struct pt_regs *regs);
void arm_create_user_thread(int (*fn)(char *), void *arg, struct pt_regs *regs);
void __switch_to(struct task_struct *prev, struct thread_info *prev_thread, struct thread_info *next_thread);

#endif /* _PROCESS_H_ */
