#include <sys/types.h>
#include <time.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>


/*funciones de libreria*/
#include "timing.h"

/*variables constantes*/
#define MICRO 1000000
#define NANOFACT 1000

/*variables globales*/
struct timespec counter;
clockid_t precision;
int state;/*0 nomarcha 1 marcha -1 totalmente parado*/

clockid_t parse(char *buf){
	if(strcmp(buf,"CLOCK_REALTIME")==0)return CLOCK_REALTIME;
	if(strcmp(buf,"CLOCK_REALTIME_COARSE")==0)return CLOCK_REALTIME_COARSE;
	if(strcmp(buf,"CLOCK_MONOTONIC")==0)return CLOCK_MONOTONIC;
	if(strcmp(buf,"CLOCK_MONOTONIC_COARSE")==0)return CLOCK_MONOTONIC_COARSE;
	if(strcmp(buf,"CLOCK_MONOTONIC_RAW")==0)return CLOCK_MONOTONIC_RAW;
	if(strcmp(buf,"CLOCK_PROCESS_CPUTIME_ID")==0)return CLOCK_BOOTTIME;
	if(strcmp(buf,"CLOCK_REALTIME")==0)return CLOCK_PROCESS_CPUTIME_ID;
	if(strcmp(buf,"CLOCK_THREAD_CPUTIME_ID")==0)return CLOCK_THREAD_CPUTIME_ID;
	return -1;
}

/*• void start() tomará el instante actual como referencia para el contador.*/
void start(){
	int fd;
	
	if((fd=open("conf.txt", O_RDONLY))== -1){
		perror("Fallo al intentar abrir el archivo conf.txt");
		exit(0);
	}
	else{
		int nBytes;
		char buf[30];
		if((nBytes =read(fd,buf,30))== -1){
			perror("Fallo de lectura");
			exit(0);
		}
		else{
			if(buf[nBytes-1]== '\n'){
				buf[nBytes-1] = '\0';
			}
			if((precision=parse(buf))==-1) {
				perror("fallo en parse");
				exit(0);			
			}
		}
		if(clock_gettime(precision,&counter)==-1){
	
			perror("fallo al iniciar el contador");
			exit(EXIT_FAILURE);	
		}
		state=1;
	}
}
/*Calcula el tiempo en microsegundos que sera devuelto en cada una de las operaciones implementadas int pause(), int stop.*/
int calculatiempo(struct timespec ini, struct timespec fin){
	int ret;
	time_t sec=( fin.tv_sec - ini.tv_sec )*MICRO;
	int nanosec=( fin.tv_nsec - ini.tv_nsec )/NANOFACT;
	ret=sec+nanosec;
	return ret;
}
/*• int pause() parará temporalmente la cuenta asociada al contador.
Devuelve el tiempo transcurrido desde el último resume().
Si el contador ya estaba en pausa, esta llamada se ignora.*/
int pause(){
	if(state==-1){
		perror("El timer no ha sido iniciado");
		exit(EXIT_FAILURE);

	} 
	if(state==1){
		struct timespec pause;
		if(clock_gettime(precision,&pause)==-1){
				perror("fallo al pausar el contador");
				exit(EXIT_FAILURE);	
		}
		int time= calculatiempo(counter, pause);
		state=0;
		return time;
	}
}
/*• void resume() continúa la cuenta tras una pause().
Si el contador no estaba en pausa, esta llamada se ignora.*/
void resume(){
	if(state==-1){
		perror("El timer no ha sido iniciado");
		exit(EXIT_FAILURE);

	} 
	if(state==0){
		if(clock_gettime(precision,&counter)==-1){
			perror("fallo al reiniciar el contador");
			exit(EXIT_FAILURE);	
		}
		state=1;
	}

}
/*• int stop() para definitivamente el contador Devuelve el tiempo transcurrido desde la última llamada a resume() ( o start() si nunca se llamó a pause() ).*/
int stop(){
	struct timespec stop;
	if(state==1){
		if(clock_gettime(precision,&stop)==-1){
			perror("fallo al parar el contador");
			exit(EXIT_FAILURE);
		}
		int time= calculatiempo(counter, stop);
		state=-1;
		return time;
	}
	else{
		perror("No hay ningun temporizador corriendo.");
		exit(EXIT_FAILURE);
	}
}


