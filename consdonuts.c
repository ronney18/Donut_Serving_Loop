/* Name: Ronney Sanchez
 * Date: 3/12/19
 * Course: COMP3080 Operating System
 * Assignment 3
 * Email: Ronney_Sanchez@student.uml.edu
 */
/** consdonuts.c for donuts process implementation **/

#include <stdio.h>
#include <string.h>
#include "donuts.h"

int shmid, semid[3];

int get_cpu_id()
{
	int i;
	char* line;
	FILE* procfile = fopen("/proc/self/stat", "r");
	long to_read = 8192;
	char buffer[to_read];
	int read = fread(buffer, sizeof(char), to_read, procfile);
	fclose(procfile);

	line = strtok(buffer, " ");
	for(i = 1; i < 38; i++)
	{
		line = strtok(NULL, " ");
	}

	line = strtok(NULL, " ");
	int cpu_id = atoi(line);
	return cpu_id;
}

int main(int argc, char *argv[])
{
	int i, j, k, donut;
	struct donut_ring *shared_ring;
	struct timeval randtime;
	unsigned short xsubl[3];
	char *dtype[] = {"jelly", "coconut", "plain", "glazed"};
	key_t memkey, semkey;
	
	memkey = semkey = MEMKEY + getuid();
	
	if((shmid = shmget(memkey, sizeof(struct donut_ring), 0)) == -1)
	{
		perror("SHARED GET FAILED: ");
		exit(1);
	}

	if((shared_ring = (struct donut_ring *)shmat(shmid, NULL, 0)) == (void *) -1)
	{
		perror("SHARED ATTACH FAILED: " );
		exit(1);
	}

	for(i = 0; i < NUMSEMIDS; i++)
	{
		if((semid[i] = semget(semkey + i, NUMFLAVORS, 0)) == -1)
		{
			perror("SEMAPHORE ALLOCATION FAILED: ");
			exit(1);
		}
	}

	gettimeofday(&randtime, NULL);

	xsubl[0] = (ushort)randtime.tv_usec;
	xsubl[1] = (ushort)(randtime.tv_usec >> 16);
	xsubl[2] = (ushort)(getpid());

	for(i = 0; i < 10; i++)
	{
		fprintf(stderr, "CONSUMER %s\t Number of Dozens: %d\n", argv[1], i);
		for(k = 0; k < 12; k++)
		{
			j = nrand48(xsubl) & 3;
			if(p(semid[CONSUMER], j) == -1)
			{
				perror("P OPERATION FAILED: ");
				exit(3);
			}
			if(p(semid[OUTPTR], j) == -1)
                        {
                                perror("P OPERATION FAILED: ");
                                exit(3);
                        }
			donut = shared_ring->flavor[j][shared_ring->outptr[j]];
			shared_ring->outptr[j] = (shared_ring->outptr[j] + 1) % NUMSLOTS;
			if(v(semid[PROD], j) == -1)
			{
				perror("V OPERATION FAILED: ");
				exit(3);
			}
			if(v(semid[OUTPTR], j) == -1)
                        {
                                perror("V OPERATION FAILED: ");
                                exit(3);
                        }
			printf("Donut Type: %s \tSerial Number: %d\n", dtype[j], donut);
		}
		usleep(10000);
	}
	fprintf(stderr, " CONSUMER %s DONE\n", argv[1]);
	return 0;
}
