#include "idle.h"
void idle();

void idle(){
	char* x = "1";
  while(1){
    write(0,x,1);
  }
}


void (*entry_idle)() = &idle;