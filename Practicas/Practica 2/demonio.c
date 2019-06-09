#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>


#define LOG_FILE "time.log"

#ifndef MAXTUB
#define MAXTUB 1000
#endif

//-----------TUBERIAS--------------------------
//array de tuberias de clientes
int tubLectores[MAXTUB];
int tubEscritores[MAXTUB];
//variable para actualizar el array de tuberias
int lectoresInd = 0;
int escritInd = 0;

//---------------------------------------------

//descripor del fichero de conficuracion 
int lockfd;
//ESTRUCTURAS NECESARIAS PARA LA SEÑAL SIGALARM
struct itimerval count;
struct itimerval it;
//para la señal sigpipe
extern int errno;
int signalEPIPE = 0;
int arrayEpipe;
// conjunto de descriptores de fichero de lectura
fd_set conjuntoRead;



void leerConf() {
	int fd;
	if ((fd = open("tiempo.conf", O_RDWR)) != -1) {
		char buf[100];
		read(fd, buf, sizeof(int));
		close(fd);
		//leemos y fijamos el tiempo de espera de la alarma
		int segundos = atoi(buf);
		it.it_value.tv_sec = segundos;
		it.it_value.tv_usec = 0;

		it.it_interval.tv_sec = segundos;
		it.it_interval.tv_usec = 0;

		//FIJAMOS EL TIMPER QUE VAMOS A USAR.
		setitimer(ITIMER_REAL, &it, NULL);
	}
	else {
		openlog("tiempo_demonio", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_DAEMON);
		syslog(LOG_NOTICE, "No existe el fichero tiempo.conf hace falta crearlo en el directorio ~/HOME");
		closelog();
		for (int i = 0; i < lectoresInd; i++) {
			if (tubLectores[i] != -1)
				close(tubLectores[i]);
		}
		remove("/tmp/newclient");
		exit(0);
	}
}
void handPipe(int sig) {
	openlog("tiempo_demonio", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_DAEMON);
	syslog(LOG_NOTICE, "Se ha cerrado la conexion con una tuberia");
	closelog();

}
void handTerm() {
	close(lockfd);
	if (remove("tiempo.lock") == -1) {
		openlog("tiempo_demonio", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_DAEMON);
		syslog(LOG_NOTICE, "no se puede eliminar el archivo tiempo.lock");
		closelog();
	}
	if (remove("/tmp/newclient") == -1) {
		openlog("tiempo_demonio", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_DAEMON);
		syslog(LOG_NOTICE, "no se puede eliminar la tuberia newclient");
		closelog();
	}
	for (int i = 0; i < escritInd; i++) {
		if (tubLectores[i] != -1)
			close(tubLectores[i]);
	}
	//terminamos el demonio
	openlog("tiempo_demonio", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_DAEMON);
	syslog(LOG_NOTICE, "se ha terminado el demonio");
	closelog();
	exit(0);

}
void handHup() {
	openlog("tiempo-demonio", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_DAEMON);
	syslog(LOG_NOTICE, "Se comprueba si se ha modificado el archivo tiempo.conf y se realiza lectura");
	closelog();
	leerConf();
}
void handAl() {
	char hora[30];
	time_t t;
	struct tm *tm;
	t = time(NULL);
	tm = localtime(&t);
	strftime(hora, 100, "%H : %M : %S", tm);
	int i;
	for (i = 1; i < escritInd; i++) {
		if (tubEscritores[i] != -1) {
			int bytesWrited;
			if (((bytesWrited = write(tubEscritores[i], hora, sizeof(hora))) == -1) && (errno == EPIPE)) {
				signalEPIPE = 1;
				arrayEpipe = i;
			}
			if (bytesWrited == 0) {
				openlog("tiempo_demonio", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_DAEMON);
				syslog(LOG_NOTICE, "No se ha podido escribir la hora tras recibir SIGALRM en las tuberias");
				closelog();
			}
			else {
				openlog("tiempo_demonio", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_DAEMON);
				syslog(LOG_NOTICE, "Se ha recibido la señal de alarma, hora actual = %s", hora);
				closelog();
			}
		}
	}
}
void handUsr() {
	char hora[30];
	time_t t;
	struct tm *tm;
	t = time(NULL);
	tm = localtime(&t);
	strftime(hora, 100, "%H : %M : %S", tm);
	openlog("tiempo_demonio", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_DAEMON);
	syslog(LOG_NOTICE, "Se ha recibido la señal de alarma, hora actual = %s", hora);
	closelog();
}

