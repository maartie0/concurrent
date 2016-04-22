#include "kernel.h"

void create_shell();
int create_process_child(uint32_t parent_pid,ctx_t* ctx);
void create_empty_processes();
int get_available_pid();
void initialise_stack_locations();
void initialise_pcb_shell();
int get_running_processes_number();
void initialise_input();
void print(char* x,int n);
void make_1(uint32_t parent_pid, ctx_t* ctx);
void initialise_input();
void shell_yield();
void scheduler(ctx_t* ctx);
int get_next_process_pid();
int age_process();

uint32_t stack_locations[5];
pcb_t pcb_shell;
int total_processes = 5;
pcb_t pcb[ 5 ], *current = NULL;



void kernel_handler_rst( ctx_t* ctx              ) { 
   initialise_input();
   initialise_stack_locations();
   create_empty_processes();
   initialise_pcb_shell();
   current = &pcb_shell; 
   memcpy( ctx, &current->ctx, sizeof( ctx_t ) );
   irq_enable ();
  return;
}

void kernel_handler_svc( ctx_t* ctx, uint32_t id ) { 

  switch( id ) {
    case 0x00 : { // yield()
      //scheduler( ctx );
      shell_yield(ctx);
      break;
    }


    case 0x01 : { // write( fd, x, n )
      int   fd = ( int   )( ctx->gpr[ 0 ] );  
      char*  x = ( char* )( ctx->gpr[ 1 ] );  
      int    n = ( int   )( ctx->gpr[ 2 ] ); 

      for( int i = 0; i < n; i++ ) {
        PL011_putc( UART0, *x++ );
      }
      
      ctx->gpr[ 0 ] = n;
      break;
    }


    case 0x02 : { //read
    	char* x = ( char*   )( ctx->gpr[ 0 ] );
    	int i = 0;
    	int reading = 1;

    	while(reading){
    		x[i] = PL011_getc( UART0 );
    		if(x[i] == '\r'){
    			reading = 0;
    		}
    		PL011_putc(UART0,x[i]);
    		i++;
    	}

    	ctx -> gpr[ 0 ] = i - 1; 
    	PL011_putc(UART0,'\n');
    	break;
    }


    case 0x03 : { //fork
    	int pid = create_process_child(current->pid,ctx);
      //todo put pid in gpr
    	ctx -> gpr[ 0 ] = pid;
      break;
    }


    case 0x04 : { //exit
      int current_pid = current->pid;
      memset( &pcb[current_pid], 0, sizeof( pcb_t ) );
      pcb[current_pid].available = 1;
      scheduler(ctx);
      pcb[current_pid].pid  = 0;
      pcb[current_pid].ctx.cpsr = 0;
      pcb[current_pid].ctx.pc   = 0 ;
      pcb[current_pid].ctx.sp   = 0;
      pcb[current_pid].priority  = 0;
      break;
    }
    default   : { // unknown
      break; 
    }
  }

  return;
}



void kernel_handler_irq( ctx_t* ctx 		){

	// Read interrupt Id
	uint32_t id = GICC0 -> IAR;

	// Handle interrupt then reset Timer
	if ( id == GIC_SOURCE_TIMER0 ) {
    int volatile current_pid = current->pid;
    age_process();
    scheduler( ctx ); 
		TIMER0 -> Timer1IntClr = 0x01;
	}
  else if(id == GIC_SOURCE_UART0){

     PL011_putc(UART0,'\n');
    // print("$bash: ",7);
    int current_pid = current->pid;

    if(current_pid == -1){
      memcpy( &pcb_shell.ctx, ctx, sizeof( ctx_t ) );
    }else{
      memcpy( &pcb[ current_pid ].ctx, ctx, sizeof( ctx_t ) );
    }
    memcpy( ctx, &pcb_shell.ctx, sizeof( ctx_t ) );

    current = &pcb_shell;

    UART0->ICR = 0x10;
  }
	
	GICC0 -> EOIR = id;
}

void scheduler( ctx_t* ctx ) {
  int current_pid = current->pid;
  int volatile running = get_running_processes_number();
  int next_pid;

  if(running > 0 && current_pid == -1){//in shell with other programs running
    return;
  }else if(running == 0 && current_pid == -1){//in shell with no other programs running
    return;
  }else if(running > 0 && current_pid != -1){//in program with other programs running
    next_pid = get_next_process_pid();
    memcpy( &pcb[ current_pid ].ctx, ctx, sizeof( ctx_t ) );
    memcpy( ctx, &pcb[ next_pid ].ctx, sizeof( ctx_t ) );
    current = &pcb[ next_pid ];
  }else if(running == 1 && current_pid != -1){//in program with no other programs running
    return;
  }


  /*if(running == 0 || current_pid == -1){
    memcpy( &pcb[ current_pid ].ctx, ctx, sizeof( ctx_t ) );
    memcpy( ctx, &pcb_shell.ctx, sizeof( ctx_t ) );
    current = &pcb_shell;
  }else if(pcb[current_pid + 1].available == 0 && current_pid >= 0){
     memcpy( &pcb[ current_pid ].ctx, ctx, sizeof( ctx_t ) );
     memcpy( ctx, &pcb[ current_pid + 1 ].ctx, sizeof( ctx_t ) );
     current = &pcb[ current_pid + 1 ];
  }else{
     memcpy( &pcb[ current_pid ].ctx, ctx, sizeof( ctx_t ) );
     memcpy( ctx, &pcb[ 0 ].ctx, sizeof( ctx_t ) );
     current = &pcb[ 0 ];
  }*/

}

