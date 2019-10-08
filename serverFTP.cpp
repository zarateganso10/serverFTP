/*FTP server*/

#include <iostream>
#include <pthread.h>
#include <fstream>
#include <string>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/*for getting file size using stat()*/
#include<sys/stat.h>

/*for sendfile()*/
#include<sys/sendfile.h>

/*for O_RDONLY*/
#include<fcntl.h>

using namespace std;

#define nummaxconexao 4
#define PORTA 30100
#define RAIZ "/home/zarate//Documents/projetos/clientserverFTP/client-raiz/"
#define RAIZ2 "/home/zarate/Documents/projetos/clientserverFTP/client-raiz/"

typedef struct {
    int Clientsockfd;
    pthread_t idthread;
    bool available = true;
}paramthread_t;

typedef struct {
    string command, tamanho, nm, dados, 
    caminho = "/home/zarate//Documents/projetos/clientserverFTP/client-raiz/", dir = "client-raiz/";
    int size;
}mensagem_t;

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void printVariaveis(char buffer[], mensagem_t *mensagem){
    // cout << "command: " << mensagem->command << endl;
    // cout << "tamanho: " << mensagem->tamanho << endl;
    // cout << "nm: " << mensagem->nm << endl;
    // cout << "dados:" << mensagem->dados << endl;
    cout << "buffer: " << buffer << endl;
}

void limparStrings(mensagem_t *mensagem){
    mensagem->command.erase();
    mensagem->tamanho.erase();
    mensagem->nm.erase();
    mensagem->dados.erase();
}

void construirMensagem(char buffer[], mensagem_t *mensagem){
    // bzero(buffer, 1412);
    int size;
    const char *tempo;
    string temp;
    size = mensagem->command.length();
    for(int i = 0; i < 5; i++){
        if(size <= i){
            mensagem->command += " ";
        }
    }
    int n = mensagem->dados.length();
    mensagem->tamanho = to_string(n);
    size = mensagem->tamanho.length();
    for(int i = 0; i < 4; i++){
        if(size <= i){
            mensagem->tamanho = " " + mensagem->tamanho;
        }
    }
    temp = '1' + mensagem->command + mensagem->tamanho + mensagem->nm + mensagem->dados;
    for(int i = 1; i < 6; i++){
        temp[i] = toupper(temp[i]);
    }
    strcpy(buffer, temp.c_str());
}



void descontruirMensagem(char buffer[], mensagem_t *mensagem ){
    limparStrings(mensagem);
    const char *temp;
    for(int i=1; i < 6; i++){
        if(buffer[i] == ' '){
            break;
        }
        buffer[i] = tolower(buffer[i]);
        mensagem->command += buffer[i];
    }
    for(int i=6; i < 10; i++){
        if(buffer[i] == ' '){
            continue;
        }else{
            mensagem->tamanho += buffer[i];
        }
    }
    int size;
    temp = mensagem->tamanho.c_str();
    size = atoi(temp);
    for(int i=10; i < 12; i++){
        mensagem->nm += buffer[i];
    }
    for(int i= 12; i < size+12; i++){
        mensagem->dados += buffer[i];
    }
}

void enviarMensagem(int socket, char buffer[], mensagem_t *mensagem){
    construirMensagem(buffer, mensagem);
    int n;
    n = write(socket,buffer, 1412);
    if (n < 0) error("ERROR writing to socket");
    // cout << "enviando mensagem" << endl;
    // printVariaveis(buffer, mensagem);
}

void receberMensagem(int socket, char buffer[], mensagem_t *mensagem){
    int n;
    n = read(socket,buffer, 1412);
    if (n < 0) error("ERROR reading from socket");
    descontruirMensagem(buffer, mensagem);
    // cout << "recebendo mensagem" << endl;
    // printVariaveis(buffer, mensagem);
}

