//
//  client.c
//  Adapted by Christophe Soares & Pedro Sobral on 15/16
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define BUFSIZE 8096

int pexit(char * msg)
{
    perror(msg);
    exit(1);
}

    //igual ao GET
void newget(char* fileName, char buffer[BUFSIZE], int sockfd, int filedesc, int i)
{

    sprintf(buffer, "get /%s\n", fileName);

    // Now the sockfd can be used to communicate to the server the GET request
    write(sockfd, buffer, strlen(buffer));

    filedesc = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, 0777);  //abrir o ficheiro

    while ((i = read(sockfd, buffer, BUFSIZE)) > 0) //copiar conteudo
    {
        write(filedesc, buffer, i);
    }

}

int main(int argc, char *argv[])
{
    int i, sockfd, filedesc;
    long long ret = 0, offset = 0;
    char buffer[BUFSIZE];
    static struct sockaddr_in serv_addr;

    struct stat stat_buf; // argument to fstat

    char fileName[50];

    if (argc != 5)
    {
        printf("Usage: ./client <SERVER IP ADDRESS> <LISTENING PORT>\n");
        printf(
            "Example: ./client 127.0.0.1 8141 get remoteFileName\n./client 127.0.0.1 8141 [put/get] localFileName\n\n");
        printf("./client 127.0.0.1 8141 ls /path");
        exit(1);
    }

    printf(
        "client trying to connect to IP = %s PORT = %s method= %s for FILE= %s\n",
        argv[1], argv[2], argv[3], argv[4]);

    strcpy(fileName, argv[4]);

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        pexit("socket() failed");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    // Connect tot he socket offered by the web server
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        pexit("connect() failed");




    if (!strcmp(argv[3], "get"))
    {

        sprintf(buffer, "get /%s", fileName);

        // Now the sockfd can be used to communicate to the server the GET request
        write(sockfd, buffer, strlen(buffer));

        filedesc = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, 0777);

        while ((i = read(sockfd, buffer, BUFSIZE)) > 0)
            write(filedesc, buffer, i);

    }
    else if (!strcmp(argv[3], "put"))
    {

        sprintf(buffer, "put /%s", fileName);
        printf("-> put /%s\n", fileName);

        write(sockfd, buffer, strlen(buffer));

        ret = read(sockfd, buffer, BUFSIZE); 	// read Web request in one go
        buffer[ret] = 0; 						// put a null at the end

        if (ret > 0)
        {
            printf("<- %s\n", buffer);
            if (!strcmp(buffer, "OK"))   // check if it is OK on the ftp server side
            {

                // open the file to be sent
                filedesc = open(fileName, O_RDWR);

                // get the size of the file to be sent
                fstat(filedesc, &stat_buf);

                // Read data from file and send it
                ret = 0;
                while (1)
                {
                    unsigned char buff[BUFSIZE] = { 0 };
                    int nread = read(filedesc, buff, BUFSIZE);
                    ret += nread;
                    printf("\nBytes read %d \n", nread);

                    // if read was success, send data.
                    if (nread > 0)
                    {
                        printf("Sending \n");
                        write(sockfd, buff, nread);
                    }

                    // either there was error, or we reached end of file.

                    if (nread < BUFSIZE)
                    {
                        if (ret == stat_buf.st_size)
                            printf("End of file\n");
                        else
                            printf("Error reading\n");
                        break;
                    }

                }

                if (ret == -1)
                {
                    fprintf(stderr, "error sending the file\n");
                    exit(1);
                }
                if (ret != stat_buf.st_size)
                {
                    fprintf(stderr,
                            "incomplete transfer when sending: %lld of %d bytes\n",
                            ret, (int) stat_buf.st_size);
                    exit(1);
                }
            }
            else
            {
                printf("ERROR on the server");
            }

            // close descriptor for file that was sent
            close(filedesc);

            // close socket descriptor
            close(sockfd);

        }

    }
    else if (!strcmp(argv[3], "ls"))
    {

        int valread;

        sprintf(buffer, "ls /%s", fileName);
        write(sockfd, buffer, strlen(buffer));
        valread=read(sockfd, buffer, BUFSIZE);
        buffer[valread]='\0';
        puts(buffer);
        //printf("o LS acabou");


    }
    else if(!strcmp(argv[3], "mget"))
    {

        long long int startTime = 0;
        long long int finishTime = 0;
        long long int difTime = 0;

    //gettimeuseconds(&startTime);    //tempo no inicio

        //LS

        int valread;

        sprintf(buffer, "ls /%s", fileName);
        write(sockfd, buffer, strlen(buffer));
        valread=read(sockfd, buffer, BUFSIZE);
        printf("\n%s", buffer);

        //j = numero total de ficheiros ---- k= cada ficheiro
        int j=0, k=0;

        //criar um array list
        char ** array=(char**)malloc(sizeof(char*)*50);  //50 espaços para ficheiros
        char* aux=NULL;
        *(array+j)=(char*)malloc(sizeof(char*)*50);    //ficheiros até 50 caracteres
        aux=strtok(buffer, "\n");   //separar por /n
        strcpy(*(array+j), aux);
        j++;    //incrementar n de ficheiros


        //enquanto nao estiver vazio, faz ciclo
        while(aux!=NULL)
        {
            aux=strtok(NULL, "\n");
            if(aux!=NULL)   //enquanto a variavel naõ estiver a null
            {
                *(array+j)=(char*)malloc(sizeof(char*)*50);
                strcpy(*(array+j), aux);
                j++;
            }
        }
        //processo
        pid_t pid;
        int sockfd=0;
        int pidj[j];
        printf("\n numero de ficheiros total %d\n", j);
        for(i=0; k<j; k++)
        {
            printf("ficheiro-> %s\n", *(array+k));

            pid=fork();

            if(pid==0)
            {
                //ligações
                if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
                    pexit("socket() failed");

                serv_addr.sin_family = AF_INET;
                serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
                serv_addr.sin_port = htons(atoi(argv[2]));

                // Connect tot he socket offered by the web server
                if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
                    pexit("connect() failed");


            //chamar a nova função de get
            newget(*(array+k), buffer, sockfd, filedesc, i);
            exit(0);
            }
            //gettimeuseconds(&finishTime); //tempo final
            difTime = finishTime - startTime;
            printf("\n tempo: %lld\n", difTime);    //imprimir o tempo final
        }

        //PAI
        //O pai espera que o filho acabe
        for(i=0; i<j; i++)
        {
            wait(NULL);
        }
    }
    else
        // implement new methods
        printf("unsuported method\n");

    return 1;
}
