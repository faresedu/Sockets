#include "../headers/server.h"

// Add/Create a new chanel in the server chanel vector
Channel * Server::CreateChannel(string channelName) {
    if(FindChannelByName(channelName) != NULL) {
        return NULL;
    }
    
    Channel *channel = NULL;
    try{ 
        channel = new Channel(channelName);
        m_cha.lock();
        this->channelsOpen.push_back(channel);
        m_cha.unlock();
        return channel;
    }
    catch(...) {
        delete channel;
        throw invalid_argument("Nome de canal inválido");
    }
}

// Delete chanel known 
void Server::DeleteChannel(Channel *channel) {
    m_cha.lock();
    for(vector<Channel *>::iterator cha = this->channelsOpen.begin() ; cha != this->channelsOpen.end() ; cha++) {
        if((*cha) == channel) {
            delete (*cha);
            this->channelsOpen.erase(cha);
            break;
        }
    }
    m_cha.unlock();
}

// Search from known channels
Channel * Server::FindChannelByName(string channelName) {
    Channel *channelFound = NULL;
    for(vector<Channel *>::iterator cha = this->channelsOpen.begin() ; cha != this->channelsOpen.end() ; cha++) {
        if((*cha)->GetName() == channelName) {
            channelFound = (*cha);
            break;
        }
    }
    return channelFound;
}

// Turn to string avaiable channels
string Server::ChannelsAvailableMessage() {
    if(this->channelsOpen.empty()) {
        return "Não há canais abertos no momento\n";
    }

    string message = "";
    message += "Os canais existentes no momento são:\n";
    for(vector<Channel *>::iterator cha = this->channelsOpen.begin() ; cha != this->channelsOpen.end() ; cha++) {
        message += (*cha)->GetName();
        if((*cha)->IsPrivate()) {
            message += " (privado)";
        }
        message += " ==> " + to_string((*cha)->NumberOfClientsConnected()) + " online\n";
    }
    message += "\n";
    return message;
}

// Add Client to a server vector of clients
Client * Server::AddClientToServer(Socket *clientSocket) {
    if((int) this->clientsConnected.size() >= this->maxClients) {
        return NULL;
    }
    
    Client *client = new Client(clientSocket);  // Já cria com Channel = NULL, mute = false, adm = false e nickname = ""

    m_cli.lock();
    this->clientsConnected.push_back(client);
    m_cli.unlock();
    
    thread clientHandling([] (Server *server, Client *client) {server->HandleClient(client);}, this, client);
    clientHandling.detach();
    
    return client;
}