void functionArquivo(mensagem_t *mensagem){
    struct stat obj;
    FILE *file;
    file = fopen("temps.txt", "rb");
    stat("temps.txt",&obj);
    mensagem->size = obj.st_size;
    int size = mensagem->size;
    char temp[size], temp2[5], temp3[size];
    while(!feof(file)){
        fgets(temp, size,file);
        if (strcmp(temp, "temps.txt\n") != 0 && strcmp(temp, "\n") != 0 && strcmp(temp, temp3) != 0 && strcmp(temp, "temps.txt") != 0 ){
            mensagem->dados = mensagem->dados + temp;
            strcpy(temp3, temp);
        }
    }
    fclose(file);
    remove("temps.txt");
    sprintf(temp2, "%i", size);
    mensagem->tamanho = temp2;
}

void construirMensagemGET(char buffer[],mensagem_t *mensagem){
    int size;
    const char *tempo;
    string temp;
    size = mensagem->command.length();
    for(int i = 0; i < 5; i++){
        if(size <= i){
            mensagem->command += " ";
        }
    }
    int n = mensagem->dados.length();
    mensagem->tamanho = to_string(n);
    size = mensagem->tamanho.length();
    for(int i = 0; i < 4; i++){
        if(size <= i){
            mensagem->tamanho = " " + mensagem->tamanho;
        }
    }
    temp = '1' + mensagem->command + mensagem->tamanho + mensagem->nm;
    for(int i = 1; i < 6; i++){
        temp[i] = toupper(temp[i]);
    }
    for(int i = 0; i < 12;i++){
        buffer[i] = temp[i];
    }
}

void enviarMensagemGET(int socket, char buffer[], mensagem_t *mensagem){
    construirMensagemGET(buffer, mensagem);
    int n;
    n = write(socket,buffer, 1412);
    if (n < 0) error("ERROR writing to socket");
    // cout << "enviando mensagem" << endl;
    // printVariaveis(buffer, mensagem);
}

void functionGET(int socket, char buffer[], mensagem_t *mensagem){
    struct stat obj;
    FILE *file;
    char *ponteiro;
    char c;
    char arquivo[mensagem->dados.length()];
    strcpy(arquivo, mensagem->dados.c_str());
    file = fopen(arquivo, "rb");
    if (file == NULL){
        mensagem->command = "error";
        enviarMensagem(socket, buffer, mensagem);
    }else{
        fclose(file);
        file = fopen(arquivo, "rb");
        enviarMensagem(socket, buffer, mensagem);
        limparStrings(mensagem);
        int j = 0;
        while(fread(&c, sizeof(char), 1, file)){
            if(j == 1400){
                j = 0;
                mensagem->nm = "NM";
                mensagem->command = "get";
                enviarMensagemGET(socket, buffer, mensagem);
                limparStrings(mensagem);
                bzero(buffer,1412);
            }
            mensagem->dados += c;
            buffer[j+12] = c;
            j++;
        }
        fclose(file);
        mensagem->command = "get";
        mensagem->nm = "FM";
        enviarMensagemGET(socket, buffer, mensagem);
        // cout << "dadosarquivo: " << dadosarquivo << endl;
    }
}

