/* Operacni systemy 2018/2019
 * Projekt 2 
 * Katerina Cibulcova (xcibul12)
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <semaphore.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/shm.h>

int create_sem(void);
void remove_sem(void);
int create_shm(void);
void remove_shm(void);
void make_hackers(int P);
void make_serfs(int P);
void hacker(int i);
void serf(int i);
void printer(int i, char *character, char *text);

sem_t *SEMmolo;
sem_t *SEMwriting;
sem_t *SEMboard_serf;
sem_t *SEMboard_hacker;
sem_t *SEMmember;
sem_t *SEMentered;
sem_t *SEMall_landed;
sem_t *SEMwelcome;

int molo_shm_counter = 0;
int action_shm_counter = 0;
int waiting_shm_hacker = 0;
int waiting_shm_serf = 0;
int ready_shm_hacker = 0;
int ready_shm_serf = 0;


int *molo_counter = 0;
int *action_counter = 0;
int *waiting_hacker = 0;
int *waiting_serf = 0;
int *ready_hacker = 0;
int *ready_serf = 0;


int P; //pocet osob v kazde kategorii (serf a hacker)
int H; //maximalni doba generovani hackera [ms]
int S; //maximalni doba generovani serfa [ms]
int R; //maximalni doba plavby [ms]
int W; //maximalni hodnota doby, po ktere se osoba vraci zpet na molo [ms]
int C; //kapacita mola

FILE* output;
//funkce na vytvoreni semaforu
int create_sem(void)
{
	remove("/xcibul12_molo");
	remove("/xcibul12_writing");
	remove("/xcibul12_board_serf");
	remove("/xcibul12_board_hacker");
	remove("/xcibul12_member");
	remove("/xcibul12_entered");
	remove("/xcibul12_welcome");
	remove("/xcibul12_all_landed");
	if (
		((SEMmolo = sem_open("/xcibul12_molo", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED) ||
		((SEMwriting = sem_open("/xcibul12_writing", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED) ||
		((SEMboard_serf = sem_open("/xcibul12_board_serf", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED) ||
		((SEMboard_hacker = sem_open("/xcibul12_board_hacker", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED) ||
		((SEMmember = sem_open("/xcibul12_member", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED) ||
		((SEMentered = sem_open("/xcibul12_entered", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED) ||
		((SEMwelcome = sem_open("/xcibul12_welcome", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED) ||
		((SEMall_landed = sem_open("/xcibul12_landed", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED)

		)
	{
		return 1;
	}
	return 0;
}

//funkce na smazani semaforu
void remove_sem(void)
{
	sem_close(SEMmolo);
	sem_close(SEMwriting);
	sem_close(SEMboard_serf);
	sem_close(SEMboard_hacker);
	sem_close(SEMmember);
	sem_close(SEMentered);
	sem_close(SEMwelcome);
	sem_close(SEMall_landed);
	sem_unlink("/xcibul12_molo");
	sem_unlink("/xcibul12_writing");
	sem_unlink("/xcibul12_board_hacker");
	sem_unlink("/xcibul12_board_serf");
	sem_unlink("/xcibul12_member");
	sem_unlink("/xcibul12_entered");
	sem_unlink("/xcibul12_all_landed");
	sem_unlink("/xcibul12_welcome");
}

//funkce pro vytvoreni sdilene pameti
int create_shm(void)
{
	if (
		((molo_shm_counter = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666)) == -1) ||
		((action_shm_counter = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666)) == -1) ||
		((waiting_shm_hacker = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666)) == -1) ||
		((waiting_shm_serf = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666)) == -1) ||
		((ready_shm_hacker = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666)) == -1) ||
		((ready_shm_serf = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666)) == -1)
		)
	{
		remove_sem(); //smazani semaforu
	remove_shm(); //smazani sdilene pameti
	fclose(output); //zavreni souboru pro zapis
		return 1;
	}
	molo_counter = shmat(molo_shm_counter, NULL, 0);
	action_counter = shmat(action_shm_counter, NULL, 0);
	waiting_hacker = shmat(waiting_shm_hacker, NULL, 0);
	waiting_serf = shmat(waiting_shm_serf, NULL, 0);
	ready_hacker = shmat(ready_shm_hacker, NULL, 0);
	ready_serf = shmat(ready_shm_serf, NULL, 0);
	return 0;
}

//funkce na smazani sdilene pameti
void remove_shm(void)
{
	shmdt(molo_counter);
	shmdt(action_counter);
	shmdt(waiting_hacker);
	shmdt(waiting_serf);
	shmdt(ready_hacker);
	shmdt(ready_serf);
	shmctl(molo_shm_counter, IPC_RMID, NULL);
	shmctl(action_shm_counter, IPC_RMID, NULL);
	shmctl(waiting_shm_hacker, IPC_RMID, NULL);
	shmctl(waiting_shm_serf, IPC_RMID, NULL);
	shmctl(ready_shm_hacker, IPC_RMID, NULL);
	shmctl(ready_shm_serf, IPC_RMID, NULL);
}

void make_hackers(int P) {

int status;

	for (int i = 1; i <= P; i++)
	{
		pid_t hacker_id;

		int tmp;
		if (H > 0)
		{
			tmp = (rand() % H) * 1000;
		}
		if (tmp > 0)
		{
			usleep(tmp);
		}
		hacker_id = fork();
		if (hacker_id == 0)
		{
			hacker(i);
		}
	}
	while (wait(&status) > 0);

	exit(0);

}


 void make_serfs(int P) {
	
	int status;
	for (int i = 1; i <= P; i++)
	{
		pid_t serf_id;

		int tmp;
		if (S > 0)
		{
			tmp = (rand() % S) * 1000;
		}
		if (tmp > 0)
		{
			usleep(tmp);
		}
		serf_id = fork();
		if (serf_id == 0)
		{
			serf(i);
		}
	}
	while (wait(&status) > 0);

	exit(0);
} 


void hacker(int i) {

	printer(i, "HACK", "starts");
	
	while (1) {
		sem_wait(SEMmolo);
	if ((*molo_counter) < C) {
		(*molo_counter)++;
		(*waiting_hacker)++;
		(*ready_hacker)++;

		printer(i, "HACK", "waits");
		sem_post(SEMmolo);
		break;
	}
	else {
		sem_post(SEMmolo);

		printer(i, "HACK", "leaves queue");

		int waiting = rand() % W * 1000;
			usleep(waiting);

		printer(i, "HACK", "is back");
		
	}
}
//printf("hello\n");


sem_wait(SEMentered);
sem_wait(SEMmolo);

if (*ready_serf >= 2 && *ready_hacker == 2)
	{
		(*ready_hacker) = 0;
		(*ready_serf) = (*ready_serf) - 2;

		sem_post(SEMmolo);
		sem_post(SEMentered);
		sem_wait(SEMwelcome);
		sem_wait(SEMmolo);	

		(*waiting_serf)--;
		(*waiting_serf)--;
		(*waiting_hacker)--;
		(*waiting_hacker)--;
		


		(*molo_counter)--;
		(*molo_counter)--;
		(*molo_counter)--;
		(*molo_counter)--;

		sem_post(SEMmolo);	

		sem_post(SEMboard_hacker);
		sem_post(SEMboard_serf);
		sem_post(SEMboard_serf);

		
		printer(i, "HACK", "boards");



		int voyage = rand() % R * 1000;
		usleep(voyage);


		sem_post(SEMmember);
		sem_post(SEMmember);
		sem_post(SEMmember);

		

		sem_wait(SEMall_landed);
		sem_wait(SEMall_landed);	
		sem_wait(SEMall_landed);			

		printer(i, "HACK", "captain exits");
		sem_post(SEMwelcome);
		
	} else if (*ready_hacker == 4)
	{
		*ready_hacker = 0;

		sem_post(SEMmolo);
		sem_post(SEMentered);
		sem_wait(SEMwelcome);
		sem_wait(SEMmolo);	

		(*waiting_hacker)--;
		(*waiting_hacker)--;
		(*waiting_hacker)--;
		(*waiting_hacker)--;
	


		(*molo_counter)--;
		(*molo_counter)--;
		(*molo_counter)--;
		(*molo_counter)--;

		sem_post(SEMmolo);	

		sem_post(SEMboard_hacker);
		sem_post(SEMboard_hacker);
		sem_post(SEMboard_hacker);
		

		printer(i, "HACK", "boards");


		int voyage = rand() % R * 1000;
		usleep(voyage);


		sem_post(SEMmember);
		sem_post(SEMmember);
		sem_post(SEMmember);

	
		sem_wait(SEMall_landed);
		sem_wait(SEMall_landed);	
		sem_wait(SEMall_landed);			

		printer(i, "HACK", "captain exits");

		sem_post(SEMwelcome);
	}
	 else {
	 	sem_post(SEMmolo);
	 	sem_post(SEMentered);
	 	
	 	sem_wait(SEMboard_hacker);
	 	sem_wait(SEMmember);
		printer(i, "HACK", "member exits");
		sem_post(SEMall_landed);

	}

exit(0);
}	



void serf(int i) {

	printer(i, "SERF", "starts");

	while (1) {
		sem_wait(SEMmolo);

	if ((*molo_counter) < C) {
		(*molo_counter)++;
		(*waiting_serf)++;
		(*ready_serf)++;

		printer(i, "SERF", "waits");
		sem_post(SEMmolo);
		break;
		

	}
	else {

		sem_post(SEMmolo);
		printer(i, "SERF", "leaves queue");

		int waiting = rand() % W * 1000;
			usleep(waiting);

		printer(i, "SERF", "is back");
	
		
	}
}
//

sem_wait(SEMentered);
sem_wait(SEMmolo);
if (*ready_serf == 2 && *ready_hacker >= 2)
	{
		*ready_serf = 0;
		*ready_hacker = *ready_hacker - 2;


		sem_post(SEMmolo);	
		sem_post(SEMentered);
		sem_wait(SEMwelcome);
		sem_wait(SEMmolo);

		(*waiting_serf)--;
		(*waiting_serf)--;
		(*waiting_hacker)--;
		(*waiting_hacker)--;
		
		(*molo_counter)--;
		(*molo_counter)--;
		(*molo_counter)--;
		(*molo_counter)--;

		sem_post(SEMmolo);		
		

		
		sem_post(SEMboard_serf);
		sem_post(SEMboard_hacker);
		sem_post(SEMboard_hacker);
		
		printer(i, "SERF", "boards");

	

		int voyage = rand() % R * 1000;
		usleep(voyage);


		sem_post(SEMmember);
		sem_post(SEMmember);
		sem_post(SEMmember);

	
		sem_wait(SEMall_landed);
		sem_wait(SEMall_landed);	
		sem_wait(SEMall_landed);			

		printer(i, "SERF", "captain exits");

		sem_post(SEMwelcome);
	} 
	else if (*ready_serf == 4)
	{
		*ready_serf = 0;

		sem_post(SEMmolo);	
		sem_post(SEMentered);
		sem_wait(SEMwelcome);
		sem_wait(SEMmolo);	

		(*waiting_serf)--;
		(*waiting_serf)--;
		(*waiting_serf)--;
		(*waiting_serf)--;

			
		(*molo_counter)--;
		(*molo_counter)--;
		(*molo_counter)--;
		(*molo_counter)--;
	
		sem_post(SEMmolo);	
	
		
		sem_post(SEMboard_serf);
		sem_post(SEMboard_serf);
		sem_post(SEMboard_serf);

		printer(i, "SERF", "boards");

		int voyage = rand() % R * 1000;
		usleep(voyage);
	

		sem_post(SEMmember);
		sem_post(SEMmember);
		sem_post(SEMmember);

	
		sem_wait(SEMall_landed);
		sem_wait(SEMall_landed);	
		sem_wait(SEMall_landed);			

		printer(i, "SERF", "captain exits");

		sem_post(SEMwelcome);
	} 
	else {
		sem_post(SEMmolo);	
		sem_post(SEMentered);
	 	sem_wait(SEMboard_serf);
	 	sem_wait(SEMmember);
		printer(i, "SERF", "member exits");
		sem_post(SEMall_landed);

	}
exit(0);
}



void printer(int i, char *character, char *text)
{
	int shorter = 0;
	if (strcmp(text, "starts") == 0) { shorter = 1; }
	if (strcmp(text, "is back") == 0) { shorter = 1; }
	sem_wait(SEMwriting);
	(*action_counter)++;
	if (shorter == 0)
	{
		fprintf(output, "%d: %s %d: %s: %d: %d\n", *action_counter, character, i, text, *waiting_hacker, *waiting_serf);
		//printf("%d: %s: %d: %s: %d: %d\n", *action_counter, character, i, text, *waiting_hacker, *waiting_serf);
	}
	else
	{
		fprintf(output, "%d: %s %d: %s\n", *action_counter, character, i, text);
		//printf("%d: %s: %d: %s\n", *action_counter, character, i, text);
	}
	fflush(output);
	sem_post(SEMwriting);
}

int main(int argc, char **argv) {

	if (argc != 7) {
		fprintf(stderr, "Byl zadan chybny pocet argumentu.\n");
		return 1;
	}
	else {
		//zpracovani argumentu
		//////////////////////P
		char *endptr;
		if (((strtol(argv[1],&endptr,10)) >= 2) && ((strtol(argv[1],&endptr,10)) % 2 == 0)) {

			P = strtol(argv[1],&endptr,10);

		} 
		else {
			fprintf(stderr, "Byl zadan chybny argument P.\n");
			return 1;
		}
		//////////////////////H
		if (((strtol(argv[2],&endptr,10)) >= 0) && ((strtol(argv[2],&endptr,10)) <= 2000)) {

			H = strtol(argv[2],&endptr,10);

		} 
		else {
			fprintf(stderr, "Byl zadan chybny argument H.\n");
			return 1;
		}
		//////////////////////S
		if (((strtol(argv[3],&endptr,10)) >= 0) && ((strtol(argv[3],&endptr,10)) <= 2000)) {

			S = strtol(argv[3],&endptr,10);

		} 
		else {
			fprintf(stderr, "Byl zadan chybny argument S.\n");
			return 1;
		}

		//////////////////////R
		if (((strtol(argv[4],&endptr,10)) >= 0) && ((strtol(argv[4],&endptr,10)) <= 2000)) {

			R = strtol(argv[4],&endptr,10);

		} 
		else {
			fprintf(stderr, "Byl zadan chybny argument R.\n");
			return 1;
		}

		//////////////////////W
		if (((strtol(argv[5],&endptr,10)) >= 20) && ((strtol(argv[5],&endptr,10)) <= 2000)) {

			W = strtol(argv[5],&endptr,10);

		} 
		else {
			fprintf(stderr, "Byl zadan chybny argument W.\n");
			return 1;
		}

		//////////////////////C
		if ((strtol(argv[6],&endptr,10)) >= 5) {

			C = strtol(argv[6],&endptr,10);

		} 
		else {
			fprintf(stderr, "Byl zadan chybny argument C.\n");
			return 1;
		}

	}


	//otevreni vystupniho souboru
	if ((output = fopen("./proj2.out", "w")) == NULL) 
	{
		fprintf(stderr, "Chyba pri otvirani souboru \"proj2.out\" pro zapis.\n");
		return 1;
	}

	//pokus otevrit semafory
	if (create_sem() == 1)
	{
		fprintf(stderr, "Chyba pri vytvareni semaforu.\n");
		remove_sem(); //smazani semaforu
		fclose(output); //zavreni souboru pro zapis
		exit(1);
	}

	//pokus alokovat sdilenou pamet
	if (create_shm() == 1)
	{
		fprintf(stderr, "Chyba pri vytvareni sdilene pameti.\n");
		remove_sem(); //smazani semaforu
		remove_shm(); //smazani sdilene pameti
		fclose(output); //zavreni souboru pro zapis
		exit(1);
	}



	//PROCESY


	(*action_counter) = 0;
	(*waiting_hacker) = 0;
	(*waiting_serf) = 0;


	pid_t ID;


	int status;

	//generovani hackeru
	ID = fork();
	if (ID == 0) {

		make_hackers(P);
	}

	//generovani serfu
	ID = fork();
	if (ID == 0) {

		make_serfs(P);
	}

	while (wait(&status) > 0);
	remove_sem(); //smazani semaforu
	remove_shm(); //smazani sdilene pameti
	fclose(output); //zavreni souboru pro zapis

	return 0;

}