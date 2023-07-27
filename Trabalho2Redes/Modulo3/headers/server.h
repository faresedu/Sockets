#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include "mkSocket.h"
#include "client.h"
#include "channel.h"

using namespace std;

class Server {

    private:
        Socket *listenerSocket;
        vector<Client *> clientsConnected;
        vector<Channel *> channelsOpen;
        int maxClients;
        mutex m_cli, m_cha, m_log;

        Channel * CreateChannel(string channelName);
        void DeleteChannel(Channel *channel);
        Channel * FindChannelByName(string channelName);
        string ChannelsAvailableMessage();

        Client * AddClientToServer(Socket *clientSocket);
        void AddClientToChannel(Client *client, Channel *channel);
        void RemoveClientFromServer(Client *clientToRemove);
        void RemoveClientFromChannel(Client *clientToRemove, Channel **channel);

        void Log(string msg);
        void SendToChannel(string msg, Channel *channel);
        void SendToChannel(string msg, Channel *channel, Client *sender);
        void SendToClient(string msg, Client *receiver);
        void SendToClient(string msg, Client *receiver, Client *sender);

        void ExecuteCommand(string commandReceived, Client *sender);
        void CommandHelp(Client *sender);
        void CommandInfo(Client *sender);
        void CommandPing(Client *sender);
        void CommandNickname(string nickname, Client *sender);
        void CommandJoin(string channelName, Client *sender);
        void CommandPart(Client *sender);
        void CommandKick(string nickname, Client *sender);
        void CommandWhois(string nickname, Client *sender);
        void CommandMute(string nickname, Client *sender);
        void CommandUnmute(string nickname, Client *sender);
        void CommandMode(bool isPrivate, Client *sender);
        void CommandInvite(string nickname, Client *sender);
        void CommandInvalid(Client *sender);

        bool RespondIfNotInChannel(Client *client);
        bool RespondIfClientNotAdm(Client *client);
        bool RespondIfNickNonExistingInChannel(string nickname, Channel *channel, Client *client);
        bool RespondIfNotEnoughArguments(int have, int needed,  Client *client);

    protected:
        void ListenForClients();
        void HandleClient(Client *client);

    public:
        Server(string ip, int port, int maxClients);
        Server(string ip, int maxClients);
        Server(int port, int maxClients);
        Server(int maxClients);

        void Start();

        ~Server();
};

#endif