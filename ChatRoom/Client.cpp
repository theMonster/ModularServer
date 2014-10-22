//
//  Client.cpp
//  ChatRoom
//
//  Created by Caleb Jonassaint on 10/20/14.
//  Copyright (c) 2014 Caleb Jonassaint. All rights reserved.
//

#include "Server.h"
#include "Client.h"
#include "Settings.h"
#include <pthread.h>
#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <sstream>

void* listenToClient(void* client);

Client::Client(int sock, struct sockaddr_in* sockAddress, void* server) {
    this->sock = sock;
    this->sockAddress = sockAddress;
    this->server = (Client*)server;
    // start thread
    pthread_create(&clientThread, NULL, listenToClient, this);
}

bool Client::shouldAuthenticateService(void* service) {
    return true; // For now, we don't need to really do anything with this.
}

bool Client::sendMessage(std::string message) {
    ssize_t bufferSize = write(sock, message.c_str(), message.length());
    // check if the message was successfully sent
    bool success = false; // by default it's not
    if (bufferSize == message.length()) { // it was
        success = true;
    }
    return success;
}

/********************** LAZY UTILS / TODO: PLACE IN IT'S OWN CLASS *********************/

void findAndRepalceString(std::string& target, const std::string& oldStr, const std::string& newStr)
{
    if (oldStr.compare(newStr) == 0 || oldStr.compare("")) { // it doesn't make sense to replaces something with itself and it doesn't make any sense to remove nothing.
        return; // don't process this request because it doesn't make sense.
    }
    size_t pos = 0;
    while((pos = target.find(oldStr, pos)) != std::string::npos)
    {
        target.replace(pos, oldStr.length(), newStr);
        pos += newStr.length();
    }
}

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

/********************** LAZY UTILS / TODO: PLACE IN IT'S OWN CLASS *********************/

void* listenToClient(void* client_t) {
    Client* clientObj = (Client*)client_t;
    Server* server = (Server*)clientObj->server;
    long read_size;
    int client = clientObj->sock;
    
    printf("Connected to client #%i\n", client);
    
    // send info message
    
    
    while(1)
    {
        char buffer[CONNECTION_BUFFER_SIZE] = "";
        read_size = recv(client, buffer, CONNECTION_BUFFER_SIZE , 0);
        if (read_size > 0) {
            std::string message = std::string(buffer);
            // (clean str) remove \n and \r
            findAndRepalceString(message, "\n", "");
            findAndRepalceString(message, "\r", "");
            std::vector<std::string> stringsSplitBySpace = split(message, ' ');
            
            // log to console
            printf("message from client #%i: \"%s\"\n", client, message.c_str());

            // check for '/quit' (it's a command, but it's not implemented by a service, instead it's implemented by the client to ensure security)
            if (message.compare("/quit") == 0) {
                // client has disconected.
                server->clientDidDisconnect(clientObj);
                close(clientObj->sock);
                delete clientObj;
                printf("Client #%i has quit\n", client);
                return NULL; // breaks out of loop and destroys thread
            }
            
            // process message as command, or as normal message
            char prefix = (stringsSplitBySpace.size() > 0) ? stringsSplitBySpace[0][0] : '\0';
            if (prefix == '/') { // this is a command
                findAndRepalceString(stringsSplitBySpace[0], "/", "");
                findAndRepalceString(message, stringsSplitBySpace[0], ""); // removes command from string
                findAndRepalceString(message, "/ ", ""); // removes extras from string
                server->clientDidSendCommand(stringsSplitBySpace[0], stringsSplitBySpace, message, clientObj);
            } else {
                server->clientDidSendMessage(message, clientObj);
            }
        } else {
            // client has disconected.
            close(clientObj->sock);
            server->clientDidDisconnect(clientObj);
            printf("Disconnected from client: #%i\n", client);
            // Clean up
            delete clientObj;
            break;
        }
    }
    
    return NULL;
}