void comando(int c_socket, char buffer[], mensagem_t *mensagem){
    int size;
    FILE *file;
    struct stat obj;

    if(mensagem->command == "quit"){
        close(c_socket);
        exit(1);
    }else if(mensagem->command == "pwd"){
        chdir(mensagem->caminho.c_str());
        system("pwd > temps.txt");
        functionArquivo(mensagem);
        enviarMensagem(c_socket, buffer, mensagem);
    }else if(mensagem->command == "cd"){
        cout << "caminho: " << mensagem->caminho << endl;
        cout << "mensagem: dados: " << mensagem->dados << endl;
        if((mensagem->dados == ".." || mensagem->dados == "../") && (mensagem->caminho == RAIZ || mensagem->caminho == RAIZ2)){
            mensagem->command = "erro";
            enviarMensagem(c_socket, buffer, mensagem);
        }else{
            int n;
            string temp3 = mensagem->caminho + mensagem->dados;
            n = chdir(temp3.c_str());
            if(n == 0){
                system("pwd > temps.txt");
                file = fopen("temps.txt", "r");
                stat("temps.txt",&obj);
                size = obj.st_size;
                char temp[size + 1], temp2[5];
                fgets(temp, size,file);
                fclose(file);
                remove("temps.txt");
                mensagem->caminho = temp;
                mensagem->caminho += "/";
                enviarMensagem(c_socket, buffer, mensagem);
            }else{
                mensagem->command = "error";
                enviarMensagem(c_socket, buffer, mensagem);
            }
        }
    }else if(mensagem->command == "ls"){
        chdir(mensagem->caminho.c_str());
        system("ls > temps.txt");
        functionArquivo(mensagem);
        enviarMensagem(c_socket, buffer, mensagem);
    }else if(mensagem->command == "mkdir"){
        chdir(mensagem->caminho.c_str());
        string temp = mensagem->command + " " +mensagem->dados;
        int n = system(temp.c_str());
        if(n == 0){
            mensagem->dados = "Diretorio criado";
            enviarMensagem(c_socket, buffer, mensagem);

        }
    }else if(mensagem->command == "get"){
        chdir(mensagem->caminho.c_str());
        functionGET(c_socket, buffer, mensagem);
    }else if(mensagem->command == "put"){
        chdir(mensagem->caminho.c_str());
        FILE *file;
        string arquivo = mensagem->dados;
        string dadosarquivo;
        file = fopen(arquivo.c_str(), "ab");
        if(file == NULL){
            mensagem->command = "error";
            enviarMensagem(c_socket, buffer, mensagem);
        }else{
            enviarMensagem(c_socket, buffer, mensagem);
            fclose(file);
            while(true){
                receberMensagem(c_socket, buffer, mensagem);
                dadosarquivo += mensagem->dados;
                if(mensagem->nm == "FM"){
                    break;
                }
            }
            file = fopen(arquivo.c_str(), "ab");
            fwrite(dadosarquivo.c_str(), sizeof(char), dadosarquivo.length(), file);
            fclose(file);
        }
    }else{
        mensagem->command = "error";
        enviarMensagem(c_socket, buffer, mensagem);
    }
}

void *gerenciamentoConexao(void * value){
    char buffer[1412];
    mensagem_t mensagem;
    paramthread_t *param = (paramthread_t *)value;
    int n;
	cout << "Conexao aceita com ClienteSocket: " << param->Clientsockfd << endl;
    while(true){
        bzero(buffer,1412);
        // Le a mensagem do cliente
        receberMensagem(param->Clientsockfd, buffer, &mensagem);
        // cout << "command: " << mensagem.command << endl;
        if(mensagem.command == "close"){
            cout << "Clientsocket desconectado: " << param->Clientsockfd << endl;
            close(param->Clientsockfd);
            break;
        }
        comando(param->Clientsockfd, buffer, &mensagem);
    }
    close(param->Clientsockfd);
    param->available = true;
    return 0;

}

int threadsleep(paramthread_t* param, int TAM){
    for(int i = 0; i < TAM; i++){
        if(param[i].available == true){
            return i;
            break;
        }
    }
    return -1;
}

int main()
{
    /* Variables */
    int Listensockfd, Clientsockfd;
    socklen_t clilen;
    int indice;
    paramthread_t parametro[nummaxconexao];
    struct sockaddr_in serv_addr, cli_addr;
    

    /*Create a socket */

    Listensockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (Listensockfd < 0) error("ERROR opening socket");

    bzero((char *) &serv_addr, sizeof(serv_addr));
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORTA); 

    /* Bind */
    if (bind(Listensockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    
    /* Listen */
    listen(Listensockfd,nummaxconexao);
    clilen = sizeof(cli_addr);
    cout << "Aguardando conexao..." << endl;

    chdir(RAIZ);

    while(true){
        /* Accept */
        Clientsockfd = accept(Listensockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (Clientsockfd < 0) error("ERROR on accept");
        // configurando thread
        indice = threadsleep(parametro, nummaxconexao);
        if(indice == -1){
            cout << "servidor cheio " << endl;
        }else{
            parametro[indice].available = false;
            parametro[indice].Clientsockfd = Clientsockfd;
            pthread_create(&parametro[indice].idthread, NULL, gerenciamentoConexao, &parametro[indice]);
        }
    }

    close(Clientsockfd);
    close(Listensockfd);
    return 0; 
   
}