void * ini_manejador(int signal, void  *manejador)
{
	struct sigaction act;
	struct sigaction old_act;

	int i;

	act.sa_handler = manejador;

	sigemptyset(&act.sa_mask);

	act.sa_flags = SA_RESTART;

	sigaction(signal, &act, &old_act);
}

void bloquearSeniales() {
	sigset_t grupo;//señles que seran bloqueadas
	sigfillset(&grupo);
	sigdelset(&grupo, SIGALRM);
	sigdelset(&grupo, SIGHUP);
	sigdelset(&grupo, SIGUSR1);
	sigdelset(&grupo, SIGTERM);
	sigdelset(&grupo, SIGPIPE);
	sigprocmask(SIG_BLOCK, &grupo, NULL);


}
void iniciarSeniales() {
	ini_manejador(SIGALRM, handAl);
	//SEÑAL PARA LEER NUEVAMENTE EL FICHERO DE CONFIGURACION
	ini_manejador(SIGHUP, handHup);
	//SEÑAL PARA FORZAR LA ALARMA 
	ini_manejador(SIGUSR1, handUsr);
	//SEÑAL PARA TEMIRAR EL PROCESO DE FORMA ORDENADA
	ini_manejador(SIGTERM, handTerm);
	//SEÑAL PARA indicar que se ha cerrado el extremo de lectura
	ini_manejador(SIGPIPE, handPipe);
}
int getTiempo() {
	int segundos;
	//estructuras para obtener el tiempo
	time_t tim;
	struct tm *tmAct;
	tim = time(NULL);
	return tim;

}
void demonio() {
	pid_t pid;
	int i;
	char str[10];

	//creacion de la tuberia
	mkfifo("/tmp/newclient", 0666);

	//CREAR EL PROCESO PARA QUE EL PADRE HAGA EXIT, EL HIJO SERA EL NUEVO DEMONIO.
	pid = fork();
	if (pid < 0) {
		perror("fallo en FORK");
		exit(EXIT_FAILURE);
	}
	else if (pid > 0) {
		exit(EXIT_SUCCESS);
	}
	//EL PROCESO HIJO PASA A SER LIDER DE LA SESION.
	setsid();
	//cerrar todos los descriptores de fichero de la tabla del proceso hijo.
	for (i = getdtablesize(); i >= 0; --i)
		close(i);
	//reapertura de entrada salida estandar
	i = open("/dev/null", O_RDWR);
	dup(i);
	dup(i);
	//PONER LA MASCARA A 0
	umask(0);
	//SE CAMBIA EL DIRECTORIO AL /HOME
	char *bufer = getenv("HOME");
	chdir(bufer);

	//grantizamos una unica instancia para el demonio en un fichero
	lockfd = open("tiempo.lock", O_RDWR | O_CREAT, 0640);
	if (lockfd < 0) {
		perror("No se puede abrir el archivo tiempo.lock");
		exit(1);
	}
	if (lockf(lockfd, F_TLOCK, 0) < 0) {
		perror("No se puede bolquear");
		exit(0);
	}
	sprintf(str, "%d\n", getpid());
	write(lockfd, str, strlen(str));

	//apertura y lectura del archivo tiempo.conf para indicar cada cuanto tiempo se mandara la señal SIGALARM
	leerConf();

	//tratamiento de señales
	//Bloqueamos las señales que no estan contempladas
	bloquearSeniales();
	//iniciamos el handler de la señal SIGALARM
	iniciarSeniales();



	//cambios == 0 si no se ha recibido nada por la tuberia
	int cambios;
	//tiempo que esperara el select hasta recibir cambios
	struct timeval timeout;


	//Inicializacion del conjunto de tuberias y el array de tuberias
	FD_ZERO(&conjuntoRead);
	int fdTub = open("/tmp/newclient", O_RDONLY);

	tubLectores[0] = fdTub;// arrayTUB[0], se ha recibido algo por la tuberia /tmp/newClient
	int max = tubLectores[0];
	FD_SET(tubLectores[0], &conjuntoRead);
	//bucle para el proceso, escucha de tuberias
	tubEscritores[0] = -1;
	lectoresInd++;
	escritInd++;
	char cadena[20];
	int bytesRead;
	int bytesWrite;
	while (1) {

		FD_ZERO(&conjuntoRead);

		for (int x = 0; x < lectoresInd + 1; x++) {
			if (tubLectores[x] > -1) {
				FD_SET(tubLectores[x], &conjuntoRead);
				if (max < tubLectores[x]) {
					max = tubLectores[x];
				}
			}

		}
		// esperamos a que pasen 25 segundos en estado de bloqueo
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;
		cambios = select(max + 1, &conjuntoRead, NULL, NULL, &timeout);
		//leerConf();
		if (cambios == 0) {
			openlog("tiempo_demonio", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_DAEMON);
			syslog(LOG_NOTICE, "Time out! de select");
			closelog();

		}
		else {
			int nBytes;
			int clientPid;
			if (FD_ISSET(tubLectores[0], &conjuntoRead)) {//si leemos dato en la tuberia /tmp/newClient abrimos los extremos de lectura y escritura
				if ((nBytes = read(tubLectores[0], &clientPid, 30)) > 0) {
					char path1[20];
					char path2[20];
					snprintf(path1, sizeof(path1), "/tmp/toServer%d", clientPid);// se abre la tuberia /tmp/toServer(pidProceso)
					snprintf(path2, sizeof(path2), "/tmp/toClient%d", clientPid);// se abre la tuberia /tmp/toClient(pidProceso)


					int toClientfd = open(path2, O_WRONLY);//abre la tuberia solo con permisos de escritura

					int toServerfd = open(path1, O_RDONLY);//abre la tuberia solo con permisos de lectura

					if (toServerfd != -1 && toClientfd != -1) {

						openlog("tiempo_demonio", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_DAEMON);
						syslog(LOG_NOTICE, "Se abre las tuberias %s y %s", path1, path2);
						closelog();
						tubEscritores[escritInd] = toClientfd;//posiciones impares la tuberia hacia el cliente determinado,impares write
						escritInd++;
						tubLectores[lectoresInd] = toServerfd;//posiciones pares la tuberia del servidor,pares read
						lectoresInd++;
					}
					else {
						openlog("tiempo_demonio", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_DAEMON);
						syslog(LOG_NOTICE, "Las tuberias no se pueden abrir");
						closelog();
						exit(0);


					}
				}
			}
			else {


				if (signalEPIPE == 1) {
					//Quitamos los descriptores del set
					FD_CLR(tubLectores[arrayEpipe + 1], &conjuntoRead);
					//Cerramos los dos descriptores de fichero									
					close(tubEscritores[arrayEpipe]);
					close(tubLectores[arrayEpipe]);
					//Eliminamos los descriptores del array de descriptores abiertos
					tubEscritores[arrayEpipe] = -1;
					tubLectores[arrayEpipe] = -1;
					signalEPIPE = 0;
				}

				for (int j = 1; j < lectoresInd + 1; j++) {
					if (FD_ISSET(tubLectores[j], &conjuntoRead)) {
						if ((bytesRead = read(tubLectores[j], cadena, sizeof(cadena))) == -1) {
							openlog("tiempo_demonio", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_DAEMON);
							syslog(LOG_NOTICE, "Fallo al intentar leer de la tuberia");
							closelog();
							exit(0);
						}
						if (bytesRead > 0) {
							char comprobacion[20] = "times";
							int cmp = strcmp(cadena, comprobacion);
							openlog("tiempo_demonio", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_DAEMON);
							syslog(LOG_NOTICE, "Comparacion con times");
							closelog();
							if (cmp == 0) {
								int segundos = getTiempo();
								char snum[20];
								// convert 123 to string [buf]
								sprintf(snum, "%d", segundos);
								bytesWrite = write(tubEscritores[j], snum, sizeof(snum));
							}
						}
					}
				}

			}
		}
	}
}

//Main del programa
int main() {
	demonio();
}
