#include"timing.h"
#include<unistd.h>
#include <stdio.h>

void esperas(){
	resume();
	sleep(2);
	int j =0;
	for(int i=0;i<=500;i++){	
		if(i%50==0){
			printf("%d microseconds, timer paused",pause());
			printf("\n");
			resume();
			printf("sleeping 2 seconds");
			printf("\n");
			sleep(2);
			
		}
		if(j==51){
			 printf("sleeping 1 second");
			 sleep(1);
			 printf("\n");
			 j=0;
		}
		j++;
	}
	printf("wait the timer to close\n");
	sleep(4);
	printf("6 seconds wasted on close");
	printf("\n");
	printf("microseconds, timer stopped.");
	printf("\n");

}

int main(){
	int copyTime;
	int waitTime;
    start();
    printf("Timer started. Copying file /etc/mailcap\n");
    FILE *file1 , *file2;
    int data1 =0;

    file1 = fopen ( "/etc/mailcap", "r" );
    file2 = fopen ( "copia.txt" ,"w+");

    while ( (data1 = fgetc ( file1 )) != EOF ) {
        fputc ( data1, file2 );
	sleep(0.001);
    }
    copyTime=pause();
    printf("The file has been copied in %d microseconds\n", copyTime);
    esperas();
    fclose ( file1 );
    fclose ( file2 );
    waitTime = stop();
    printf("%d microseconds, timer stopped.\n",waitTime);
    printf("Total time used: %d microseconds\n", waitTime + copyTime);
	
return 0;
}

