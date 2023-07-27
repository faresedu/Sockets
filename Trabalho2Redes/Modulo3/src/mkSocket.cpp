#include "../headers/mkSocket.h"

Socket::Socket(int id, sockaddr_in addr) {
    this->id = id;
    this->addr = addr;
}

void Socket::Init(in_addr_t ip, in_port_t port) {
    this->id = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    checkOperation(this->id, "Erro criando o socket!");

    int opt = 1;
    checkOperation(setsockopt(this->id, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)), "Erro configurando o socket!");
    
    sockaddr_in myAddr = CreateSocketAddr(ip, port);    
    checkOperation(bind(this->id, (sockaddr *) &(myAddr), sizeof(myAddr)), "Erro fazendo o bind!");

    socklen_t addrSize = sizeof(this->addr);
    checkOperation(getsockname(this->id, (sockaddr *) &(this->addr), &addrSize), "Erro obtendo endereço do socket!");
}

sockaddr_in Socket::CreateSocketAddr(in_addr_t ip, in_port_t port) {
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = ip;
    address.sin_port = port;
    return address;
}

Socket::Socket(string ip, int port) {
    Init(inet_addr(ip.c_str()), htons(port));
}

Socket::Socket(string ip) {
    Init(inet_addr(ip.c_str()), 0);
}

Socket::Socket(int port) {
    Init(INADDR_ANY, htons(port));
}

Socket::Socket() {
    Init(INADDR_ANY, 0);
}

bool Socket::Connect(string ip, int port, int timeout) {
    sockaddr_in addrToConnect = CreateSocketAddr(inet_addr(ip.c_str()), htons(port));

    int flags;
    flags = fcntl(this->id, F_GETFL);
    checkOperation(flags, "Erro lendo flags do socket!");
    checkOperation(fcntl(this->id, F_SETFL, flags | O_NONBLOCK), "Erro modificando a flag NONBLOCK!");  // Setando socket como NonBlocking

    bool connected = false;
    if(connect(this->id, (sockaddr *) &addrToConnect, sizeof(addrToConnect)) == -1) {
        if(errno == EINPROGRESS) {
            fd_set sockWritingSet;
            FD_ZERO(&sockWritingSet);
            FD_SET(this->id, &sockWritingSet);

            timeval tv;
            tv.tv_sec = (time_t) timeout;
            tv.tv_usec = (time_t) 0;

            int ret = select(this->id + 1, NULL, &sockWritingSet, NULL, &tv);
            checkOperation(ret, "Erro fazendo o connect_1!");
            if(ret == 0) {
                cout << "Tentativa de connect deu timeout!" << endl;
            }
            else {
                int opt;
                socklen_t optSize = sizeof(opt);
                checkOperation(getsockopt(this->id, SOL_SOCKET, SO_ERROR, &opt, &optSize), "Erro checando erro de conexão!");
                if(opt) {
                    cout << "Não foi possível se conectar!" << endl;
                }
                else {
                    cout << "Conexão estabelecida" << endl;
                    connected = true;
                }
            }
        }
        else {
            cerr << "Erro fazendo o connect_2!" << endl;
            exit(EXIT_FAILURE);
        }
    }
    else {
        cerr << "Erro fazendo o connect_3!" << endl;
        exit(EXIT_FAILURE);
    }

    checkOperation(fcntl(this->id, F_SETFL, flags & (~O_NONBLOCK)), "Erro modificando a flag NONBLOCK!");  // Setando socket como Blocking

    return connected;
}

void Socket::Listen(int queueSize) {
    checkOperation(listen(this->id, queueSize), "Erro habilitando o listen!");
}

Socket * Socket::Accept() {
    sockaddr_in clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);
    int clientSockID = accept(this->id, (sockaddr *) &clientAddr, &clientAddrSize);
    checkOperation(clientSockID, "Erro aceitando conexão!");

    return new Socket(clientSockID, clientAddr);
}

string Socket::Receive() {
    int bytesReceivedTotal = 0;
    int bytesReceived = 0; 
    
    char *buffer = NULL;
    do {
        buffer = (char *) realloc(buffer, (bytesReceivedTotal + 1) * sizeof(char));
        bytesReceived = recv(this->id, &buffer[bytesReceivedTotal], 1, 0);
        checkOperation(bytesReceived, "Erro recebendo mensagem!");
        if(bytesReceived == 0) {
            return "";
        }
        bytesReceivedTotal += bytesReceived;
    } while(buffer[bytesReceivedTotal-1] != '\0');

    string msgReceived = string(buffer);
    free(buffer);
    
    return msgReceived;
}

void Socket::Send(string msgToSend) {
    const char *buffer = msgToSend.c_str();
    const int bytesToSend = msgToSend.size() + 1;

    int bytesSentTotal = 0;
    int bytesSent = 0;
    do {
        bytesSent = send(this->id, &buffer[bytesSentTotal], bytesToSend - bytesSentTotal, 0);
        checkOperation(bytesSent, "Erro enviando mensagem");
        bytesSentTotal += bytesSent;
    } while(bytesSentTotal < bytesToSend);
}

sockaddr_in Socket::GetAddr() {
    return this->addr;
}

string Socket::GetIP() {
    return string(inet_ntoa(this->addr.sin_addr));
}

int Socket::GetPort() {
    return ntohs((uint16_t) this->addr.sin_port);
}

Socket::~Socket() {
    close(this->id);
}