/*
 * isr-vector-stub.c
 *
 *  Created on: 8 дек. 2015 г.
 *      Author: alexey
 */

typedef void
(* const pHandler)(void);

extern unsigned int _estack;

void Reset_Handler();

__attribute__ ((section(".isr_vector_stub"),used))
pHandler isr_vectors_stub[2] =
{
	// Core Level - CM3
	(pHandler) &_estack, // The initial stack pointer
	Reset_Handler        // The reset handler
	// Other vectors are unnecessary
};

