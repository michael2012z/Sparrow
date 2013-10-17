#include <type.h>
#include <linkage.h>
#include <uart.h>
#include <printk.h>

void init_uart() {
  arm_init_uart();
}
