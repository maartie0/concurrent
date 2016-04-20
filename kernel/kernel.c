#include "kernel.h"

void create_shell();
int create_process_child(uint32_t parent_pid,ctx_t* ctx);
void create_empty_processes();
int get_available_pid();
void initialise_stack_locations();
void initialise_pcb_idle();
void initialise_pcb_shell();
int get_running_processes_number();
void print(char* x,int n);
void make_1(uint32_t parent_pid, ctx_t* ctx);

/* Since we *know* there will be 2 processes, stemming from the 2 user 
 * programs, we can 
 * 
 * - allocate a fixed-size process table (of PCBs), and use a pointer
 *   to keep track of which entry is currently executing, and
 * - employ a fixed-case of round-robin scheduling: no more processes
 *   can be created, and neither is able to complete.
 */


uint32_t stack_locations[3];
pcb_t pcb_idle;
pcb_t pcb_shell;
int total_processes = 5;
pcb_t pcb[ 5 ], *current = NULL;

void scheduler( ctx_t* ctx ) {
	int current_pid = current->pid;

  if(current_pid == -2 || get_running_processes_number() == 0){
    return;
  }


  if(current_pid == 0){
    memcpy( &pcb[ current_pid ].ctx, ctx, sizeof( ctx_t ) );
    memcpy( ctx, &pcb_shell.ctx, sizeof( ctx_t ) );
    current = &pcb_shell;
  }else if(current_pid == -1){
    memcpy( &pcb_shell.ctx, ctx, sizeof( ctx_t ) );
    memcpy( ctx, &pcb[ 0 ].ctx, sizeof( ctx_t ) );
    current = &pcb[ 0 ];
  }

 //  if(current_pid == -1){
 //    memcpy( &pcb_shell.ctx, ctx, sizeof( ctx_t ) );
 //    memcpy( ctx, &pcb[ 0 ].ctx, sizeof( ctx_t ) );
 //    current = &pcb[ 0 ];
 //  }else if(pcb[current_pid + 1].available == 0 && current_pid >= 0){
	// 	memcpy( &pcb[ current_pid ].ctx, ctx, sizeof( ctx_t ) );
 //    memcpy( ctx, &pcb[ current_pid + 1 ].ctx, sizeof( ctx_t ) );
 //    current = &pcb[ current_pid + 1 ];
	// }else{
	// 	memcpy( &pcb[ current_pid ].ctx, ctx, sizeof( ctx_t ) );
 //    memcpy( ctx, &pcb[ 0 ].ctx, sizeof( ctx_t ) );
 //    current = &pcb[ 0 ];
	// }

}

void kernel_handler_rst( ctx_t* ctx              ) { 


	/* Configure the mechanism for interrupt handling by
5 *
6 * - configuring timer st. it raises a ( periodic) interrupt for each
7 * timer tick ,
8 * - configuring GIC st. the selected interrupts are forwarded to the
9 * processor via the IRQ interrupt signal , then
10 * - enabling IRQ interrupts .
11 */

  TIMER0 -> Timer1Load = 0x00100000 ; // select period = 2^20 ticks ~= 1 sec
  TIMER0 -> Timer1Ctrl = 0x00000002 ; // select 32-bit timer
  TIMER0 -> Timer1Ctrl |= 0x00000040 ; // select periodic timer
  TIMER0 -> Timer1Ctrl |= 0x00000020 ; // enable timer interrupt
  TIMER0 -> Timer1Ctrl |= 0x00000080 ; // enable timer

  UART0->IMSC           |= 0x00000010; // enable UART    (Rx) interrupt
  UART0->CR              = 0x00000301; // enable UART (Tx+Rx)

  GICC0 ->PMR = 0x000000F0 ; // unmask all interrupts
  GICD0 -> ISENABLER [ 1 ] |= 0x00000010 ; // enable timer interrupt
  GICD0 -> ISENABLER[ 1 ] |= 0x00001000; // enable UART    (Rx) interrupt
  GICC0 ->CTLR = 0x00000001 ; // enable GIC interface
  GICD0 ->CTLR = 0x00000001 ; // enable GIC distributor

  

/*
  /* Once the PCBs are initialised, we (arbitrarily) select one to be
   * restored (i.e., executed) when the function then returns.
   */

   initialise_stack_locations();

   create_empty_processes();

   initialise_pcb_idle();
   initialise_pcb_shell();


// make idle current
  current = &pcb_idle; 
  memcpy( ctx, &current->ctx, sizeof( ctx_t ) );

   //current = &pcb[ 0 ]; memcpy( ctx, &current->ctx, sizeof( ctx_t ) );

   irq_enable ();

  return;
}

