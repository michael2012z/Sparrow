/* Assembly demo for Linu.
 */

.text
	.align 4
	.global start
	.type start, function

start:
	mov r0, #0xff
	mov r1, #0xee		
	mov r2, #0xdd
	mov r3, #0xcc
	mov r4, r3

.data
	.align 4
