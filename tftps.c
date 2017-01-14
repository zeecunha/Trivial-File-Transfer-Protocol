//
//  tftps.c
//  Adapted by Christophe Soares & Pedro Sobral on 15/16
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>

#define BUFSIZE 8096

#define OperationMode 2  //0 multi-process 1 multi-thread


#if (OperationMode==1)
typedef struct  //struct que para os argumentos do ftp
{
    int socketfd;
    int hit;
} THREAD1_ARGS;
#endif


#if (OperationMode==2)
typedef struct //estrutura para semaforos
{
    int *buff;  //aponta paa a lista de inteiros
    size_t buffSize;    //tamanho da lista
    int firstFree;  //primeiro slot vazio
    int firstFUll;  //primeiro slot cheio

    sem_t full;
    sem_t empty;

    pthread_mutex_t mutex;
}  STRUCT_CONSUMIDOR;


STRUCT_CONSUMIDOR shared;
#endif


void getFunction(int fd, char * fileName)
{
    int file_fd;
    long ret;

    long long int startTime = 0;
    long long int finishTime = 0;
    long long int difTime = 0;

    //gettimeuseconds(&startTime);    //tempo no inicio

    static char buffer[BUFSIZE + 1]; /* static so zero filled */

    if ((file_fd = open(fileName, O_RDONLY)) == -1)   /* open the file for reading */
    {
        printf("ERROR failed to open file %s\n", fileName);
        close(fd);
    }

    printf("LOG SEND %s \n", fileName);

    /* send file in 8KB block - last block may be smaller */
    while ((ret = read(file_fd, buffer, BUFSIZE)) > 0)
    {
        write(fd, buffer, ret);
    }

    //gettimeuseconds(&finishTime); //tempo final
    difTime = finishTime - startTime;
    printf("\n tempo: %lld\n", difTime);    //imprimir o tempo final
}


void putFunction(int fd, char * fileName)
{
    int file_fd;
    long ret;

    long long int startTime = 0;
    long long int finishTime = 0;
    long long int difTime = 0;

    //gettimeuseconds(&startTime);    //tempo no inicio

    static char buffer[BUFSIZE + 1]; /* static so zero filled */


    printf("LOG Header %s \n", fileName);

    file_fd = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, 0777);
    if (file_fd == -1)
    {
        sprintf(buffer, "ERROR");
        write(fd, buffer, strlen(buffer));
    }
    else
    {
        sprintf(buffer, "OK");
        write(fd, buffer, strlen(buffer));

        while ((ret = read(fd, buffer, BUFSIZE)) > 0)
            write(file_fd, buffer, ret);
    }

    //gettimeuseconds(&finishTime); //tempo final
    difTime = finishTime - startTime;
    printf("\n tempo: %lld\n", difTime);    //imprimir o tempo final

}

void lsFunction(int fd, char * fileName)
{

    DIR *cdir;
    struct dirent *dit;
    struct stat statbuf;
    char lstring[200] ="";
    char temp[200]="";

    long long int startTime = 0;
    long long int finishTime = 0;
    long long int difTime = 0;

    //gettimeuseconds(&startTime);    //tempo no inicio


    /** Chamada ls para fornecer ao cliente todos os ficheiros que se encontram dentro
    da pasta especifica*/

    if((cdir = opendir(fileName)) == NULL)
    {
        strcat(lstring,"Error!");
    }

    /*Read all items in directory*/
    while((dit = readdir(cdir)) != NULL)
    {

        if(strcmp(dit->d_name, ".") == 0 || strcmp(dit->d_name, "..") == 0)
            continue;

        /*Checks if current item is of the type file (type 8)*/

        if(S_ISREG(statbuf.st_mode))
        {
            printf("\"%s\"\n", dit->d_name);

        }
        sprintf(temp,"%s\n",dit->d_name);
        strcat(lstring, temp);
    }

    lstring[strlen(lstring)] = '\0';
    puts(lstring);
    write(fd, lstring, strlen(lstring));

    //gettimeuseconds(&finishTime); //tempo final
    difTime = finishTime - startTime;
    printf("\n tempo: %lld\n", difTime);    //imprimir o tempo final
}