void kernel_handler_svc( ctx_t* ctx, uint32_t id ) { 
  /* Based on the identified encoded as an immediate operand in the
   * instruction, 
   *
   * - read  the arguments from preserved usr mode registers,
   * - perform whatever is appropriate for this system call,
   * - write any return value back to preserved usr mode registers.
   */

  switch( id ) {
    case 0x00 : { // yield()
      scheduler( ctx );
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
    case 0x02 : {
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
    case 0x03 : {
    	int pid = create_process_child(current->pid,ctx);
      //todo put pid in gpr
    	ctx -> gpr[ 0 ] = 100;
    }
    default   : { // unknown
      break; 
    }
  }

  return;
}

void print(char* x,int n){
  for( int i = 0; i < n; i++ ) {
    PL011_putc( UART0, *x++ );
  }
}

void kernel_handler_irq( ctx_t* ctx 		){

	// Read interrupt Id
	uint32_t id = GICC0 -> IAR;
  uint32_t volatile source = GIC_SOURCE_TIMER0;
  uint32_t volatile source2 = GIC_SOURCE_UART0;

	// Handle interrupt then reset Timer
	if ( id == GIC_SOURCE_TIMER0 ) {
    int volatile current_pid = current->pid;
 //   if(current_pid != -1){
      scheduler( ctx ); 
//    }
		TIMER0 -> Timer1IntClr = 0x01;
	}else if(id == GIC_SOURCE_UART0){
    PL011_putc(UART0,'\n');
    print("$bash: ",7);
    int current_pid = current->pid;
    
    if(current_pid == -2){
      memcpy( &pcb_idle.ctx, ctx, sizeof( ctx_t ) );
    }else if(current_pid == -1){
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

int get_available_pid(){
	for (int volatile i = 0; i < total_processes; i++)
	{
		if(pcb[i].available == 1) {
			pcb[i].available = 0;
			return i;
		}
	}
	return -1;
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
}


int create_process_child(uint32_t parent_pid, ctx_t* ctx){
	volatile uint32_t pid = get_available_pid();

	if(pid != -1){
		    memcpy( &pcb[ pid ].ctx, ctx, sizeof( ctx_t ) );
        pcb[ pid ].pid      = pid;
        pcb[ pid ].priority =   7;
        pcb[ pid ].ctx.gpr[0] = -1;
        pcb[ pid ].available = 0;
        return pid;
	}else{
		return 0;
	}

	
	
}


void initialise_pcb_idle(){
  memset( &pcb_idle, 0, sizeof( pcb_t ) );
  uint32_t pid = -2;
  pcb_idle.pid      = pid;
  pcb_idle.ctx.cpsr = 0x50;
  pcb_idle.ctx.pc   = ( uint32_t ) entry_idle ;
  pcb_idle.ctx.sp   = ( uint32_t )(  &tos_idle );
  pcb_idle.priority  = 0;
  pcb_idle.available = 0;
}

void initialise_stack_locations(){
	stack_locations[0] = ( uint32_t )(  &tos_P0 );
	stack_locations[1] = ( uint32_t )(  &tos_P1 );
	stack_locations[2] = ( uint32_t )(  &tos_P2 );
}

void create_empty_processes(){
	for (int i = 0; i < total_processes; i++)
	{
		memset( &pcb[ i ], 0, sizeof( pcb_t ) );
  		pcb[ i ].available = 1;
	}
}

int get_running_processes_number(){
  int total = 0;
  for (int i = 0; i < total_processes; i++)
  {
    if(pcb[i].available == 0) total++;
  }
  return total;
}