// Add Client to a channel
void Server::AddClientToChannel(Client *client, Channel *channel) {
    m_cha.lock();
    channel->AddClient(client);
    m_cha.unlock();
}
// Remove Client from Server
void Server::RemoveClientFromServer(Client *clientToRemove) {
    Channel *channel = (Channel *) clientToRemove->GetChannel();
    if(channel != NULL) {
        RemoveClientFromChannel(clientToRemove, &channel);
    }

    m_cli.lock();
    for(vector<Client *>::iterator cli = this->clientsConnected.begin() ; cli != this->clientsConnected.end() ; cli++) {
        if((*cli) == clientToRemove) {
            delete (*cli);
            this->clientsConnected.erase(cli);
            break;
        }
    }
    m_cli.unlock();
}
<<<<<<< HEAD
// Remove client from channel
void Server::RemoveClientFromChannel(Client *clientToRemove) {
    Channel *channel = (Channel *) clientToRemove->GetChannel();
=======

void Server::RemoveClientFromChannel(Client *clientToRemove, Channel **channel) {
    if((*channel)->FindClientByName(clientToRemove->GetNickname()) == NULL) {
        return;
    }

>>>>>>> c277d9b9fd3b4132da63f5c0ffe31fd9660b71cd
    bool wasAdm = clientToRemove->IsAdm();
    
    m_cha.lock();
    (*channel)->RemoveClient(clientToRemove);
    m_cha.unlock();

    if((*channel)->IsEmpty()) {
        DeleteChannel((*channel));
        *channel = NULL;
        return;
    }
    else if(wasAdm) {
        m_cha.lock();
        Client *newAdm = (*channel)->ChooseNewAdm();
        m_cha.unlock();
        SendToChannel(newAdm->GetNickname() + " é o novo Admin do canal", (*channel));
    }
}

// Print Log in Server Terminal
void Server::Log(string msg) {
    m_log.lock();
    cout << "-> " << msg << endl;
    m_log.unlock();
}

// Send Message from channel to server
void Server::SendToChannel(string msg, Channel *channel) {
    Log("(server)" + channel->GetName() + " " + msg);
    string msgToSend = msg + " <<<<<";
    
    vector<Client *> clientList = channel->GetClientsInChannel();
    for(vector<Client *>::iterator cli = clientList.begin() ; cli != clientList.end() ; cli++) {
        SendToClient(msgToSend, (*cli));
    }
}
        
void Server::SendToChannel(string msg, Channel *channel, Client *sender) {
    Log("(" + sender->GetNickname() + ")" + channel->GetName() + " " + msg);
    string msgToSend = msg;

    vector<Client *> clientList = channel->GetClientsInChannel();
    for(vector<Client *>::iterator cli = clientList.begin() ; cli != clientList.end() ; cli++) {
        SendToClient(msgToSend, (*cli), sender);
    }
}

void Server::SendToClient(string msg, Client *receiver) {
    string msgToSend = ">>>>> " + msg;
    
    m_cli.lock();
    receiver->GetSocket()->Send(msgToSend);
    m_cli.unlock();
}

void Server::SendToClient(string msg, Client *receiver, Client *sender) {
    string msgToSend;
    if(sender == receiver) {
        msgToSend = sender->GetNickname() + " (You) : " + msg;
    }  
    else {
        msgToSend = sender->GetNickname() + " : " + msg;
    }
    
    m_cli.lock();
    receiver->GetSocket()->Send(msgToSend);
    m_cli.unlock();
}

void Server::ExecuteCommand(string commandReceived, Client *sender) {
    string command;
    int argc;
    char **argv;
    getCommandAndArgs(commandReceived, &command, &argc, &argv);
    
    if(command == "/help") {
        CommandHelp(sender);
    }
    else if(command == "/info") {
        CommandInfo(sender);
    }
    else if(command == "/ping") {
        CommandPing(sender);
    }
    else if(command == "/nickname"){
        if(!RespondIfNotEnoughArguments(argc, 1, sender)) {
            CommandNickname(string(argv[0]), sender);
        }
    }
    else if(command == "/join"){
        if(!RespondIfNotEnoughArguments(argc, 1, sender)) {
            CommandJoin(string(argv[0]), sender);
        }
    }
    else if(command == "/part"){
        CommandPart(sender);
    }
    else if(command == "/kick"){
        if(!RespondIfNotEnoughArguments(argc, 1, sender)) {
            CommandKick(string(argv[0]), sender);
        }
    }
    else if(command == "/whois"){
        if(!RespondIfNotEnoughArguments(argc, 1, sender)) {
            CommandWhois(string(argv[0]), sender);
        }
    }
    else if(command == "/mute"){
        if(!RespondIfNotEnoughArguments(argc, 1, sender)) {
            CommandMute(string(argv[0]), sender);
        }
    }
    else if(command == "/unmute"){
        if(!RespondIfNotEnoughArguments(argc, 1, sender)) {
            CommandUnmute(string(argv[0]), sender);
        }
    }
    else if(command == "/mode"){
        if(!RespondIfNotEnoughArguments(argc, 1, sender)) {
            string argument = string(argv[0]);
            if(argument == "public") {
                CommandMode(false, sender);
            }
            else if(argument == "private") {
                CommandMode(true, sender);
            }
            else {       
                SendToClient("Argumento inválido para mode", sender);
            }
        }
    }
    else if(command == "/invite"){
        if(!RespondIfNotEnoughArguments(argc, 1, sender)) {
            CommandInvite(string(argv[0]), sender);
        }
    }
    else {
        CommandInvalid(sender);
    }

    freeArgvMemory(argc, &argv);
}

void Server::CommandHelp(Client *sender) {
    string commands = "";
    commands += "Os comandos do servidor são:\n";
    commands += " - /connect IP PORT - Conecta ao servidor pelo IP e PORT fornecidos, ligando a aplicação\n";
    commands += " - /quit - Sai do servidor, desligando a aplicação\n";
    commands += " - /help - Obter informações sobre os comandos\n";
    commands += " - /info - Obter informações sobre o servidor/canal\n";
    commands += " - /ping - Recebe uma resposta 'pong' do servidor\n";
    commands += " - /nickname NICKNAME - Define/troca seu apelido\n";
    commands += " - /join CHANNEL_NAME - Entra no canal desejado\n";
    commands += " - /part - Deixa o canal atual\n";
    commands += " - /kick NICKNAME - Expulsa usuário do canal (apenas adm)\n";
    commands += " - /whois NICKNAME - Descobre o IP do usuário (apenas adm)\n";
    commands += " - /mute NICKNAME - Muta usuário do canal (apenas adm)\n";
    commands += " - /unmute NICKNAME - Desmuta usuário do canal (apenas adm)\n";
    commands += " - /mode 'public'|'private' - Determina se o canal é público ou privado (apenas adm)\n";
    commands += " - /invite NICKNAME - Convida usuário ao canal, permitindo sua entrada se este for privado (apenas adm)\n\n";
    
    SendToClient(commands, sender);
}

void Server::CommandInfo(Client *sender) {
    string message = "";
    message += "\n";
    message += "!-------- CRÉDITOS --------!\n";
    message += "!--------------------------!\n";
    message += "!......Eduardo Fares.......!\n";
    message += "!....Gabriel Batistella....!\n";
    message += "!....Guilherme Villela.....!\n";
    message += "!.....Vítor Rinaldini......!\n";
    message += "!--------------------------!\n";
    message += "\n";
    message += "!-------- Informações do MKserver --------!\n";
    message += "Clientes conectados: " + to_string(this->clientsConnected.size()) + "\n";
    message += "Total de canais: " + to_string(this->channelsOpen.size()) + "\n";
    message += ChannelsAvailableMessage();
    
    if(sender->GetChannel() != NULL) {
        Channel *channel = (Channel *) sender->GetChannel();

        message += "!-------- Informações do canal --------!\n";
        message += "Nome: " + channel->GetName() + "\n";
        message += "Clientes conectados: " + to_string(channel->NumberOfClientsConnected()) + "\n";
        if(channel->IsPrivate()) {
            message += "Status: Privado\n";
        }
        else {
            message += "Status: Público\n";
        }
        if(sender->IsAdm()) {
            if(channel->IsPrivate()) {
                message += channel->ClientsInvitedMessage();
            }
            message += channel->ClientsOnChannelMessage();
        }
    }

    SendToClient(message, sender);
}

void Server::CommandPing(Client *sender) {
    if(sender->GetNickname() == "") {
        Log("um cliente enviou um ping");
    }
    else {
        Log(sender->GetNickname() + " enviou um ping");
    }
    SendToClient("pong", sender);
    Log("servidor respondeu com um pong");
}

void Server::CommandNickname(string nickname, Client *sender) {
    for(vector<Client *>::iterator cli = this->clientsConnected.begin() ; cli != this->clientsConnected.end() ; cli++) {
        if((*cli)->GetNickname() == nickname) {
            SendToClient("O nickname " + nickname + " já está sendo utilizado", sender);
            return;
        }
    }

    if(sender->SetNickname(nickname)) {
        SendToClient("O seu nickname agora é " + nickname, sender);
    }
    else {
        SendToClient("O nickname " + nickname + " é inválido", sender);
    }
}

void Server::CommandJoin(string channelName, Client *sender) {
    if(sender->GetChannel() != NULL) {
        SendToClient("Você já está em um canal", sender);
        return;
    }

    if(sender->GetNickname() == "") {
        SendToClient("Você deve setar um nickname primeiro", sender);
        return;
    }
    
    Channel *channel = this->FindChannelByName(channelName);  
    if(channel == NULL) {
        try {
            channel = CreateChannel(channelName);
            Log(sender->GetNickname() + " abriu o canal " + channelName);
            
            channel->AddClient(sender);
            SendToChannel(sender->GetNickname() + " ingressou no canal", channel);
            SendToChannel(sender->GetNickname() + " é o novo Admin do canal", channel);
        }
        catch(...) {
            SendToClient("O canal não foi aberto porque o nome escolhido é inválido", sender);
        }
    }
    else {
        if(channel->AddClient(sender)) {
            SendToChannel(sender->GetNickname() + " ingressou no canal", channel); 
        } else {
            SendToClient("O canal em que você tentou ingressar é privado", sender);
        }
    }
}

void Server::CommandPart(Client *sender) {
    if(RespondIfNotInChannel(sender)) return;

    Channel *channel = (Channel *) sender->GetChannel();
    RemoveClientFromChannel(sender, &channel);

    if(channel != NULL) {
        SendToChannel(sender->GetNickname() + " saiu do canal", channel);
    }
    SendToClient("Você saiu do canal", sender);
    SendToClient(ChannelsAvailableMessage(), sender);
}

void Server::CommandKick(string nickname, Client *sender){
    if(RespondIfNotInChannel(sender)) return;
    if(RespondIfClientNotAdm(sender)) return;
    if(RespondIfNickNonExistingInChannel(nickname, (Channel *) sender->GetChannel(), sender)) return;

    Channel *channel = (Channel *) sender->GetChannel();
    Client *client = channel->FindClientByName(nickname);
    RemoveClientFromChannel(client, &channel);

    if(channel != NULL) {
        SendToChannel("O usuário " + nickname + " foi expulso do canal pelo admin " + sender->GetNickname(), channel);
    }
    SendToClient("Você foi expulso do canal pelo admin " + sender->GetNickname(), client);
    SendToClient(ChannelsAvailableMessage(), client);
}

void Server::CommandWhois(string nickname, Client *sender){
    if(RespondIfNotInChannel(sender)) return;
    if(RespondIfClientNotAdm(sender)) return;
    if(RespondIfNickNonExistingInChannel(nickname, (Channel *) sender->GetChannel(), sender)) return;

    Channel *channel = (Channel *) sender->GetChannel();
    Client *client = channel->FindClientByName(nickname);

    SendToClient("O IP encontrado para o nickname " + nickname + " é: " + client->GetSocket()->GetIP(), sender);
}

void Server::CommandMute(string nickname, Client *sender){
    if(RespondIfNotInChannel(sender)) return;
    if(RespondIfClientNotAdm(sender)) return;
    if(RespondIfNickNonExistingInChannel(nickname, (Channel *) sender->GetChannel(), sender)) return;
    
    Channel *channel = (Channel *) sender->GetChannel();
    Client *client = channel->FindClientByName(nickname);

    if(!client->IsMute()) {
        client->SetMute(true);
        SendToClient("O usuário " + nickname + " foi mutado", sender);
    }   
    else {
        SendToClient("O usuário " + nickname + " já estava mutado", sender);   
    }
}

void Server::CommandUnmute(string nickname, Client *sender){
    if(RespondIfNotInChannel(sender)) return;
    if(RespondIfClientNotAdm(sender)) return;
    if(RespondIfNickNonExistingInChannel(nickname, (Channel *) sender->GetChannel(), sender)) return;
    
    Channel *channel = (Channel *) sender->GetChannel();
    Client *client = channel->FindClientByName(nickname);

    if(client->IsMute()) {
        client->SetMute(false);
        SendToClient("O usuário " + nickname + " foi desmutado", sender);
    }   
    else {
        SendToClient("O usuário " + nickname + " já estava desmutado", sender);   
    }
}

void Server::CommandMode(bool isPrivate, Client *sender) {
    if(RespondIfNotInChannel(sender)) return;
    if(RespondIfClientNotAdm(sender)) return;
    
    Channel *channel = (Channel *) sender->GetChannel();

    if(channel->IsPrivate()) {
        if(!isPrivate) {
            channel->SetPrivate(false);
            SendToChannel("O canal agora é público", channel);
        }
        else {
            SendToClient("O canal já é privado", sender);
        }
    }
    else {
        if(isPrivate) {
            channel->SetPrivate(true);
            channel->ResetInvitedList();
            SendToChannel("O canal agora é privado", channel);
        }
        else {
            SendToClient("O canal já é público", sender);
        }
    }
}

void Server::CommandInvite(string nickname, Client *sender){
    if(RespondIfNotInChannel(sender)) return;
    if(RespondIfClientNotAdm(sender)) return;
    
    Channel *channel = (Channel *) sender->GetChannel();

    if(!channel->IsPrivate()) {
        SendToClient("O canal é público, não há lista de convidados", sender);
        return;
    }
    
    if(channel->IsClientInvited(nickname)) {
        SendToClient("O usuário " + nickname + " já está convidado para o canal", sender);
    }   
    else {
        channel->InviteClient(nickname);
        SendToClient("O usuário " + nickname + " foi convidado para o canal", sender);   
    }

    SendToClient(channel->ClientsInvitedMessage(), sender);
}

void Server::CommandInvalid(Client *sender) {
    SendToClient("Comando inválido!", sender);
}

bool Server::RespondIfNotInChannel(Client *client) {
    if(client->GetChannel() == NULL) {
        SendToClient("Você deve entrar em um canal primeiro", client);
        return true;
    }
    else {
        return false;
    }
}

bool Server::RespondIfClientNotAdm(Client *client) {
    if(!client->IsAdm()) {
        SendToClient("Você precisa ser admin para utilizar esse comando", client);
        return true;
    }
    else {
        return false;
    }
}

bool Server::RespondIfNickNonExistingInChannel(string nickname, Channel *channel, Client *client) {
    if(channel->FindClientByName(nickname) == NULL) {
        SendToClient("O nickname passado não foi encontrado no canal", client);
        return true;
    }
    else {
        return false;
    }
}

bool Server::RespondIfNotEnoughArguments(int have, int needed, Client *client) {
    if(have < needed) {
        SendToClient("Argumentos insuficientes para o comando", client);
        return true;
    }
    else {
        return false;
    }
}

void Server::ListenForClients() {
    while(true) {
        Socket *socket = this->listenerSocket->Accept();
        Client *newClient = AddClientToServer(socket);
        if(newClient == NULL) {
            delete socket;
        }
        else {
            Log("Um novo cliente entrou no servidor");
            SendToClient(ChannelsAvailableMessage(), newClient);
        }
    }
}

void Server::HandleClient(Client *client) {
    while(true) {
        string receiveMsg = client->GetSocket()->Receive();
        if(receiveMsg == "") {
            if(client->GetChannel() != NULL) {
                SendToChannel(client->GetNickname() + " saiu do servidor", (Channel *) client->GetChannel());
            }
            else {
                Log("Um cliente saiu do servidor");
            }
            RemoveClientFromServer(client);
            return;
        }
        else if(receiveMsg.at(0) == '/') {
            ExecuteCommand(receiveMsg, client);
        }
        else if(client->GetChannel() != NULL) {
            if(client->IsMute()) {
                SendToClient("Você está mutado. Suas mensagens não são lidas pelos outros participantes do canal.", client);
            }
            else {
                SendToChannel(receiveMsg, (Channel *) client->GetChannel(), client);
            }
        }
    }
}

Server::Server(string ip, int port, int maxClients) {
    this->listenerSocket = new Socket(ip, port);
    this->maxClients = maxClients;
}

Server::Server(string ip, int maxClients) {
    this->listenerSocket = new Socket(ip);
    this->maxClients = maxClients;
}

Server::Server(int port, int maxClients) {
    this->listenerSocket = new Socket(port);
    this->maxClients = maxClients;
}

Server::Server(int maxClients) {
    this->listenerSocket = new Socket();
    this->maxClients = maxClients;
}

void Server::Start() {
    this->listenerSocket->Listen(SOMAXCONN);
    cout << "Servidor Aberto. Esperando conexões na porta " << this->listenerSocket->GetPort() << endl;

    thread listeningThread([] (Server *server) {server->ListenForClients();}, this);
    listeningThread.detach();
}

Server::~Server() {
    delete listenerSocket;

    while(this->clientsConnected.size() > 0) {
        RemoveClientFromServer(this->clientsConnected.at(0));
    }
    
    this->clientsConnected.clear();
    this->channelsOpen.clear();
}