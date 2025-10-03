/*
 * Your code here for context_switch
 *
 * It should restore the register values in the Trapframe with the 'iret' instruction.
 * This exits the kernel and starts executing some environment's code.
 *
 * This function does not return.
 */

.globl context_switch;
context_switch:
	mov 4(%esp), %esp 	/* Salta direccion de retorno*/

	popal				/*eax, ecx, edx, ebx, oesp, ebp, esi, edi */

	pop %es
	pop %ds 

	add $8, %esp    	/* Salta tf_trapno y tf_err */

	iret 				/*eip, eflags, cs, esp */

spin:
	jmp spin
