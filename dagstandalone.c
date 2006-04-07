/*  dagstandalone.c -- ARMulator RDP/RDI interface:  ARM6 Instruction Emulator.
    Copyright (C) 1994 Advanced RISC Machines Ltd.
 
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
 
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. */

/*********************************************************************/
/* Modified by Dave Gilbert (arcem@treblig.org) to be more           */
/* independent. Not very cleanly done - more of a hack than anything */
/*********************************************************************/

#include <sys/types.h>
#include <signal.h>

#ifdef LINUX
#include <linux/time.h>
#endif

#include "dagstandalone.h"
#include "armdefs.h"
#include "armrdi.h"
#include "dbg_conf.h"
#include "dbg_hif.h"
#include "dbg_rdi.h"

#include "ArcemConfig.h"

/* RDI interface */

static int rdi_state = 0;

/**************************************************************/
/* Signal handler that terminates excecution in the ARMulator */
/**************************************************************/
#ifndef WIN32
#ifndef AMIGA
static void dagstandalone_handlesignal(int sig) {
#ifdef DEBUG
  fprintf(stderr, "Terminate ARMulator - excecution\n");
#endif
#ifdef BENCHMARKEXIT
  printf("Emulated cycles = %ld\n", ARMul_Time);
  exit(0);
#endif
  if (sig != SIGUSR1) {
    fprintf(stderr,"Unsupported signal.\n");
    return;
  }
  armul_rdi.info(RDISignal_Stop, NULL, NULL);
}
#endif
#endif

/* Functions to be called by the emulator core - based on gdbhost.* */
static void myprint(void *arg,const char *format, va_list ap) {
  vfprintf(stderr,format, ap);
};


static void mypause(void *arg) {
  /* Should wait for the user to do something */
};


static void mywritec(void *arg, int c) {
  putchar(c);
};


static int myreadc(void *arg) {
  return(getchar());
};


/* I don't quite understand what's going on here! */
static int mywrite(void *arg, char const *buffer, int len) {
  return 0;
};


static char *mygets(void *arg, char *buffer, int len) {
  return buffer;
};

/**
 * dagstandalone
 *
 * Called directly from main() and WinMain() this
 * is in effect the main function of the program.
 *
 *
 */
 void dagstandalone(void) {
  int i;
#ifndef WIN32
  struct sigaction action;
#endif
  PointHandle point;
  Dbg_ConfigBlock config;
  Dbg_HostosInterface hostif;
  struct Dbg_MCState *MCState = NULL;
  
  ARMword RegVals[] = { 0 }; /* PC - reset*/
  
#ifndef WIN32
#ifndef AMIGA
  /* Setup a signal handler for SIGUSR1 */
  action.sa_handler = dagstandalone_handlesignal;
  sigemptyset (&action.sa_mask);
  action.sa_flags = 0;
  
  sigaction(SIGUSR1, &action, (struct sigaction *) 0);
#endif
#endif

  config.processor = ARM2;
  
  switch(hArcemConfig.eMemSize) {
    case MemSize_256K:
    case MemSize_512K:
    case MemSize_1M:
      fprintf(stderr, "256K, 512K and 1M memory size not yet supported, rounding up to 2M\n");
      config.memorysize = 2 * 1024 * 1024;
      break;

    case MemSize_2M:
      config.memorysize = 2 * 1024 * 1024;
      break;

    case MemSize_4M:
      config.memorysize = 4 * 1024 * 1024;
      break;

    case MemSize_8M:
      config.memorysize = 8 * 1024 * 1024;
      break;

    case MemSize_12M:
      config.memorysize = 12 * 1024 * 1024;
      break;

    case MemSize_16M:
      config.memorysize = 16 * 1024 * 1024;
      break;

    default:
      fprintf(stderr, "Unsupported memory size");
      exit(EXIT_FAILURE);
  }

  config.bytesex = RDISex_Little;

  hostif.dbgprint = myprint;
  hostif.dbgpause = mypause;
  hostif.dbgarg = stdout;
  hostif.writec = mywritec;
  hostif.readc = myreadc;
  hostif.write = mywrite;
  hostif.gets = mygets;
  hostif.reset = mypause; /* do nothing */
  hostif.resetarg = "Do I love resetting or what!\n";

  if (rdi_state)
  {
    /* we have restarted, so kill off the existing run.  */
    /* armul_rdi.close(); */
  }
  i = armul_rdi.open(0, &config, &hostif, MCState);
  rdi_state = 1;

  armul_rdi.CPUwrite(3 /* That should just about be svc 26 */, RDIReg_PC,
      RegVals);
  /*x = ~0x4;
  armul_rdi.info(RDIVector_Catch, &x, 0); */


  /* Excecute */
  i = armul_rdi.execute(&point);

  /* Close and Finalise */
  i = armul_rdi.close();
  rdi_state = 0;
  emu_state = NULL;
}
