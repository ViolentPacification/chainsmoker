#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

// agent releasing item on table.
sem_t agentSEM;

//Smoker semaphore.
sem_t smokerSEM[3];

// Ingredients required by each smoker.
char* ingredient[3] = { "teeli & tambako", "teeli & kaghaz", "tambako & kaghaz" };

// This list represents item types that are on the table. This should corrispond
// with the ingredient, such that each item is the one the smoker has. So the
// first item would be paper, then tobacco, then matches.
bool tableITEMS[3] = { false, false, false };

// Each pusher pushes a certian type item, manage these with this semaphore
sem_t pusherSEM[3];

/**
 * Smoker function, wait for the item Smoker need , and then
 * smoking. Repeat this three times
 */


void* smoker(void* arg)
{
	int smokerID = *(int*) arg;
	int typeID   = smokerID % 3;

	// Smoke 3 times
	for (int i = 0; i < 3; ++i)
	{
		printf("Smoker %d Waiting for %s\n", smokerID+1, ingredient[typeID]);

		// Wait for required combination on the table.
		sem_wait(&smokerSEM[typeID]);

		// Make the ciggerate,then release the agent.
		printf("Smoker %d Now making the cigarette\n", smokerID+1);
		usleep(rand() % 50000);
		

		// Now smoking.
		printf("Smoker %d Now smoking\n", smokerID+1);
		usleep(10000);
		sem_post(&agentSEM);

	}

	return NULL;
}

// pusher gets access to the items on the table.
sem_t pusherLOCK;

/**
 * The pusher's job is to release proper smoker semaphore according to the items present.
 */

void* pusher(void* arg)
{
	int pusherID = *(int*) arg;

	for (int i = 0; i < 6; ++i)
	{
		// Wait for this pusher to be needed
		sem_wait(&pusherSEM[pusherID]);
		sem_wait(&pusherLOCK);

		// Check for next required item
		if (tableITEMS[(pusherID + 1) % 3])
		{
			tableITEMS[(pusherID + 1) % 3] = false;
			sem_post(&smokerSEM[(pusherID + 2) % 3]);
		}
		else if (tableITEMS[(pusherID + 2) % 3])
		{
			tableITEMS[(pusherID + 2) % 3] = false;
			sem_post(&smokerSEM[(pusherID + 1) % 3]);
		}
		else
		{
			// Other item not present
			tableITEMS[pusherID] = true;
		}

		sem_post(&pusherLOCK);
	}

	return NULL;
}

/**
 * agent puts items on table
 */
void* agent(void* arg)
{
	int agentID = *(int*) arg;

	for (int i = 0; i < 9; ++i)
	{
		usleep(rand() % 200000);

		// Wait for lock on agent
		sem_wait(&agentSEM);

		// Release the item agent is holding
		sem_post(&pusherSEM[i]);
		sem_post(&pusherSEM[(i + 1) % 3]);

		// What did agent offered
		printf("\n\nAgent is giving out %s\n", ingredient[(i + 2) % 3]);
	}

	return NULL;
}

/**
 * main thread handles the agent's arbitration of items.
 */
int main(int argc, char* arvg[])
{
	// Seeding the random number
	srand(time(NULL));

	// There is only one agent semaphore. Value==1 if nothing on table.
	sem_init(&agentSEM, 0, 1);

	// Intializing pusher lock semaphore.
	sem_init(&pusherLOCK, 0, 1);

	// Initialize for smoker and pusher.
	for (int i = 0; i < 3; ++i)
	{
		sem_init(&smokerSEM[i], 0, 0);
		sem_init(&pusherSEM[i], 0, 0);
	}



	// Smoker Id is passed to the threads.
	int smokerIDS[3];

	pthread_t smokerTHREAD[3];

	// Create the 3 smoker threads with Id's
	for (int i = 0; i <= 2; ++i)
	{
		smokerIDS[i] = i;

		if (pthread_create(&smokerTHREAD[i], NULL, smoker, &smokerIDS[i]) == EAGAIN)
		{
			perror("Error making the thread!");
			return 0;
		}
	}

	// Pusher Id's is passed to the threads.
	int pusherIDs[3];

	pthread_t pusherTHREAD[3];

	for (int i = 0; i < 3; ++i)
	{
		pusherIDs[i] = i;

		if (pthread_create(&pusherTHREAD[i], NULL, pusher, &pusherIDs[i]) == EAGAIN)
		{
			perror("Error making the thread!");
			return 0;
		}
	}

	// Agent Id's is passed to the threads.
	int agentIDS=1;

	pthread_t agentTHREAD;

		if (pthread_create(&agentTHREAD, NULL, agent, &agentIDS) == EAGAIN)
		{
			perror("Error making the thread");
			return 0;
		}
	

	// Make sure sab ki talb poori ho
	for (int i = 0; i < 3; ++i)
	{
		pthread_join(smokerTHREAD[i], NULL);
	}

	return 0;
}
