/* $Id: computer_status.c,v 1.3 2001/05/09 10:53:08 jorge Exp $ */

#include <stdio.h>
#include <signal.h>
#include <stdint.h>

#include "computer_status.h"
#include "task.h"
#include "logger.h"

void get_computer_status (struct computer_status *cstatus)
{
  /* Get status not only gets the load average but also */
  /* checks that every task is running and in case they're not */
  /* it sets the task record to unused (used = 0) */
  get_loadavg (cstatus->loadavg);
  check_tasks (cstatus);
}

void init_computer_status (struct computer_status *cstatus)
{
  init_tasks (cstatus->task);
}

void check_tasks (struct computer_status *cstatus)
{
  int i;

  cstatus->numtasks = 0;
  for (i=0;i<MAXTASKS;i++) {
    if (cstatus->task[i].used) {
      if (kill(cstatus->task[i].pid,0) == 0) { /* check if task is running */
	cstatus->numtasks++;
      } else {
	/* task is registered but not running */
	cstatus->task[i].used = 0;
      }
    }
  }
}

#ifdef __LINUX			/* __LINUX  */

void get_loadavg (uint16_t *loadavg)
{
  FILE *f_loadavg;
  float a,b,c;
  
  if ((f_loadavg = fopen("/proc/loadavg","r")) == NULL) {
    perror ("get_loadavg: fopen");
    exit (1);
  }

  fscanf (f_loadavg,"%f %f %f",&a,&b,&c);
  
  loadavg[0] = a * 100;
  loadavg[1] = b * 100;
  loadavg[2] = c * 100;

  fclose (f_loadavg);
}

#else
#error You need to define the OS, or OS defined not supported
#endif 

void report_computer_status (struct computer_status *status)
{
  int i;

  printf ("Load Average: %i %i %i\n",status->loadavg[0],status->loadavg[1],status->loadavg[2]);
  printf ("Number of tasks running: %i\n",status->numtasks);
  for (i=0;i<MAXTASKS;i++) {
    if (status->task[i].used) {
      printf ("Task record: %i\n",i);
      printf ("Job name: %s\n",status->task[i].jobname);
      printf ("Job index: %i\n",status->task[i].jobindex);
      printf ("Job command: %s\n",status->task[i].jobcmd);
      printf ("Current frame: %i\n",status->task[i].frame);
      printf ("Task pid: %i\n",status->task[i].pid);
      printf ("Task status: %i\n",status->task[i].status);
    }
  }
}






