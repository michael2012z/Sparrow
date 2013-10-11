
unsigned long sim_heap_start = 0;
unsigned long sim_heap_end = 0;

/*  
 * If brk == 0, return current heap border
 */
unsigned long _brk(unsigned long brk) {
  if (0 == brk)
	return sim_heap_end;
  else {
	sim_heap_end = (brk + 0x1000 - 1) & 0xfffff000;
	return sim_heap_end;
  }
}
