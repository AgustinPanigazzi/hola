#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h> 
#include <string.h> 
#include <ctype.h>
#include <sys/shm.h> 	
#include <fcntl.h>	
#include <semaphore.h> 
#include <signal.h>
#define SEGMENTO_ID		234
#define SEGMENTO_ID2	532 
#define REGISTROS 	1

typedef struct 
{
    char nombre[15];
    char raza[15];
    char sexo;
    char castracion[3];
}Gato;

typedef struct{
	int decision;
	Gato gato;
}Decision;

int main(){	

	signal(SIGINT, SIG_IGN);
	Decision des;
	sem_t *leer		 =	sem_open("/leer",		O_CREAT);
	sem_t *escribir =	sem_open("/escribir",O_CREAT);

	sem_t *leer2			=	sem_open( "/leer",		O_CREAT);
	sem_t *escribir2	=	sem_open( "/escribir",	O_CREAT); 
	
	int shmid = shmget(SEGMENTO_ID, sizeof(Decision), IPC_CREAT | 0666);
	int shmid2 = shmget(	SEGMENTO_ID2, sizeof(int), IPC_CREAT | 0666);		
	
	Decision *area_compartida = (Decision*)shmat( shmid, NULL, 0);
	int *area_compartidaRes = (int*)shmat(shmid2,NULL,0);
	int opcion;
	

		puts("MENU");
		do{
			printf("1.ALTA(Nombre,Raza,Sexo(M-H),Castracion(CA-SC) \n");
			printf("2.BAJA(Nombre) \n");
			printf("3.CONSULTA(Nombre) \n");
			printf("4.Consulta() \n");
			scanf("%d",&opcion);
		}while(opcion>4 || opcion<1);

		switch (opcion)
		{
		case 1:
			des.decision = 1;
			printf("ingrese nombre: ");
			scanf("%s",des.gato.nombre);

			printf("ingrese raza: ");
			scanf("%s",des.gato.raza);
			do
			{
				printf("ingrese sexo: ");
				scanf(" %c", &des.gato.sexo);
				fflush(stdin);
				des.gato.sexo= toupper(des.gato.sexo);
			} while (des.gato.sexo!='M' && des.gato.sexo!='H');

			do
			{
				printf("ingrese estado de castracion: ");
				scanf("%s",des.gato.castracion);

			} while (strcmp(des.gato.castracion,"SC")!=0 && strcmp(des.gato.castracion,"CA")!=0);

			sem_wait( escribir ); // P(escribir)
			*area_compartida = des;
			sem_post( leer ); // V(leer)
			
			sem_wait(leer2);
			if(*area_compartidaRes == 1)
				printf("%s ha sido ingresado correctamente",des.gato.nombre);
			else if(*area_compartidaRes == 2)
				printf("El gato: %s, ya se encuentra cargado en el sistema", des.gato.nombre);
				else
					printf("sin memoria");
			sem_post(escribir2);

			break;
		case 2:
			des.decision = 2;
			printf("ingrese nombre: ");
			scanf("%s",des.gato.nombre);

			break;
		case 3:
			des.decision = 3;
			printf("ingrese nombre: ");
			scanf("%s",des.gato.nombre);

			break;
		case 4:
			des.decision = 4;
			break;
		}

	
	shmdt( &area_compartida );	
	shmdt( &area_compartidaRes);

	sem_close( leer );
	sem_close( escribir ); 
	sem_close(leer2);
	sem_close(escribir2);	
	 	
	return 0;
}





