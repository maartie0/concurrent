#ifndef __IDLE_H
#define __IDLE_H

#include <stddef.h>
#include <stdint.h>

#include "libc.h"

// define symbols for idle entry point and top of stack
extern void (*entry_idle)(); 
extern uint32_t tos_idle;

#endif
