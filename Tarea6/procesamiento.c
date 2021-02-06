/** @brief: Este programa muestra el uso del divice driver de caracter
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>

#define MUESTRAS 4096
#define N 4096


short int hamming[MUESTRAS];
char buffer[N], buffer_read[N];

void genera_seno(short int seno[]);
void guarda_datos(short int datos[], char *archivo);
void guarda_corr(long int datos[], char *archivo);
void genera_hamming();
void guarda_hamming();

int main()
{

	short int seno [MUESTRAS];
    	long int seno_proc[MUESTRAS];
	int fd, len;

 	fd = open( "/dev/ESCOM_device", O_RDWR );
	if( fd == -1 )
	{
		perror("Error al abrir el DDC \n");
		exit( EXIT_FAILURE );
  	}
    
	genera_seno(seno);
	guarda_datos(seno, "seno.dat");
    genera_hamming();
    guarda_hamming();
	guarda_datos(hamming,"hamming.dat");

	len = write(fd, seno, MUESTRAS);
	printf("Muestras enviadas: %d\n",len);

	

	sleep(1);

	len = read(fd, seno_proc,MUESTRAS);
	printf("Muestras recibidas: %d\n",len);


	guarda_corr(seno_proc,"seno_proc.dat");
	close( fd );

	return 0;
}

void genera_seno(short int seno[]){

	float f = 1.3, fs = 512, muestra;
	register int n;


	for(n=0; n < MUESTRAS;n++){
		muestra = sinf( 2*M_PI*n*f/fs );
		seno[n] =(short int )( muestra * 4096 );
//        seno[n] =(short int)( muestra *22);
		//seno[n] = (short int)(muestra);
	}

}

void guarda_datos(short int datos[], char *archivo){
	FILE *apArch;
	register int n;
	apArch = fopen( archivo ,"w");
	
    if(apArch == NULL){

	    perror("Error al abrir el archivo");
	    exit(EXIT_FAILURE);

	}

	for(n=0;n<MUESTRAS;n++){
		fprintf(apArch,"%d\n",datos[n]);
	}

	fclose(apArch);

}


void guarda_corr(long int datos[], char *archivo){
	FILE *apArch;
	register int n;
	apArch = fopen( archivo ,"w");
	
    if(apArch == NULL){

	    perror("Error al abrir el archivo");
	    exit(EXIT_FAILURE);

	}

	for(n=0;n<MUESTRAS;n++){
		fprintf(apArch,"%ld\n",datos[n]);
	}

	fclose(apArch);

}

void genera_hamming(){

	float a0 = 0.53836, a1 = 0.46164, v;
	register int n;

	for(n = 0; n < MUESTRAS; n++){
		v = a0 - (a1 * cosf(2 * n * M_PI / (MUESTRAS - 1)));
		hamming[n] = (short int)(v * (1 << 15));//Q15
	}
}

void guarda_hamming(){

    FILE *apArch;
    register int n;
    apArch = fopen("ventana.h", "w");
    
    if( apArch == NULL ){
        perror("Error al abrir el archivo\n");
        exit( EXIT_FAILURE );
    }

    fprintf(apArch, "short int hamming[4096]={%d", hamming[0]);

    for(n = 1; n < N; n++){
    
        fprintf(apArch,",%d",hamming[n]);

    }

    fprintf( apArch, "};");
    fclose( apArch );

}

