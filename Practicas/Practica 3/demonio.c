#include "mensaje.h"

//descripor del fichero de conficuracion 
int lockfd;
//ESTRUCTURAS NECESARIAS PARA LA SEÑAL SIGALARM
struct itimerval count;
struct itimerval it;
//para la señal sigpipe
extern int errno;
int signalEPIPE = 0;
int arrayEpipe;
//creacion de cola de mensajes
int msg_cs, msg_sc;
key_t llave;
//array para mandar la señal de alarma a cada cliente
int clientsArray[MAX];
int clientsCount=0;
 

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
	//borramos mecanismos IPC
	openlog("tiempo_demonio", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_DAEMON);
	syslog(LOG_NOTICE, "Eliminando colas y saliendo");
	closelog();                	
	msgctl(msg_cs,IPC_RMID,NULL);
	msgctl(msg_sc,IPC_RMID,NULL);    

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
	msg_t msg;
	strftime(hora, 100, "%H : %M : %S", tm);
			openlog("tiempo_demonio", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_DAEMON);
			syslog(LOG_NOTICE, "Mandando a  numero de clientes = %d", clientsCount);
			closelog();
	for(int j=0;j<clientsCount;j++){
		if(clientsArray[j]!=-1){//comprobamos que el cliente sigue conectado
			msg.dato.tipo=clientsArray[j];
			strcpy(msg.dato.mensaje,hora);
			if(msgsnd(msg_sc,&msg,LONG,0)==-1){
				openlog("tiempo_demonio", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_DAEMON);
				syslog(LOG_NOTICE, "Problema al mandar a los clientes la hora = %s", hora);
				closelog();
			}
		}
	}
	openlog("tiempo_demonio", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_DAEMON);
	syslog(LOG_NOTICE, "Se ha recibido la señal de alarma, hora actual = %s", hora);
	closelog();


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
	msg_t msg;	
	//crear las colas de mensajes s_c y c_s


	llave  = ftok(KEYFILE,TOK_CS);
	msg_cs = msgget(llave,IPC_CREAT|0600);

	llave  = ftok(KEYFILE,TOK_SC);
	msg_sc = msgget(llave,IPC_CREAT|0600); 
		    


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


	while (1) {

		if(msgrcv(msg_cs,&msg,LONG,0,0)>0){
				openlog("tiempo_demonio", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_DAEMON);
				syslog(LOG_NOTICE, "Se ha recibido ALGO");
				closelog();
				clientsArray[clientsCount]=msg.dato.tipo;
				clientsCount++;
		
			char times2[85];
			strcpy(times2,msg.dato.mensaje);
			if(strcmp(times2,"times")==0){
				openlog("tiempo_demonio", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_DAEMON);
				syslog(LOG_NOTICE, "Se ha recibido times por la cola de mensajes");
				closelog();
				//Se crea un hijo para atender la llamada 
				//pid = fork();
				//if (pid == 0){
					pid_t despid=msg.dato.tipo;
					int segundos=getTiempo();
					char snum[20];
					// convert 123 to string [buf]
					sprintf(snum, "%d", segundos);
		        		strcpy(msg.dato.mensaje,snum);
					//se envia el mensaje con el tipo determinado fijado al pid del cliente para que solo el pueda recibirlo
					msg.dato.tipo=despid;
					msgsnd(msg_sc,&msg,LONG,0);

					//exit(0);
				//}
			}
			if(strcmp(times2,"fin")==0){
				int encontrado=0;
				int count=0;
				openlog("tiempo_demonio", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_DAEMON);
				syslog(LOG_NOTICE, "Un cliente ha terminado.");
				closelog();
				while(!encontrado && count<=clientsCount){
					if(clientsArray[count]==msg.dato.tipo){
						encontrado=1;
						clientsArray[count]=-1;
					}
					count++;
				}			
				
				
			}
		}
	}
}

//Main del programa
int main() {
	demonio();
}
