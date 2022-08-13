/* Name: Ronney Sanchez
 * Date: 3/12/19
 * Course: COMP3080 Operating System
 * Assignment 3
 * Email: Ronney_Sanchez@student.uml.edu
 */
/** producers.c for donuts process implementation **/
#include <stdio.h>
#include "donuts.h"

int	shmid, semid[3];
void sig_handler (int);

int main(int argc, char *argv[])
{
        int     in_ptr [NUMFLAVORS];
        int     serial [NUMFLAVORS];
        int     i, j, k, nsigs;
        struct donut_ring *shared_ring;
        struct timeval randtime;
	unsigned short xsubl[3];
	struct sigaction new;
	sigset_t mask_sigs;
	int sigs[] = {SIGHUP, SIGINT, SIGQUIT, SIGPIPE, SIGTERM, SIGBUS, SIGSEGV, SIGFPE};
	key_t memkey, semkey;
	
	memkey = semkey = MEMKEY + getuid();
	nsigs = sizeof(sigs)/sizeof(int);
	sigemptyset(&mask_sigs);

	/* producer initializes serial counters and in-pointers  */
        for(i = 0; i < NUMFLAVORS; i++){
                in_ptr[i] = 0;
                serial[i] = 0;
        }

	for(i = 0; i < nsigs; i++)
	{
		sigaddset(&mask_sigs, sigs[i]);
	}

	for(i = 0; i < nsigs; i++)
	{
		new.sa_handler = sig_handler;
		new.sa_mask = mask_sigs;
		new.sa_flags = 0;
		if(sigaction(sigs[i], &new, NULL) == -1)
		{
			perror("CAN'T SET SIGNALS: ");
			exit(1);
		}
	}

	if((shmid = shmget(memkey, sizeof(struct donut_ring), IPC_CREAT | 0600)) == -1)
	{
		perror("SHARED GET FAILED: ");
		exit(1);
	}

	if((shared_ring = (struct donut_ring *)shmat(shmid, NULL, 0)) == (void *) - 1)
	{
		perror("SHARED ATTACH FAILED: ");
		sig_handler(-1);
	}

	for(i = 0; i < NUMSEMIDS; i++)
	{
		if((semid[i] = semget(semkey + i, NUMFLAVORS, IPC_CREAT | 0600)) == -1)
		{
			perror("SEMAPHORE ALLOCATION FAILED: ");
			sig_handler(-1);
		}
	}

	gettimeofday(&randtime, NULL);
	
	xsubl[0] = (ushort)randtime.tv_usec;
	xsubl[1] = (ushort)(randtime.tv_usec >> 16);
	xsubl[2] = (ushort)(getpid());

	if(semsetall(semid[PROD], NUMFLAVORS, NUMSLOTS) == -1)
	{
		perror("SEMSETALL FAILED: ");
		sig_handler(-1);
	}

	if(semsetall(semid[CONSUMER], NUMFLAVORS, 0) == -1)
        {
                perror("SEMSETALL FAILED: ");
                sig_handler(-1);
        }

	if(semsetall(semid[OUTPTR], NUMFLAVORS, 1) == -1)
        {
                perror("SEMSETALL FAILED: ");
                sig_handler(-1);
        }

	while(1)
	{
		j = nrand48(xsubl) & 3;
		
		if(p(semid[PROD], j) == -1)
		{
			perror("P OPERATION FAILED: ");
			sig_handler(-1);
		}
		shared_ring->flavor[j][in_ptr[j]] = serial[j];
		in_ptr[j] = (in_ptr[j] + 1) % NUMSLOTS;
		serial[j]++;

		printf("Prod Type: %d\t Serial Number: %d\n", j, serial[j] - 1);
		if(v(semid[CONSUMER], j) == -1)
		{
			perror("V OPERATION FAILED: ");
			sig_handler(-1);
		}
	}
	return 0;
}

void sig_handler(int sig)
{
        int     i,j,k;

        printf("In signal handler with signal # %d\n",sig);

        if(shmctl(shmid, IPC_RMID, 0) == -1){
                perror("handler failed shm RMID: ");
        }
        for(i = 0; i < NUMSEMIDS; i++){
          if(semctl (semid[i], 0,
                            IPC_RMID) == -1){
                perror("handler failed sem RMID: ");
          }
        }
        exit(5);
}
