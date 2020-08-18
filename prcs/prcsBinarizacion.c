#include <unistd.h> //Para utilizar fork(), pipes(), entre otros
#include <stdio.h> //Funciones de entrada y salida como printf
#include <stdlib.h> //Asignación de memoria, atoi, etc.
#include <string.h> 
#include <sys/wait.h> //Define las constantes simbólicas para usar con waitpid(), wait() por ejemplo
#include <sys/types.h> //define varios tipos de datos como pid_t
#include <jpeglib.h>
#include "../incl/binarizacion.h"

#define LECTURA 0
#define ESCRITURA 1

int main(int argc, char *argv[]) {
	printf("soy el proceso hijo que binariza la imagen\n");
    //*******************************************************************************//
    //Leer mensajes del pipe del proceso padre
    int umbralBin;
	int umbralNeg;
	int flagResultados;
	int numImagen;
	char imagename[30];
    JpegData jpegData;
    pid_t pid;

    read(STDIN_FILENO, &umbralBin, sizeof(int));
    read(STDIN_FILENO, &umbralNeg, sizeof(int));
    read(STDIN_FILENO, &flagResultados, sizeof(int));
	read(STDIN_FILENO, imagename, 30);
	read(STDIN_FILENO, &jpegData, sizeof(JpegData));
	read(STDIN_FILENO, &numImagen, sizeof(int));


    //*******************************************************************************//
    //4. binarizar imagen
	jpegData = binarizarImagen(jpegData, umbralBin);


    //*******************************************************************************//
    //Crear nuevo pipe para enviar mensajes al proceso hijo
    int *pipe5 = (int*)malloc(sizeof(int)*2); //se reserva memoria para el pipe
	pipe(pipe5); //inicializa el pipe
    int status;


    //*******************************************************************************//
    //Crear proceso hijo
    pid = fork(); 
		if(pid < 0){
			fprintf(stderr, "No se pudo crear el proceso hijo" ); 
        	return 1;
		}

		if(pid > 0){ //Es el padre

			close(pipe5[LECTURA]); //El padre no va a leer, por lo tanto se cierra su descriptor
        	write(pipe5[ESCRITURA], &umbralNeg, sizeof(int));
			write(pipe5[ESCRITURA], &flagResultados, sizeof(int));
			write(pipe5[ESCRITURA], imagename, 30);
            write(pipe5[ESCRITURA], &jpegData, sizeof(JpegData));
			write(pipe5[ESCRITURA], &numImagen, sizeof(int));
			printf("al parecer soy el padre y mi pid es: %i\n" , getpid());
        	printf("Ya escribi el arr en el pipe\n");
			waitpid(pid, &status,0);
		}
		else{ //Es el hijo
            printf("Soy el hijo de la binarizacion");
			close(pipe5[ESCRITURA]); //Como el hijo no va a escribir, cierra el descriptor de escritura
			dup2(pipe5[LECTURA], STDIN_FILENO);
			execl("/prcs/prcsClasificacion", "ls","-al", NULL);
		}
    return 0; 
}