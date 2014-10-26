//
//  Server.h
//  ChatRoom
//
//  Created by Caleb Jonassaint on 10/20/14.
//  Copyright (c) 2014 Caleb Jonassaint. All rights reserved.
//

#ifndef __ChatRoom__Server__
#define __ChatRoom__Server__

#include "Settings.h"
#include "Service.h"
#include "Client.h"
#include <stdio.h>
#include <vector>
#include <map>
#include <string.h>
#include <iostream>

class Server {
    pthread_t acceptanceThread;
    std::map<std::string, Service*> commands;
public:
    /// variables:
    std::vector<Client*> clients;
    std::vector<Service*> services;
    int sock;
    /// functions:
    Server();
    ~Server();
    void startServer();
    void joinServerThread();
    // registration / init stuff (For Services)
    void addServiceToServer(Service* service);
    bool registerCommandWithService(std::string command, Service* service);
    // message sending
    void sendMessageToAllClients(std::string message);
    void sendMessageToClients(std::string message, std::vector<Client*> clients);
    void sendMessageToClient(std::string message, Client* client);
    // listening to all messages from a client (For Services)
    bool startListeningToClient(Client* client, Service* service);
    void stopListeningToClient(Client* client, Service* service);
    // client delegate stuff
    void clientDidConnect(Client *client);
    void clientDidSendMessage(std::string message, Client* client);
    void clientDidSendCommand(std::string command, std::vector<std::string> params, std::string parameters, Client* client);
    void clientDidDisconnect(Client* client);
};

#endif /* defined(__ChatRoom__Server__) */
