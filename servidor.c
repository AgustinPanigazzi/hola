#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <sys/ipc.h> 	//Biblioteca para los flags IPC_ 
#include <sys/shm.h> 	//Para memoria compartida SYSTEM-V	
#include <semaphore.h>	//Para semáforos POSIX
#include <fcntl.h>		//Para utilizar los flags O_
#include <signal.h>

#define SIN_MEM             0
#define TODO_OK             1
#define DUPLICADO           2
#define SEGMENTO_ID	234 //La clave también puede ser creada con ftok()
#define SEGMENTO_ID2	532
#define REGISTROS 	1

#define min(a,b) ((a)<(b)? (a) : (b))

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


typedef struct s_nodo 
{
	void *dato;
	unsigned TamElem;
	struct s_nodo *sig;
}Nodo;


typedef Nodo* Lista;


typedef int (*Cmp)(const void* e1, const void* e2);
int cmp_nombre(const void* v1, const void* v2);

void crearLista(Lista* pl);
int insertarEnListaOrd(Lista *pl, const void* dato, size_t tamElem, Cmp cmp);
int buscarEnListaDesord(const Lista* pl, void* dato, size_t tamElem, Cmp cmp);
int eliminarDeListaOrdPorValor(Lista* pl, void* dato, size_t tamElem, Cmp cmp);
int eliminarDeListaUltimo(Lista* pl, void* dato, size_t tamElem);
int listaVacia(const Lista* pl);
void vaciarLista(Lista* pl);


int main(){	
	//Creación de los semáforos
	sem_t *leer			=	sem_open( "/leer",		O_CREAT | O_EXCL,	0666,	0);
	sem_t *escribir	=	sem_open( "/escribir",	O_CREAT | O_EXCL,	0666,	1); 

	sem_t *leer2			=	sem_open( "/leer",		O_CREAT | O_EXCL,	0666,	0);
	sem_t *escribir2	=	sem_open( "/escribir",	O_CREAT | O_EXCL,	0666,	1); 
	
	signal(SIGINT, SIG_IGN);

	Decision des;
	//Creación de memoria compartida
	int shmid = shmget(	SEGMENTO_ID, sizeof(Decision), IPC_CREAT | 0666);

	int shmid2 = shmget(	SEGMENTO_ID2, sizeof(int), IPC_CREAT | 0666);	

	//Vincular la memoria compartida a una variable local
	Decision *area_compartida = (Decision*)shmat(shmid,NULL,0);
	int *area_compartidaRes = (int*)shmat(shmid2,NULL,0);
	int resultado;

	Lista lista;
	crearLista(&lista);


		sem_wait(leer);	//P( leer )
		des = *area_compartida;

		switch (des.decision)
		{
		case 1:
			resultado = insertarEnListaOrd(&lista,&des.gato,sizeof(Gato),cmp_nombre);

			sem_wait(escribir2);
			*area_compartidaRes = resultado;
			sem_post(leer2);
            sem_post(escribir);		//V( escribir ) 
			break;
		
		default:
			break;
		}


	//Desvincular la memoria compartida de la variable local
	shmdt( &area_compartida );
    shmdt( &area_compartidaRes );	
	
	//Marcar la memoria compartida para borrar
	shmctl( shmid, IPC_RMID, NULL );
    shmctl( shmid2, IPC_RMID, NULL );	
	
	//Cierre de los semáforos
	sem_close(leer); 
	sem_close(escribir); 	
	sem_close(leer2); 
	sem_close(escribir2); 

	//Marcar los semáforos para destruirlos
	sem_unlink("/leer");
	sem_unlink("/escribir");
	sem_unlink("/leer2");
	sem_unlink("/escribir2");
	
	return 0;
}










///LISTA


void crearLista(Lista* pl)
{
    *pl=NULL;
}

int insertarEnListaOrd(Lista *pl, const void* dato, size_t tamElem, Cmp cmp)
{
    while(*pl && cmp((*pl)->dato,dato)>0)
        pl= &(*pl)->sig;

    if(*pl && cmp((*pl)->dato,dato)==0)
        return DUPLICADO;

    Nodo* nue = (Nodo*)malloc(sizeof(Nodo));
    void* datoN= malloc(tamElem);

    if(!nue || !datoN)
    {
        free(nue);
        free(datoN);
        return SIN_MEM;
    }

    nue->dato=datoN;
    nue->TamElem=tamElem;
    memcpy(datoN,dato,tamElem);

    nue->sig= *pl;
    *pl=nue;

    return TODO_OK;

}

int buscarEnListaDesord(const Lista* pl, void* dato, size_t tamElem, Cmp cmp)
{
    while(*pl && cmp(dato, (*pl)->dato)!=0)
        pl = &(*pl)->sig;

    if(!*pl)
        return 0;

    Nodo* elem = *pl;

    memcpy(dato,elem->dato,min(elem->TamElem,tamElem));
    return TODO_OK;
}

int eliminarDeListaOrdPorValor(Lista* pl, void* dato, size_t tamElem, Cmp cmp)
{
    while(*pl && cmp((*pl)->dato,dato)!=0)
        pl= &(*pl)->sig;

    if(!*pl || cmp((*pl)->dato,dato)!=0)
        return 0;

    Nodo* nae=*pl;
    *pl = nae->sig;

    memcpy(dato,nae->dato,min(nae->TamElem,tamElem));

    free(nae->dato);
    free(nae);

    return TODO_OK;
}

int eliminarDeListaUltimo(Lista* pl, void* dato, size_t tamElem)
{
    if(!*pl)
        return 0;

    while((*pl)->sig)
        pl= &(*pl)->sig;

    Nodo* nae = *pl;
    *pl = nae->sig;

    memcpy(dato, nae->dato,min(tamElem,nae->TamElem));
    free(nae->dato);
    free(nae);

    return 1;
}

void vaciarLista(Lista* pl)
{
    Nodo* nae;

    while(*pl)
    {
        nae = *pl;
        *pl = nae->sig;
        free(nae->dato);
        free(nae);
    }
}

int cmp_nombre(const void* v1, const void* v2){
	Gato *p1 = (Gato*) v1;
	Gato *p2 = (Gato*) v2;

	return strcmp(p1->nombre,p2->nombre);
}