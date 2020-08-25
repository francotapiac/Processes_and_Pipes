#include <unistd.h> //Para utilizar fork(), pipes(), entre otros
#include <stdio.h> //Funciones de entrada y salida como printf
#include <stdlib.h> //Asignación de memoria, atoi, etc.
#include <string.h> 
#include <sys/wait.h> //Define las constantes simbólicas para usar con waitpid(), wait() por ejemplo
#include <sys/types.h> //define varios tipos de datos como pid_t
#include <jpeglib.h>
#include "../incl/filtro.h"
#include "../incl/lecturaImagenes.h"

#define LECTURA 0
#define ESCRITURA 1

int main(int argc, char *argv[])
{
    //Se crean las variables para almacenar los datos leidos del pipe34
    int umbralBin = 0;
    int umbralNeg = 0;
    int flagResultados = 0;
    int numImagen = 0;
    int lenNombreMasc = 0;
    int height = 0;
    int width = 0;
    int lenImagen = 0;
    JpegData jpegData;

    //Se leen los datos del pipe34
    read(STDIN_FILENO, &umbralBin, sizeof(int));
    read(STDIN_FILENO, &umbralNeg, sizeof(int));
    read(STDIN_FILENO, &flagResultados, sizeof(int));
    read(STDIN_FILENO, &numImagen, sizeof(int));
    read(STDIN_FILENO, &lenNombreMasc, sizeof(int));
    read(STDIN_FILENO, &height, sizeof(int));
    read(STDIN_FILENO, &width, sizeof(int));
    lenImagen = height*width;
    jpegData.height = height;
    jpegData.width = width;
    jpegData.ch = 1;
    alloc_jpeg(&jpegData);
	uint8_t dataImagen[lenImagen];
    read(STDIN_FILENO, dataImagen, sizeof(uint8_t)*lenImagen);
    char nombreArchivoMasc[lenNombreMasc];
    read(STDIN_FILENO, nombreArchivoMasc, lenNombreMasc*sizeof(char));

	for (int i = 0; i < lenImagen; i++)
	{
		jpegData.data[i] = dataImagen[i];
	}
	

    //-------------------------------------------------------------------
    //Se aplica el Filtro Laplaciano
	printf("la mascara se llama == %s\n", nombreArchivoMasc);
    int **mascara = leerMascara(nombreArchivoMasc);
    jpegData = aplicarFiltroLaplaciano(jpegData,mascara);
    liberarMascara(mascara);
	//-----------------------------------------------------------------

	for (int i = 0; i < lenImagen; i++)
	{
		dataImagen[i] = jpegData.data[i];
	}
	

    //------------------------------------------------------------------
    //Se crea un nuevo pipe y un nuevo proceso
    int pipe45[2];
    int status;
    pid_t pid;
    pipe(pipe45);

    pid = fork();
    if(pid < 0) //No se pudo crear el hijo
    {
        fprintf(stderr, "No se pudo crear el proceso de Binarizacion\n");
        return 1;
    }

    else if (pid > 0) //es el padre
    {
        close(pipe45[LECTURA]);
        //se escriben los datos en el pipe45
        write(pipe45[ESCRITURA], &umbralBin, sizeof(int));
        write(pipe45[ESCRITURA], &umbralNeg, sizeof(int));
        write(pipe45[ESCRITURA], &flagResultados, sizeof(int));
        write(pipe45[ESCRITURA], &numImagen, sizeof(int));
        write(pipe45[ESCRITURA], &(jpegData.height), sizeof(int));
        write(pipe45[ESCRITURA], &(jpegData.width), sizeof(int));
        write(pipe45[ESCRITURA], dataImagen, sizeof(uint8_t)*lenImagen);

        //Se espera al hijo
        waitpid(pid, &status, 0);
    }

    else //es el hijo
    {
        close(pipe45[ESCRITURA]);
        dup2(pipe45[LECTURA], STDIN_FILENO);
        
        //Se cambia el codigo del hijo
        char *args[]={"./pBinarizacion",NULL}; 
        execv(args[0],args);
    }

    liberarJpeg(&jpegData);
    printf("El proceso Filtro termina su ejecución\n");
    return 1;
    
}