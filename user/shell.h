#ifndef __shell_H
#define __shell_H

#include <stddef.h>
#include <stdint.h>

#include "libc.h"

// define symbols for P0 entry point and top of stack
extern void (*entry_shell)(); 
extern uint32_t tos_shell;

#endif
