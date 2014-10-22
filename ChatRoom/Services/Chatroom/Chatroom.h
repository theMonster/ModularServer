//
//  Chatroom.h
//  ChatRoom
//
//  Created by Caleb Jonassaint on 10/20/14.
//  Copyright (c) 2014 Caleb Jonassaint. All rights reserved.
//

#ifndef __ChatRoom__Chatroom__
#define __ChatRoom__Chatroom__

#include <stdio.h>
#include "Client.h"

struct Room {
    std::string roomName;
    std::vector<Client*> clients;
};

class Chatroom : Service {
    void *server;
    std::vector<struct Room*> rooms;
    std::map<void*,std::string*> usernames;
    void joinRoom(struct Room* room, Client *client);
    struct Room* findRoomForName(std::string name);
    struct Room* findRoomForName(std::string name, int& index);
    void removeClientFromRoom(Client* client, struct Room* room);
public:
    Chatroom(void* server);
    void recievedCommand(std::string command, std::vector<std::string> params, std::string parameters, void *client);
    void recievedMessageFromClient(std::string message, void *client);
    void newClientDidConnectToServer(void *client);
    void clientDidDisconnectFromServer(void* client);
};

#endif /* defined(__ChatRoom__Chatroom__) */
