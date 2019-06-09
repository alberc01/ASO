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

int main() {
	fflush(NULL);
	//SE CAMBIA EL DIRECTORIO AL /HOME
	char *bufer = getenv("HOME");
	chdir(bufer);
	int fd = open("/tmp/newclient", O_WRONLY);
	if (fd != -1) {
		int i = 0;
		int n;
		pid_t pid;
		char pathClient[20];
		char pathServer[20];
		char parRead[100];
		char imparRead[100];

		int bytesRead = 0;
		int bytesImparRead = 0;
		pid = getpid();
		snprintf(pathClient, sizeof(pathClient), "/tmp/toClient%d", pid);
		snprintf(pathServer, sizeof(pathServer), "/tmp/toServer%d", pid);

		if ((n = mkfifo(pathClient, 0666)) == -1) {
			perror("error en pathclient");

			exit(0);
		}
		if ((n = mkfifo(pathServer, 0666)) == -1) {
			perror("error en pathServer");

			exit(0);
		}

		int nBytes = write(fd, &pid, sizeof(pid));

		int iteraciones;
		srand(time(NULL));
		iteraciones = rand() % (10 - 1 + 1) + 1;   // rand % (N-M+1) + M; Este está entre M y N numero aleatorio entre 1 y 10

		int fdClient;
		if ((fdClient = open(pathClient, O_RDONLY)) == -1) {
			perror("fallo en toclient");
			remove(pathClient);
			remove(pathServer);
			exit(0);
		}
		int fdServer;
		if ((fdServer = open(pathServer, O_WRONLY)) == -1) {
			perror("fallo en toServer");
			remove(pathClient);
			remove(pathServer);
			exit(0);
		}

		while (i < iteraciones) {
			if (i % 2 == 0) {
				printf("se manda times\n");
				char time[20] = "times";
				int bytesWrited = write(fdServer, time, sizeof(time));
				if (bytesWrited <= 0) {
					perror("No se ha podido escribir en el lado del cliente");
					exit(0);
				}
				else {
					printf("se lee lo que le llega tras mandar times\n");
					if (fdClient != -1) {
						if ((bytesRead = read(fdClient, parRead, sizeof(parRead))) == -1) {
							perror("fallo en la lectura 1");
							remove(pathClient);
							remove(pathServer);
							exit(0);
						}
					}
					else {
						printf("ha habido un problema al leer del la tuberia cliente");
					}

				}

			}
			else {
				printf("No se manda times\n");
				if ((bytesImparRead = read(fdClient, imparRead, sizeof(imparRead))) == -1) {
					perror("fallo en la lectura 2");
					remove(pathClient);
					remove(pathServer);
					exit(0);
				}

			}
			if (bytesRead != 0) {

				printf("se han devuelto los segundos = %s\n", parRead);
				bytesRead = 0;
			}
			if (bytesImparRead != 0) {
				printf("Se ha devuelto la hora= %s\n", imparRead);
				bytesImparRead = 0;
				bytesRead = 0;

			}
			i++;
		}

		close(fdClient);
		close(fdServer);
		remove(pathClient);
		remove(pathServer);
		close(fd);
		exit(0);

	}
	else {
		perror("no se puede abrir newclient");
		exit(0);
	}

	return 0;
}
