#include <iostream>
#include <string>
#include <thread>
#include <arpa/inet.h>
#include <locale.h>
#include <signal.h>
#include "../headers/util.h"
#include "../headers/mkSocket.h"

#define MAX_CHARS_IN_MESSAGE 4096
#define CONNECT_ATTEMPT_TIMEOUT 10
#define IP_DEFAULT "127.0.0.1"
#define PORT_DEFAULT 3000

using namespace std;


bool leave = false;

void catchInterruption(int a);
void receivingFromFriend(Socket *socket);
void sendingToFriend(Socket *socket);
void waitingFriendToConnect(Socket *listenerSocket, string friendIP);
void createChatThreads(Socket *socket);


int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "pt_BR.UTF-8");
    signal(SIGINT, catchInterruption);  // Listen if someone exits

    string targetIP = IP_DEFAULT;
    int targetPort = PORT_DEFAULT;
    setIPandPortFromArgs(argc - 1, argv + 1, &targetIP, &targetPort);
    
    Socket *socket = new Socket();
    bool connected = socket->Connect(targetIP, targetPort, CONNECT_ATTEMPT_TIMEOUT);

    // Verify if the user gonna be a "server" to a friend(client) connect with
    if(connected) {
        cout << "Você se conectou ao seu amigo!" << endl;
        cout << endl;
        createChatThreads(socket);
    }
    else {
        cout << "Seu amigo não está disponível" << endl;
        delete socket;
        socket = new Socket(PORT_DEFAULT);
        socket->Listen(SOMAXCONN);
        thread listeningThread(waitingFriendToConnect, socket, targetIP);
        listeningThread.detach();
        cout << "Esperando conexão na Porta: " << socket->GetPort() << endl;
        cout << endl;
    }

    while(!leave) { };
    delete socket;

    return EXIT_SUCCESS;
}

void catchInterruption(int a) {
    leave = true;
}

// Listen and read messages received
void receivingFromFriend(Socket *socket) {
    while(true) {
        string msgReceived = socket->Receive();
        if(msgReceived == "") {
            leave = true;
            return;
        }
        cout << "Friend : " << msgReceived << endl;
    }
}

// Send messagens catch in terminal
void sendingToFriend(Socket *socket) {    
    while(true) {
        string fullMsgToSend;
        getline(cin, fullMsgToSend);
        for(int i = 0 ; MAX_CHARS_IN_MESSAGE*i < (int) fullMsgToSend.size() ; i++) {
            string msgToSend = fullMsgToSend.substr(MAX_CHARS_IN_MESSAGE*i, MAX_CHARS_IN_MESSAGE);
            cout << "You : " << msgToSend << endl;
            socket->Send(msgToSend);
        }
    }
}

// The fist client waiting the user who he want to connect to
void waitingFriendToConnect(Socket *listenerSocket, string friendIP) {
    while(true) {
        Socket *client = listenerSocket->Accept();
        if(client->GetAddr().sin_addr.s_addr == inet_addr(friendIP.c_str())) {
            cout << "Amigo se conectou!" << endl;
            cout << endl;
            createChatThreads(client);
            delete listenerSocket;
            return;
        }
        else {
            delete client;
        }
    }
}

void createChatThreads(Socket *socket) {
    thread receiveThread(receivingFromFriend, socket);
    thread sendThread(sendingToFriend, socket);
    receiveThread.detach();
    sendThread.detach();
}