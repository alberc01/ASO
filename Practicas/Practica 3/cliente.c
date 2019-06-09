#include "mensaje.h"
int main() {
	fflush(NULL);
	//SE CAMBIA EL DIRECTORIO AL /HOME
	char *bufer = getenv("HOME");
	//tipos de datos par la cola de mensajes
	msg_t msg;
	int msg_cs, msg_sc;
	key_t llave;


	//crear las colas de mensajes s_c y c_s
	llave  = ftok(KEYFILE,TOK_SC);
	msg_sc = msgget(llave,IPC_CREAT|0600); 

	llave  = ftok(KEYFILE,TOK_CS);
	msg_cs = msgget(llave,IPC_CREAT|0600); 

	int i = 0;
	int n;
	char parRead[100];
	char imparRead[100];
	int bytesRead = 0;
	int bytesImparRead = 0;

	int iteraciones;
	srand(time(NULL));
	iteraciones = rand() % (10 - 1 + 1) + 1;   // rand % (N-M+1) + M; Este está entre M y N numero aleatorio entre 1 y 10
	pid_t pid=getpid();
	while (i < iteraciones) {
		if (i % 2 == 0) {
			printf("se manda times\n");

			strcpy(msg.dato.mensaje,"times");
			msg.dato.tipo=pid;

			msgsnd(msg_cs,&msg,LONG,0);
			
			if(msgrcv(msg_sc,&msg,LONG,pid,0)>0)
				printf("se ha recibido: %s\n",msg.dato.mensaje);
			

		}
		else {
			printf("No se manda times\n");
			if(msgrcv(msg_sc,&msg,LONG,pid,0)>0)
			printf("se ha recibido: %s\n",msg.dato.mensaje);

		}
		
		i++;
	}
	//MANDAMOS INFORMACION DE DESCONEXION PARA QUE SEA ELIMINADO DEL ARRAY EN EL QUE ESTA EN EL LADO DEL DEMONIO
	strcpy(msg.dato.mensaje,"fin");
	msgsnd(msg_cs,&msg,LONG,0);

	exit(0);
	return 0;
}