#if(OperationMode==0)
/* this is the ftp server function */
int ftp(int fd, int hit)
{
#endif
#if(OperationMode==2)
int ftp(int fd, int hit)
{
#endif
#if(OperationMode==1)
    void  * ftp(void * args)
    {
        THREAD1_ARGS* p =(THREAD1_ARGS*) args;
        int fd, hit;
        fd=p->socketfd;
        hit=p->hit;

        free(p);
#endif

        int j, file_fd, filedesc;
        long i, ret, len;
        char * fstr;
        static char buffer[BUFSIZE + 1]; /* static so zero filled */

        ret = read(fd, buffer, BUFSIZE); // read FTP request

        if (ret == 0 || ret == -1)   /* read failure stop now */
        {
            close(fd);
            //return 1;
        }
        if (ret > 0 && ret < BUFSIZE) /* return code is valid chars */
            buffer[ret] = 0; /* terminate the buffer */
        else
            buffer[0] = 0;

        for (i = 0; i < ret; i++) /* remove CF and LF characters */
            if (buffer[i] == '\r' || buffer[i] == '\n')
                buffer[i] = '*';

        printf("LOG request %s - hit %d\n", buffer, hit);

        /* null  terminate after the second space to ignore extra stuff */
        for (i = 4; i < BUFSIZE; i++)
        {
            if (buffer[i] == ' ')   /* string is "GET URL " +lots of other stuff */
            {
                buffer[i] = 0;
                break;
            }
        }

        if (!strncmp(buffer, "get ", 4))
        {
            // GET
            getFunction(fd, &buffer[5]);

        }
        else if (!strncmp(buffer, "put ", 4))
        {
            // PUT
            putFunction(fd,&buffer[5]);

        }
        else if(!strncmp(buffer, "ls ", 3))
        {
            //LS
            lsFunction(fd,&buffer[4]);
        }

        sleep(1); /* allow socket to drain before signalling the socket is closed */
        close(fd);
#if(OperationMode==0)
        return 0;
#else
        pthread_exit(NULL);
#endif
    }

    /*
    como a funcao ftp recebe dois argumentos, mas so lhe e passado um,
    criei esta funcao para receber esse argumento e so depois chamar a funcao original ftp
    */

    /*void * ftp_thread(void * args)
    {

        THREAD1_ARGS* p = args;

        ftp(p->socketfd, p->hit);   //chama a funcao ftp original

        free(p);

        return 0;

    }
    */

    /* just checks command line arguments, setup a listening socket and block on accept waiting for clients */

#if(OperationMode == 2 )

void initStructConsumidor(STRUCT_CONSUMIDOR *sh, size_t buffSize)
{
    sh->buff = (int*) calloc(buffSize, sizeof(int));    //calloc ( numero de elementos a alocar, tamanho)
    sh->buffSize = buffSize;
}

void *consumidor(void *arg)
{
    int fd, hit=0;
    while(1) //não sabe quantos pedidos vao chegar, entao esta sempre a consumir
        {
                sem_wait(&shared.full);

                pthread_mutex_lock(&shared.mutex);

                fd = shared.buff[shared.firstFUll];

               // shared.buff[shared.firstFUll] = 0;

                shared.firstFUll = (shared.firstFUll+1)%shared.buffSize;
                pthread_mutex_unlock(&shared.mutex);    //libertar o buffer

                printf("\nCondumidores %d\n", fd);

                sem_post(&shared.empty); // incrementa o numero de espaços cheios

                ftp(fd, hit);


                hit++;

        }
    return NULL;
}

#endif




    int main(int argc, char **argv)
    {
        int i, port, pid, listenfd, socketfd, hit, consumidores;
        socklen_t length;
        static struct sockaddr_in cli_addr; /* static = initialised to zeros */
        static struct sockaddr_in serv_addr; /* static = initialised to zeros */


        #if OperationMode == 2

        if (argc < 5 || argc > 5 || !strcmp(argv[1], "-?"))
        {
            printf("\n\nhint: ./tftps Port-Number TOP-Directory COnsumidor Buffsize\n\n"
            "\ttftps is a small and very safe mini ftp server\n"
                   "\tExample: ./tftps 8181 ./fileDir \n\n");
            exit(0);
        }
        consumidores = atoi(argv[3]);
        initStructConsumidor(&shared, (size_t) atoi(argv[4]));
        #else

        if (argc < 3 || argc > 3 || !strcmp(argv[1], "-?"))
        {
            printf("\n\nhint: ./tftps Port-Number Top-Directory\n\n"
                   "\ttftps is a small and very safe mini ftp server\n"
                   "\tExample: ./tftps 8181 ./fileDir \n\n");

            exit(0);
        }
#endif // OperationMode
        if (chdir(argv[2]) == -1)
        {
            printf("ERROR: Can't Change to directory %s\n", argv[2]);
            exit(4);
        }


        printf("LOG tftps starting %s - pid %d\n", argv[1], getpid());

        /* setup the network socket */
        if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            printf("ERROR system call - setup the socket\n");
        port = atoi(argv[1]);
        if (port < 0 || port > 60000)
            printf("ERROR Invalid port number (try 1->60000)\n");


        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        serv_addr.sin_port = htons(port);

        if (bind(listenfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
            printf("ERROR system call - bind error\n");
        if (listen(listenfd, 64) < 0)
            printf("ERROR system call - listen error\n");


    #if OperationMode == 2
        pthread_t idC;
        int index = 0, item;

        sem_t *semFull = sem_open("/semFUll", O_CREAT,0644, 0);
        sem_t *semEmpty = sem_open("/semEMpty", O_CREAT, 0644, shared.buffSize);

        shared.full = *semFull;
        shared.empty = *semEmpty;

        pthread_mutex_init(&shared.mutex, NULL);

        //criar um novo consumidor

        for(index=0; index<consumidores; index++)
        {
            pthread_create(&idC, NULL, consumidor, (void*)&index);
        }

       #endif

        // Main LOOP
        for (hit = 1 ;; hit++)
        {
            length = sizeof(cli_addr);
            /* block waiting for clients */

            socketfd = accept(listenfd, (struct sockaddr *) &cli_addr, &length);
            if (socketfd < 0)
                printf("ERROR system call - accept error\n");


#if (OperationMode == 1)

            //threads

            pthread_t thread1;
            THREAD1_ARGS *thread1_args=(THREAD1_ARGS  *)malloc(sizeof(THREAD1_ARGS));   //struct para passar os argumentos de ftp

            thread1_args->socketfd = socketfd;   //sockedfd
            thread1_args->hit = hit;    //hit

            pthread_create(&thread1, NULL, &ftp, thread1_args);


#elif OperationMode == 2

            item = socketfd; // produtor

            sem_wait(&shared.empty);    //se nao houver espaços vazios, espera

            //pthread_mutex_lock(&shared.mutex);  //se alguma tarefa tentar usar o buffer, espera

            shared.buff[shared.firstFree] =item ;

            printf("Produtores %d", item);

            //pthread_mutex_unlock(&shared.mutex);    //levanta o buffer

            shared.firstFree = (shared.firstFree + 1) % shared.buffSize;

            sem_post(&shared.full); //incremeta o numero de espaços preenchidos

#else
            //fork
            //iniciar o processo paralelização

            if ((pid = fork()) == -1)
            {
                perror(argv[0]);
                exit(1);
            }

            if (pid == 0)  /* Processo filho */
            {
                ftp(socketfd, hit);
                exit(1);
            }
            else   /* Processo pai */
            {
                close(socketfd);

                signal(SIGCHLD, SIG_IGN);
            }
#endif

        }
        return 0;

    }
