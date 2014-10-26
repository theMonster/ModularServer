//
//  Server.cpp
//  ChatRoom
//
//  Created by Caleb Jonassaint on 10/20/14.
//  Copyright (c) 2014 Caleb Jonassaint. All rights reserved.
//

#include "Server.h"

/* socket imports */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <time.h>
#include <pthread.h>

Server::Server() {
    commands = std::map<std::string, Service*>();
    services = std::vector<Service*>();
    clients = std::vector<Client*>();
    sock = socket(AF_INET, SOCK_STREAM, 0);
    // create serv sock
    if (sock < 0) {
        servAssert("Socket failed to be created", 1);
    }
    
    // create our address
    struct sockaddr_in servAddress;
    servAddress.sin_family = AF_INET;
    servAddress.sin_addr.s_addr = INADDR_ANY;
    servAddress.sin_port = htons(PORT_NO); // htons converts the value given to the appropriate network byte order (Little/Big Endian)
    
    if (bind(sock, (const struct sockaddr *)&servAddress, sizeof(servAddress))) {
        servAssert("Failed to bind to socket", 1);
    }
}

Server::~Server() {
    printf("Server is shutting down.\n");
    close(sock);
}

void* acceptanceFunction(void *server_t);

void Server::startServer() {
    // begin to listen for clients
    listen(sock, 3);

    // create thread to accept clients
    pthread_create(&acceptanceThread, NULL, acceptanceFunction, this);
}

void* acceptanceFunction(void* server_t) {
    printf("Server is up on port #%i, and is accepting incoming sockets.\n", PORT_NO);
    Server* server = (Server*)server_t;
    // accept anyone's connection request, and keep accepting... forever :O
    while (1) {
        const uint64_t tmp = sizeof(struct sockaddr_in);
        struct sockaddr_in* clientAddress;
        int clientSock = accept(server->sock, (struct sockaddr*)&clientAddress, (socklen_t*)&tmp);
        // create client
        Client* client = new Client(clientSock, clientAddress, server);
        server->clients.push_back(client);
    }
    
    return NULL;
}


void Server::joinServerThread() {
    pthread_join(acceptanceThread, NULL);
}

#pragma mark - registration / init stuff (For Services)
void Server::addServiceToServer(Service* service) {
    services.push_back(service);
}

bool Server::registerCommandWithService(std::string command, Service* service) {
    if (commands[command] == NULL) {
        commands[command] = service;
        return true;
    } else {
        return false;
    }
}

#pragma mark - message sending
void Server::sendMessageToAllClients(std::string message) {
    sendMessageToClients(message, clients);
}

void Server::sendMessageToClients(std::string message, std::vector<Client*> clients) {
    for (int i = 0; i < clients.size(); ++i) {
        clients[i]->sendMessage(message);
    }
}

void Server::sendMessageToClient(std::string message, Client* client) {
    client->sendMessage(message);
}

#pragma mark - listening to all messages from a client (For Services)
bool Server::startListeningToClient(Client* client, Service* service) {
    if (client->shouldAuthenticateService(service)) {
        client->services.push_back(service);
        return true;
    } else {
        return false;
    }
}

void Server::stopListeningToClient(Client* client, Service* service) {
    for (int i = 0; i < client->services.size(); ++i) {
        if (client->services[i] == service)
            (client->services.begin() - 1) + i;
    }
}
#pragma mark - client delegation stuff
void Server::clientDidSendMessage(std::string message, Client *client) {
    for (int i = 0; i < client->services.size(); ++i) {
        client->services[i]->recievedMessageFromClient(message, client);
    }
}

void Server::clientDidSendCommand(std::string command, std::vector<std::string> params, std::string parameters, Client *client) {
    Service *service = commands[command];
    if (service) service->recievedCommand(command, params, parameters, client);
    else sendMessageToClient("Command does not exist.\n", client);
}

void Server::clientDidConnect(Client *client) {
    // notify all services of the new client
    for (int i = 0; i < services.size(); ++i) {
        services[i]->newClientDidConnectToServer(client);
    }
}

void Server::clientDidDisconnect(Client *client) {
    for (int i = 0; i < services.size(); ++i) {
        // service should do all clean up here
        services[i]->clientDidDisconnectFromServer(client);
    }
    /// clean up instances of client in server
    // remove client from clients list
    for (int i = 0; i < clients.size(); ++i) {
        if (clients[i] == client) {
            clients.erase(clients.begin() + i);
        }
    }
}
