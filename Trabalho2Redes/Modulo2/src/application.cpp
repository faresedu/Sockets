#include "../headers/application.h"

bool Application::CheckConnectCommand(string command, string *ip, int *port) {
    string com;
    int argc;
    char **argv;
    *ip = "";
    *port = -1;
    
    getCommandAndArgs(command, &com, &argc, &argv);
    try {
        setIPandPortFromArgs(argc, argv, ip, port);
    }
    catch(...) {
        freeArgvMemory(argc, &argv);
        return false;
    }

    freeArgvMemory(argc, &argv);
    return true;
}

void Application::ReceiveMessages() {
    while(true) {
        string msgReceived = this->socket->Receive();
        if(msgReceived == "") {
            delete this;
        }
        cout << msgReceived << endl;
    }
}

Application::Application() {
    this->socket = new Socket();
}

// Check if client want to connect with server or quit
void Application::ConnectionStage() {
    while(true) {
        cout << "Digite o comando /connect para entrar no chat" << endl;
        cout << "ou o comando /quit para fechar a aplicação" << endl;
        cout << endl;

        string command;
        string ip;
        int port;

        getline(cin, command);
        if(command.at(0) == '/') {
            if(CheckConnectCommand(command, &ip, &port)) {
                if(this->socket->Connect(ip, port, CONNECT_ATTEMPT_TIMEOUT)) {
                    break;
                }
                else {
                    delete this->socket;
                    this->socket = new Socket();
                }
            }
            else if(command == "/quit") {
                delete this;
            }
        }
        cout << endl;
    }
}

// Begin a thread for servidor and listen client connected
void Application::Launch() {
    thread receivingThread([] (Application *app) {app->ReceiveMessages();}, this);
    receivingThread.detach();

    while(true) {
        string fullMsgToSend;
        getline(cin, fullMsgToSend);

        if(fullMsgToSend.at(0) == '/') {
            string command;
            int argc;
            char **argv;
            getCommandAndArgs(fullMsgToSend, &command, &argc, &argv);
            freeArgvMemory(argc, &argv);
            if(command == "/quit") {
                delete this;
            }
            if(command == "/connect") {
                cout << "Você já está conectado ao servidor." << endl;
                continue;
            }
        }

        for(int i = 0 ; MAX_CHARS_IN_MESSAGE*i < (int) fullMsgToSend.size() ; i++) {
            string msgToSend = fullMsgToSend.substr(MAX_CHARS_IN_MESSAGE*i, MAX_CHARS_IN_MESSAGE);
            socket->Send(msgToSend);
        }
    }
}

Application::~Application() {
    cout << "Fechando aplicação..." << endl;
    cout << endl;
    delete (this->socket);
    exit(EXIT_SUCCESS);
}