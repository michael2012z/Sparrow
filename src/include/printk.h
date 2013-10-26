#ifndef _PRINTK_H_
#define _PRINTK_H_

/* Sub-system trace */
#define PR_SS_INI 	0x00
#define PR_SS_MM 	0x01
#define PR_SS_PROC 	0x02
#define PR_SS_FS 	0x03
#define PR_SS_IRQ 	0x04

/* Trace level */
#define PR_LVL_ERR 	0x10
#define PR_LVL_WRN 	0x11
#define PR_LVL_INF 	0x12
#define PR_LVL_INFO		PR_LVL_INF
#define PR_LVL_ERROR	PR_LVL_ERR

#define PR_LVL_DBG0	0x20
#define PR_LVL_DBG1	0x21
#define PR_LVL_DBG2	0x22
#define PR_LVL_DBG3	0x23
#define PR_LVL_DBG4	0x24
#define PR_LVL_DBG5	0x25
#define PR_LVL_DBG6	0x26
#define PR_LVL_DBG7	0x27
#define PR_LVL_DBG8	0x28
#define PR_LVL_DBG9	0x29

/* Trace Guide:
 * Initialization (INI):
 *   DBG0 -- General
 *   DBG5 -- Shell
 * Memory Management (MM):
 *   DBG0 -- Boot Memory Allocator initialization and allocation
 *   DBG1 -- Boot Memory Allocator de-allocation
 *   DBG2 -- Page Allocator initialization and allocation
 *   DBG3 -- Page Allocator de-allocation
 *   DBG4 -- Slab Allocator initialization and allocation
 *   DBG5 -- Slab Allocator de-allocation
 *   DBG6 -- Paging
 *   DBG7 -- Memory Mapping/MMU operation
 *   DBG9 -- Dump memory content
 *
 * File System (FS):
 *   DBG0 -- Mount
 *   DBG1 -- Unmount
 *   DBG2 -- ListFS parsing
 *   DBG3 -- File reading
 *
 * Process & Schedule:
 *   DBG0 -- Process initialization
 *   DBG1 -- Program load
 *   DBG3 -- Program actions
 *   DBG5 -- Schedule actions
 *   DBG6 -- Schedule algorithm
 *
 * IRQ:
 *   DBG0 -- IRQ setup
 *   DBG1 -- IRQ handling: indication
 *   DBG2 -- IRQ handling: detail
 *   DBG5 -- Prefetch/Data abort handling
 *   DBG6 -- System Call
 */

void printk(int a, int b, const char *fmt, ...);
void printu(const char *fmt, ...);
void prints(char *string, int length);
void print_memory_byte (unsigned long start, unsigned long end);
void test_printk(void);

void printk_disable();
void printk_enable();

#define CRASHIF(condition) {if (condition) { printk(PR_SS_INI, PR_LVL_ERR, "crash at %s : %d\n", __func__, __LINE__); while(1); }}

#endif
