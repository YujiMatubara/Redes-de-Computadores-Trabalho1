#include "socketServer.hpp"


void* Server::connectionHandler(int socket) {
    std::cout << "Here?2\n";
    int readSize;
    std::string message;
    char msgClient[MSG_SIZE];
    
    message = "Bem-vindo ao jogo!\n";
    write(socket, message.c_str(), message.length());

    message = "Escreva algo só para testar a conexão...\n";
    write(socket, message.c_str(), message.length());
     
    
    while((readSize = recv(socket, msgClient, MSG_SIZE , 0)) > 0 ) {
        mtx.lock();
        msgClient[readSize] = '\0'; 
        write(socket, msgClient, strlen(msgClient));
		memset(msgClient, 0, MSG_SIZE);
        mtx.unlock();
    }
     
    if(readSize == 0) {
        printf("Cliente desconectado (socket nb %d\n", socket);
        close(socket);
        curClientsNo--;
        printf("Número de clientes = %d\n", curClientsNo);
        fflush(stdout);
        //matar thread
    }
    else if(readSize == -1) {
        close(socket);
        printf("Erro no recv()");
        curClientsNo--;
        //matar thread
    }
         
    return 0;
}


void Server::sendMsg(player * currPlayer, char * msg) {
    int sent;
    
    msg[MSG_SIZE] = '\0'; 
    sent = send(currPlayer->socket, msg, strlen(msg), 0);

    if (sent == -1)
        std::cout << "Erro ao enviar mensagem\n";
    else
        std::cout << "Mensagem enviada com sucesso\n";

    return;
}


void Server::listener(player * currPlayer){
    int recieved;
    char answer[MSG_SIZE];
    do {
        recieved = recv(currPlayer->socket, answer, MSG_SIZE, 0); 
        if (recieved == -1)
            std::cout << "Erro ao receber mensagem do socket \n" << currPlayer->socket << std::endl;
        else {
            answer[recieved] = '\0';
            printf("Mensagem do cliente recebida: %s\n", answer);
        }
    } while(strcmp(answer, "exit") != 0); 

    return;
}

int Server::setServerSocket() {
    socketServer = socket(AF_INET, SOCK_STREAM, 0);

    if(socketServer == -1) {
        std::cout << "Erro ao criar o socket do servidor\n";
        return 1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(serverPort);
    addr.sin_addr.s_addr = INADDR_ANY;
    memset(&addr.sin_zero, 0, sizeof(addr.sin_zero));

    if(bind(socketServer, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        std::cout << "Erro na funcao bind()\n";
        return 1;
    }

    if(listen(socketServer, playersNo) < 0) {
        std::cout << "Erro na funcao listen()\n";
        return 1;
    }
    return 0;
}

int Server::awaitPlayersConnection() {
    while (curClientsNo < playersNo){
        printf("Aguardando jogador %d...\n", curClientsNo);
        activePlayers[curClientsNo].socket = accept(socketServer, (struct sockaddr *) &serverStorage, &addrSize); 
        
        if(activePlayers[curClientsNo].socket < -1) {
            std::cout << "Erro na funcao accept()\n";
            return 1;
        }
        std::cout << "Cliente conectado (socket fd = " << activePlayers[curClientsNo].socket << ")!\n";
        
        std::cout << "socket: " << activePlayers.size() << ";" << activePlayers[curClientsNo].socket << std::endl;
        std::cout << "dest0" << (void*) &(activePlayers[curClientsNo].socket) << std::endl;
        
        threads.push_back(std::thread(&Server::connectionHandler,this,activePlayers[curClientsNo].socket));

        printf("Número de clientes = %d\n", curClientsNo);
        curClientsNo++;
    }

    while (curClientsNo > 0) { continue; } 

    return 0;
}

void Server::closeServer(player * activePlayers) {
    for (int i = 0; i < playersNo; i++)
        close(activePlayers[i].socket);
    close(socketServer);

    return;
}

void Server::initializeServer(int playersNo,int serverPort){
    this->playersNo = playersNo;
    this->serverPort = serverPort;
    
    activePlayers.resize(playersNo);
    for(int i=0;i<playersNo;i++) activePlayers[i].socket = 0;
    
    curClientsNo = 0;

    addrSize = sizeof serverStorage;
}

Server::Server(int playersNo,int serverPort){
    printf("Testando socket do servidor...\n");
    initializeServer(playersNo,serverPort);
    setServerSocket();
    awaitPlayersConnection();
    close(socketServer);
}



//g++ -Wall -std=c++11 -static -pthread -Wl,--whole-archive -lpthread -Wl,--no-whole-archive -o testServerSocket socketServer2.cpp
int main() {
    Server s(4,18120);    

    return 0;
}