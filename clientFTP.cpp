#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <ctype.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

/*for getting file size using stat()*/
#include<sys/stat.h>

using namespace std;

#define PORTA 30100
#define RAIZ "/home/zarate//Documents/projetos/clientserverFTP/client/"
// #define IP "192.168.0.21"
// #define IP "192.168.0.11"
// #define IP "127.0.0.1"

char buffer[1412];
int size;
string command, tamanho, nm, dados;
int c_socket;
struct sockaddr_in serv_addr;
struct hostent *server;

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

void printVariaveis(){
    
    // cout << "command: " << command << endl;
    // cout << "tamanho: " << tamanho << endl;
    // cout << "nm: " << nm << endl;
    // cout << "dados:" << dados << endl;
    cout << "buffer: " << buffer << endl;
}

void limparStrings(){
    command.erase();
    tamanho.erase();
    nm.erase();
    dados.erase();
    // dadosarquivo.erase();
}

// void functionTamanho(){
//     int size;
//     size = dados.length();
//     char temp2[size];
//     sprintf(temp2, "%i", size);
//     tamanho = temp2;
// }

void construirMensagem(){
    bzero(buffer, 1412);
    // functionTamanho();
    int size;
    const char *tempo;
    string temp;
    size = command.length();
    for(int i = 0; i < 5; i++){
        if(size <= i){
            command += " ";
        }
    }
    int n = dados.length();
    tamanho = to_string(n);
    size = tamanho.length();
    for(int i = 0; i < 4; i++){
        if(size <= i){
            tamanho = " " + tamanho;
        }
    }
    temp = '1' + command + tamanho + nm + dados;
    for(int i = 1; i < 6; i++){
        temp[i] = toupper(temp[i]);
    }
    strcpy(buffer, temp.c_str());
}

void descontruirMensagem(){
    limparStrings();
    const char *temp;
    for(int i=1; i < 6; i++){
        if(buffer[i] == ' '){
            break;
        }
        buffer[i] = tolower(buffer[i]);
        command += buffer[i];
    }
    for(int i=6; i < 10; i++){
        if(buffer[i] == ' '){
            continue;
        }else{
            tamanho += buffer[i];
        }
    }
    for(int i=10; i < 12; i++){
        nm += buffer[i];
    }
    temp = tamanho.c_str();
    int size = atoi(tamanho.c_str());
    for(int i= 12; i < size + 12; i++){
        dados += buffer[i];
    }
}

void receberMensagem(){
    int n;
    n = read(c_socket,buffer,1412);
    if (n < 0) 
        error("ERROR reading from c_socket");
    descontruirMensagem();
    // cout << "recebendo mensagem" << endl;
    // printVariaveis();
}

void enviarMensagem(){
    int n;
    construirMensagem();
    n = write(c_socket,buffer,strlen(buffer));
    if (n < 0) error("ERROR writing to c_socket");
    // cout << "enviando mensagem" << endl;
    // printVariaveis();
}

void construirMensagemGET(){
    int size;
    const char *tempo;
    string temp;
    size = command.length();
    for(int i = 0; i < 5; i++){
        if(size <= i){
            command += " ";
        }
    }
    int n = dados.length();
    tamanho = to_string(n);
    size = tamanho.length();
    for(int i = 0; i < 4; i++){
        if(size <= i){
            tamanho = " " + tamanho;
        }
    }
    temp = '1' + command + tamanho + nm;
    for(int i = 1; i < 6; i++){
        temp[i] = toupper(temp[i]);
    }
    for(int i = 0; i < 12;i++){
        buffer[i] = temp[i];
    }
}

void enviarMensagemGET(){
    construirMensagemGET();
    int n;
    n = write(c_socket,buffer, 1412);
    if (n < 0) error("ERROR writing to socket");
    // cout << "enviando mensagem" << endl;
    // printVariaveis();
}

bool open(string IP){
    char serverip[IP.length()];
    strcpy(serverip, IP.c_str());
    // Criando c_socket
    c_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (c_socket < 0)
        error("ERROR opening c_socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(serverip);
    serv_addr.sin_port = htons(PORTA);
    // conectando com o servidor
    int n = connect(c_socket,(struct sockaddr *) &serv_addr,sizeof(serv_addr));
    if(n < 0){
        // error("ERROR connecting");
        return false;
    }else{
        return true;
    }
}

void comando(){
    int size;
    if(command == "close"){
        nm = "FM";
        enviarMensagem();
        close(c_socket);
    }
    if(command == "quit") {
        cout << "Fechando programa" << endl;
        close(c_socket);
        exit(1);
    }else if(command == "pwd"){
        nm = "FM";
        enviarMensagem();
        receberMensagem();
        cout << dados << endl;
    }else if(command == "cd"){
        nm = "FM";
        enviarMensagem();
        receberMensagem();
        if(command == "erro"){
            cout << "ERRO ao trocar de diretorio" << endl;
        }
    }else if(command == "ls"){
        nm = "FM";
        enviarMensagem();
        receberMensagem();
        cout << dados << endl;
    }else if(command == "mkdir"){
        nm = "FM";
        enviarMensagem();
    }else if(command == "get"){
        string dadosarquivo;
        FILE *file;
        nm = "FM";
        string arquivo = dados;
        enviarMensagem();
        receberMensagem();
        if(command == "error"){
            cout << "nao existe o arquivo";
        }else{
            while(true){
                receberMensagem();
                dadosarquivo += dados;
                if(nm == "FM"){
                    break;
                }
            }
            // cout << "dadosarquivo: " << dadosarquivo << endl;
            file = fopen(arquivo.c_str(), "ab");
            fwrite(dadosarquivo.c_str(), sizeof(char), dadosarquivo.length(), file);
            fclose(file);
        }
        
    }else if(command == "put"){
        FILE *file;
        char c;
        nm = "FM";
        string arquivo = dados;
        file = fopen(arquivo.c_str(), "rb");
        if(file == NULL){
            cout << "Arquivo nao existe" << endl;
        }else{
            fclose(file);
            enviarMensagem();
            receberMensagem();
            if(command == "error"){
                cout << "Nao pode criar o arquivo no servidor" << endl;
            }else{
                file = fopen(arquivo.c_str(), "rb");
                limparStrings();
                string dadosarquivo;
                int j = 0;
                while(fread(&c, sizeof(char), 1, file)){
                    if(j == 1400){
                        j = 0;
                        nm = "NM";
                        command = "put";
                        enviarMensagemGET();
                        limparStrings();
                        bzero(buffer, 1412);
                    }
                    dados += c;
                    buffer[j + 12] = c;
                    j++;
                }
                fclose(file);
                command = "put";
                nm = "FM";
                enviarMensagemGET();
            }
        }
    }else if(command == "open"){
        if(open(dados) == true){
            cout << "conexao aceita" << endl;
        }else{
            cout << "nao conectado" << endl;
        }
    }
}

int main()
{
    chdir(RAIZ);
    while(true){
        limparStrings();
        // mandando uma mensagem pro servidor
        cout << "$ ";
        cin >> command;
        if(command == "cd" || command == "mkdir" || command == "get" || command == "put" || command == "open"){
            cin >> dados;
        }
        comando();
    }
    close(c_socket);
    return 0;
}
