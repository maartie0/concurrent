#include "kernel.h"

/* Since we *know* there will be 2 processes, stemming from the 2 user 
 * programs, we can 
 * 
 * - allocate a fixed-size process table (of PCBs), and use a pointer
 *   to keep track of which entry is currently executing, and
 * - employ a fixed-case of round-robin scheduling: no more processes
 *   can be created, and neither is able to complete.
 */

pcb_t pcb[ 3 ], *current = NULL;

void scheduler( ctx_t* ctx ) {
  if      ( current == &pcb[ 0 ] ) {
    memcpy( &pcb[ 0 ].ctx, ctx, sizeof( ctx_t ) );
    memcpy( ctx, &pcb[ 1 ].ctx, sizeof( ctx_t ) );
    current = &pcb[ 1 ];
  }
  else if ( current == &pcb[ 1 ] ) {
    memcpy( &pcb[ 1 ].ctx, ctx, sizeof( ctx_t ) );
    memcpy( ctx, &pcb[ 2 ].ctx, sizeof( ctx_t ) );
    current = &pcb[ 2 ];
  } else if ( current == &pcb[ 2 ] ) {
    memcpy( &pcb[ 2 ].ctx, ctx, sizeof( ctx_t ) );
    memcpy( ctx, &pcb[ 0 ].ctx, sizeof( ctx_t ) );
    current = &pcb[ 0 ];
  }
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

  GICC0 ->PMR = 0x000000F0 ; // unmask all interrupts
  GICD0 -> ISENABLER [ 1 ] |= 0x00000010 ; // enable timer interrupt
  GICC0 ->CTLR = 0x00000001 ; // enable GIC interface
  GICD0 ->CTLR = 0x00000001 ; // enable GIC distributor

  

  /* Initialise PCBs representing processes stemming from execution of
   * the two user programs.  Note in each case that
   *    
   * - the CPSR value of 0x50 means the processor is switched into USR 
   *   mode, with IRQ interrupts enabled, and
   * - the PC and SP values matche the entry point and top of stack. 
   */

  memset( &pcb[ 0 ], 0, sizeof( pcb_t ) );
  pcb[ 0 ].pid      = 0;
  pcb[ 0 ].ctx.cpsr = 0x50;
  pcb[ 0 ].ctx.pc   = ( uint32_t )( entry_P0 );
  pcb[ 0 ].ctx.sp   = ( uint32_t )(  &tos_P0 );

  memset( &pcb[ 1 ], 0, sizeof( pcb_t ) );
  pcb[ 1 ].pid      = 1;
  pcb[ 1 ].ctx.cpsr = 0x50;
  pcb[ 1 ].ctx.pc   = ( uint32_t )( entry_P1 );
  pcb[ 1 ].ctx.sp   = ( uint32_t )(  &tos_P1 );

  memset( &pcb[ 2 ], 0, sizeof( pcb_t ) );
  pcb[ 2 ].pid      = 2; //changed from 1 to 2 when it worked so might have to change to 1 again if breaks
  pcb[ 2 ].ctx.cpsr = 0x50;
  pcb[ 2 ].ctx.pc   = ( uint32_t )( entry_P2 );
  pcb[ 2 ].ctx.sp   = ( uint32_t )(  &tos_P2 );

  /* Once the PCBs are initialised, we (arbitrarily) select one to be
   * restored (i.e., executed) when the function then returns.
   */

  current = &pcb[ 0 ]; memcpy( ctx, &current->ctx, sizeof( ctx_t ) );

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
		//PL011_putc( UART0, 'T' );
		//age_Time +=1 ;
		
		/*if ( age_Time >= max_Age ) {
			age_process();
			age_Time = 0;
		} */

		scheduler( ctx ); 
		TIMER0 -> Timer1IntClr = 0x01;
	}

	GICC0 -> EOIR = id;
}