void initialise_stack_locations(){
	stack_locations[0] = ( uint32_t )(  &tos_0 );
	stack_locations[1] = ( uint32_t )(  &tos_1 );
	stack_locations[2] = ( uint32_t )(  &tos_2 );
  stack_locations[3] = ( uint32_t )(  &tos_3 );
  stack_locations[4] = ( uint32_t )(  &tos_4 );
}

void create_empty_processes(){
	for (int i = 0; i < total_processes; i++)
	{
		memset( &pcb[ i ], 0, sizeof( pcb_t ) );
  		pcb[ i ].available = 1;
	}
}

void print(char* x,int n){
  for( int i = 0; i < n; i++ ) {
    PL011_putc( UART0, *x++ );
  }
}

void shell_yield(ctx_t* ctx){
    int pid = get_next_process_pid();
    if(get_running_processes_number() != 0 && pid != -1){
      memcpy( &pcb_shell.ctx, ctx, sizeof( ctx_t ) );
      memcpy( ctx, &pcb[ pid ].ctx, sizeof( ctx_t ) );
      current = &pcb[ pid ];
    }
    
}

int get_available_pid(){
  for (int volatile i = 0; i < total_processes; i++)
  {
    if(pcb[i].available == 1) {
      pcb[i].available = 0;
      return i;
    }
  }
  return -2;
}

int get_running_processes_number(){
  int total = 0;
  for (int i = 0; i < total_processes; i++)
  {
    if(pcb[i].available == 0) total++;
  }
  return total;
}

int age_process(){
  if(current->age == current->max_age){
    current->age = 0;
    return 0;
  }else{
    current->age++;
    return 1;
  }
}

int get_next_process_pid(){
  int best = 0;
  int pid;
  for (int i = 0; i < total_processes; i++)
  {
    if(pcb[i].available == 0){
      if(pcb[i].priority - pcb[i].age >= best){
        best = pcb[i].priority - pcb[i].age;
        pid = i;
      }
    }
  }
  if(best == 0) return -1;
  else return pid;
}

int create_process_child(uint32_t parent_pid, ctx_t* ctx){
  volatile uint32_t pid = get_available_pid();

  if(pid != -2){
        memcpy( (void*) stack_locations[pid] - 0x00001000,(void*)  &tos_shell -0x00001000, 0x00001000);
        memcpy( &pcb[ pid ].ctx, ctx, sizeof( ctx_t ) );
        pcb[ pid ].pid      = pid;
        pcb[ pid ].priority =   7;
        pcb[ pid ].ctx.gpr[0] = -1;
        pcb[ pid ].ctx.cpsr = 0x50;
        pcb[ pid ].ctx.sp = stack_locations[pid];
        pcb[ pid ].available = 0;
        pcb[ pid ].age = 0;
        pcb[ pid ].max_age = 6;
        return pid;
  }else{
    return -2;
  }
}

void initialise_pcb_shell(){
  memset( &pcb_shell, 0, sizeof( pcb_t ) );
  uint32_t pid = -1;
  pcb_shell.pid      = pid;
  pcb_shell.ctx.cpsr = 0x50;
  pcb_shell.ctx.pc   = ( uint32_t ) entry_shell ;
  pcb_shell.ctx.sp   = ( uint32_t ) &tos_shell ;
  pcb_shell.priority  = 7;
  pcb_shell.available = 0;
  pcb_shell.age = 0;
  pcb_shell.max_age = 6;
}

void initialise_input(){
  // TIMER0 -> Timer1Load = 0x00100000 ; // select period = 2^20 ticks ~= 1 sec
  TIMER0 -> Timer1Load = 0x00100000 ; // select period = 2^20 ticks ~= 1 sec
  TIMER0 -> Timer1Ctrl = 0x00000002 ; // select 32-bit timer
  TIMER0 -> Timer1Ctrl |= 0x00000040 ; // select periodic timer
  TIMER0 -> Timer1Ctrl |= 0x00000020 ; // enable timer interrupt
  TIMER0 -> Timer1Ctrl |= 0x00000080 ; // enable timer

  UART0->IMSC           |= 0x00000010; // enable UART    (Rx) interrupt
  UART0->CR              = 0x00000301; // enable UART (Tx+Rx)

  GICC0 ->PMR = 0x000000F0 ; // unmask all interrupts
  GICD0 -> ISENABLER [ 1 ] |= 0x00001010 ; // enable timer interrupt
 // GICD0 -> ISENABLER[ 1 ] |= 0x00001000; // enable UART    (Rx) interrupt
  GICC0 ->CTLR = 0x00000001 ; // enable GIC interface
  GICD0 ->CTLR = 0x00000001 ; // enable GIC distributor
}


