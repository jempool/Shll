#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>


void asignarArgumentos(char ** arrayArgs, char *linea, int *text, int *doPipe, char ** pipeArray) {
    char * aux;
    int contador = 0, contador2 = 0;
    aux = strtok(linea, " ");

    while (aux != NULL) {
        arrayArgs[contador] = strdup(aux);
        aux = strtok(NULL, " ");
        contador++;
        if (aux == NULL)break;

        if (!strcmp(">", aux)) {
            *text = contador + 2;
            arrayArgs[contador] = NULL;
            contador++;
        }

        if (!strcmp("|", aux)) {
            *doPipe = 1;
            contador2 = -1;
            while (aux != NULL) {
                pipeArray[contador2] = strdup(aux);
                aux = strtok(NULL, " ");
                contador2++;
                if (aux == NULL)break;
            }
            pipeArray[contador2] = NULL;
            break;
        }
    }
    arrayArgs[contador] = NULL;
}

void prepararSalidaArchivo(int posicion, int* archivo, char ** args) {

    if (posicion) {
        close(STDOUT_FILENO);
        char *filename = args[posicion];
        *archivo = open(filename, O_CREAT | O_WRONLY | O_TRUNC | S_IRWXU);

    }
}

int main(int argc, char*argv[]) {

    int p[2];
    char buf[512];
    FILE *swap;

    if (pipe(p) < 0)
        exit(1);

    while (1) {
        char comando[80];
        printf("$ ");
        scanf(" %[^\n]s", comando);
        if (!strcmp("exit", comando)) {
            break;
        }
        pid_t pid = fork();
        int salidaTexto = 0;
        int fd;
        int doPipe = 0;

        if (!pid) {
            char * myargs[100], * pipeArray[100];
            asignarArgumentos(myargs, comando, &salidaTexto, &doPipe, pipeArray);

            if (salidaTexto) {
                printf(". ");
                close(STDOUT_FILENO);
                char *filename = myargs[salidaTexto];
                fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC | S_IRWXU | S_IXOTH);
            }

            if (doPipe) { // Con pipe //    cat README.md | wc -l
                pid_t frk1 = fork();
                if (frk1 == 0) {
                    swap = freopen("swap.txt", "w+", stdout);
                    int aux = execvp(myargs[0], myargs);
                    fclose(swap);
                    if (aux == -1)printf("Este comando no pudo ser ejecutado\n");
                    break;
                } else {
                    waitpid(frk1, NULL, 0);
                    pid_t frk2 = fork();
                    if (frk2 < 0) {
                        fprintf(stderr, "fallo fork 2\n");
                        exit(1);
                    } else if (frk2 == 0) {
                        int i = 0;
                        while (1) {
                            if (pipeArray[i] == NULL) {
                                pipeArray[i] = "swap.txt";
                                pipeArray[i + 1] = NULL;
                                break;
                            }
                            i++;
                        }
                        execvp(pipeArray[0], pipeArray);
                        close(p[1]);                      
                        read(p[0], buf, 512); 
                        printf("%s\n", buf);
                    } else {
                        wait(NULL);
                    }
                }
            } else { // Sin pipe //
                int aux = execvp(myargs[0], myargs);
                if (aux == -1)printf("Este comando no pudo ser ejecutado\n");
                if (salidaTexto) close(fd);
                if (fd < 0) {
                    printf("%d  error!", fd);

                }
                printf("%d  error!", fd);
                break;
            }
        } else {
            waitpid(pid, NULL, 0);
        }
    }
}
