#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

int sudoku[9][9];
int pid;
int pipefd[2];
int pipes[27][2];

void cargaSudoku(){
    FILE *fp;
    fp=fopen("sudoku.txt","r");
    if(fp==NULL){
        printf("No se pudo abrir el archivo sudoku.txt\n");
        exit(EXIT_FAILURE);
    }
    int i,j;
    for(i=0;i<9;i++){
        for(j=0;j<18;j++){
            char c;
            c=fgetc(fp);
            if(j%2 == 0){
                if(c >= '0' && c <= '9'){
                    int n=atoi(&c);
                    sudoku[i][j/2] = n;
                }
            }
        }
    }
}
void crearPipes(){
    int i;
    for(i=0;i<27;i++){
        if(pipe(pipes[i])==-1){
            perror("Error pipe");
            exit(EXIT_FAILURE);
        }
    }
}
//checkea si una subseccion de 3x3 es valida
void subValida(int fil, int col){
    close(pipes[fil+col/3][0]);
    int visto[9] = {0};
        int i, j;
        for (i = fil; i < fil + 3; i++) {
            for (j = col; j < col + 3; j++) {
                int num = sudoku[i][j];
                if (visto[num - 1] == 1) {
                    write(pipes[fil+col/3][1],"f",strlen("f")+1);
                    close(pipes[fil+col/3][1]);
                    exit(EXIT_SUCCESS);
                } else {
                    visto[num - 1] = 1;
                }
            }
        }
    //la subseccion es valida
    write(pipes[fil+col/3][1],"t",strlen("t")+1);
    close(pipes[fil+col/3][1]);
    exit(EXIT_SUCCESS);
}
//checkea si una columna es valida
void colValida(int fil, int col){
    close(pipes[18+col][0]);
    int visto[9] = {0};
        int i;
        for (i = 0; i < 9; i++) {
            int num = sudoku[i][col];
            if (visto[num - 1] == 1) {
                write(pipes[18+col][1],"f",strlen("f")+1);
                close(pipes[18+col][1]);
                exit(EXIT_SUCCESS);
            } else {
                visto[num - 1] = 1;
            }
        }
    //la columna es valida
    write(pipes[18+col][1],"t",strlen("t")+1);
    close(pipes[18+col][1]);
    exit(EXIT_SUCCESS);
}
//checkea si una fila es valida
void filValida(int fil, int col){
    close(pipes[9+fil][0]);
    int visto[9] = {0};
    int i;
    for (i = 0; i < 9; i++) {
            int num = sudoku[fil][i];
            if (visto[num - 1] == 1) {
                write(pipes[9+fil][1],"f",strlen("f")+1);
                close(pipes[9+fil][1]);
                exit(EXIT_SUCCESS);
            } else {
                visto[num - 1] = 1;
            }
        }
    //la fila es valida
    write(pipes[9+fil][1],"t",strlen("t")+1);
    close(pipes[9+fil][1]);
    exit(EXIT_SUCCESS);
}
//creo los 27 procesos
void crearProcesos(){
    int i,j;
    for (i = 0; i < 9; i++) {
		for (j = 0; j < 9; j++) {
			if (i%3 == 0 && j%3 == 0) {//para un subseccion
				pid=fork();
				if(pid<0){
                    perror("Error fork");
                    exit(EXIT_FAILURE);
				}
                if(pid==0){
                    subValida(i,j);
                }
                if(pid>0){
                    close(pipes[i+j/3][1]);
                }
			}
			if (i == 0) {//para una columna
				pid=fork();
				if(pid<0){
                    perror("Error fork");
                    exit(EXIT_FAILURE);
				}
                if(pid==0){
                    colValida(i,j);
                }
                if(pid>0){
                    close(pipes[j][1]);
                }
			}
			if (j == 0) {//para una fila
				pid=fork();
				if(pid<0){
                    perror("Error fork");
                    exit(EXIT_FAILURE);
				}
                if(pid==0){
                    filValida(i,j);
                }
                if(pid>0){
                    close(pipes[i][1]);
                }
			}
		}
	}
}
int main()
{
    cargaSudoku();
    crearPipes();
    crearProcesos();
    while(wait(NULL)){//espero a que terminen los procesos
        if (errno == ECHILD){
            break;
        }
    }
    int i;
    int valid = 1;
    char c[2];
    while(valid && i<27){//checkeo el resultado de los procesos
        read(pipes[i][0],c,strlen(c)+1);
        valid = valid && (strcmp("t",c)==0);
        close(pipes[i][0]);
        i++;
    }
    if(valid){
        printf("El sudoku es valido!");
    }
    else{
        printf("El sudoku es invalido!");
    }

    return 0;
}
