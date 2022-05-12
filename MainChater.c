#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <mqueue.h>
#include <unistd.h>
#include <string.h>

#define MAX_ARRAY 32
#define MAX_COUNT 5

struct threadParam_t
{
    mqd_t *queue;
};

char str[64];
char estadoActivacion = 1;

void *EscribirCola(void *parg)
{
    struct threadParam_t *pargTmp = (struct threadParam_t *)parg;

    while (1)
    {
        if (fgets(str, sizeof(str), stdin))
        {
            if (str[strlen(str) - 1] == '\n')
                str[strlen(str) - 1] = 0;

            if (strncmp(str, "exit", strlen("exit")) == 0)
            {
                estadoActivacion = 0;
                break;
            }

            mq_send(*pargTmp->queue, str, strlen(str) + 1, 0);
        }
    }

    mq_close(*pargTmp->queue);
    exit(EXIT_FAILURE);

    return NULL;
}

int main()
{
    pthread_t threadID1;

    mqd_t mq, mq1;

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 32;
    attr.mq_curmsgs = 0;

    mq = mq_open("/mq0", O_RDONLY | O_CREAT, 0644, &attr);
    mq1 = mq_open("/mq1", O_WRONLY | O_CREAT, 0644, &attr);

    struct threadParam_t threadParam1 = {&mq1};

    char buff[32];

    pthread_create(&threadID1, NULL, &EscribirCola, (void *)&threadParam1);

    char array1[MAX_COUNT][MAX_ARRAY];
    // Útil para leer el archivo
    char buferArchivo[MAX_ARRAY];
    // Abrir el archivo...
    FILE *archivo = fopen("data", "r");

    if (archivo == NULL)
    {
        printf("No se puede abrir el archivo");
        return 0;
    }

    int contador = 0;

    while (fgets(buferArchivo, MAX_ARRAY, archivo))
    {
        // Remover salto de línea
        strtok(buferArchivo, "\n");
        // Copiar la línea a nuestro arreglo, usando el índice
        memcpy(array1[contador], buferArchivo, MAX_ARRAY);
        contador++;
    }

    fclose(archivo);

    while (estadoActivacion == 1)
    {
        mq_receive(mq, buff, MAX_ARRAY, NULL);

        if (buff[strlen(buff) - 1] != '\n')
        {

            if (strncmp(buff, "car", strlen("car")) == 0)
            {
                for (int i = 0; i < MAX_COUNT; i++)
                {
                    printf("%d.", i + 1);
                    printf("%s\n", array1[i]);
                }
            }
            else if (strncmp(buff, "reverse", strlen("reverse")) == 0)
            {
                for (int i = MAX_COUNT - 1; i >= 0; i--)
                {
                    printf("%d.", i + 1);
                    printf("%s\n", array1[i]);
                }
            }
            else
            {
                printf("Message received: %s\n", buff);
            }
        }
    }

    pthread_exit(NULL);

    return (0);
}