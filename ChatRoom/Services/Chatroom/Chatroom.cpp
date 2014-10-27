//
//  Chatroom.cpp
//  ChatRoom
//
//  Created by Caleb Jonassaint on 10/20/14.
//  Copyright (c) 2014 Caleb Jonassaint. All rights reserved.
//

#include "Server.h"
#include "Chatroom.h"
#include "Settings.h"
#include <map>
#include <stdio.h>

Chatroom::Chatroom(void* server_t) {
    rooms = std::vector<struct Room*>();
    
    // add sample room(s)
    struct Room* r = new struct Room();
    r->roomName = "General";
    r->clients = std::vector<Client*>();
    rooms.push_back(r);
    
    server = server_t;
    Server* server = (Server*)server_t;
    // register commands (/rooms, /join, /leave, /create, /delete)
    server->registerCommandWithService("rooms", this);
    server->registerCommandWithService("join", this);
    server->registerCommandWithService("leave", this);
    server->registerCommandWithService("create", this);
    server->registerCommandWithService("delete", this);
    server->registerCommandWithService("help", this);
}

#pragma mark - server delegate functions
void Chatroom::recievedCommand(std::string command, std::vector<std::string> params, std::string parameters, void *client_t) {
    Server* server = (Server*)this->server;
    Client* client = (Client *)client_t;
    // TODO: Could use refactoring (split into multiple functions)
    // handle commands
    if (command.compare("rooms") == 0) {
        std::string openRooms = "Active Rooms are:\n";
        // make string of all rooms
        for (int i = 0; i < rooms.size(); ++i) {
            struct Room* r = rooms[i];
            char tmp[CONNECTION_BUFFER_SIZE];
            snprintf(tmp, CONNECTION_BUFFER_SIZE, "%s\t*%s (%lu)\n", openRooms.c_str(), r->roomName.c_str(), r->clients.size());
            openRooms = std::string(tmp);
        }
        // send list of rooms to client (make sure to allocate a new string on the heap)
        server->sendMessageToClient(openRooms, client);
    } else if (command.compare("join") == 0) {
        struct Room* r = findRoomForName(parameters);
        // check if we found the room
        if (r) { // found it
            joinRoom(r, client);
        } else { // didn't find it
            char tmp[CONNECTION_BUFFER_SIZE];
            snprintf(tmp, CONNECTION_BUFFER_SIZE, "No room named \"%s\" exists.\n", parameters.c_str());
            server->sendMessageToClient(tmp, client);
        }
    } else if (command.compare("leave") == 0) {
        struct Room* r = findRoomForName(parameters);
        if (r) {
            // remove user from chatroom
            removeClientFromRoom(client, r);
        } else if (parameters.compare("") == 0) {
            // user wants to be removed from all rooms
            for (int i = 0; i < rooms.size(); ++i) {
                removeClientFromRoom(client, rooms[i]);
            }
        } else {
            char tmp[CONNECTION_BUFFER_SIZE];
            snprintf(tmp, CONNECTION_BUFFER_SIZE, "No room named \"%s\" exists.\n", parameters.c_str());
            server->sendMessageToClient(tmp, client);
        }
    } else if (command.compare("create") == 0) {
        // check if room name is taken
        if (findRoomForName(parameters)) {
            server->sendMessageToClient("That room already exists.\n", client);
        } else { // ok, create room!
            struct Room* newRoom = new struct Room;
            newRoom->roomName = parameters; // room name
            newRoom->clients = std::vector<Client*>();
            rooms.push_back(newRoom); // add new room to rooms
            // let user know that the operation was successful
            server->sendMessageToClient("Room was successfully created\n", client);
        }
    } else if (command.compare("delete") == 0) {
        int indexForRoom = -1;
        struct Room* r = findRoomForName(parameters, indexForRoom);
        // delete the room (only if there is no one in the room)
        if (indexForRoom == -1) {
            // let user know that it failed because the room doesn't exist.
            server->sendMessageToClient("Failed to delete room. Room does not exist.\n", client);
        } else if (rooms[indexForRoom]->clients.size() <= 0) {
            rooms.erase(rooms.begin() + indexForRoom);
            delete r;
            // tell ther user it was a success.
            server->sendMessageToClient("Room was successfully deleted.\n", client);
        } else {
            // tell user it failed, users are in the room.
            server->sendMessageToClient("Failed to delete room. Room must be empty first.\n", client);
        }
    } else if (command.compare("help") == 0) {
        std::string helpDialog = "You can use commands like:\n   /rooms - lists all the rooms on the chat server\n   /join {room name} - adds you to a room\n   /leave {room name} - removes you from a room\n   /leave - removes you from all of your active rooms\n   /create {new room name} - creates a new room\n   /delete {room name} - deletes a room\n   /quit - quits the chat server";
        server->sendMessageToClient(helpDialog, client);
    }
}

void Chatroom::recievedMessageFromClient(std::string message, void *client_t) {
    Server* server = (Server*)this->server;
    Client* client = (Client*)client_t;
    if (!usernames[client] || usernames[client]->empty()) { // the user isn't associated with a username
        // check if username exists
        bool doesUsernameExist = false;
        for(auto i = usernames.begin(); i != usernames.end(); i++) {
            if (i->second && i->second->compare(message) == 0) {
                doesUsernameExist = true;
                break;
            }
        }
        
        // handle the case
        if (!doesUsernameExist) {
            // if client doesn't have a name yet, we'll assume this message is the desired username...
            usernames[client] = new std::string(message);
        } else {
            char tmp[CONNECTION_BUFFER_SIZE];
            snprintf(tmp, CONNECTION_BUFFER_SIZE, "\"%s\" is already in use, please pick another.\nUsername?: ", message.c_str());
            server->sendMessageToClient(tmp, client);
        }
    } else {
        // broadcast message to all rooms that user belongs to
        for (int i = 0; i < rooms.size(); ++i) {
            struct Room* r = rooms[i];
            // check if the user belongs in this room:
            for (int j = 0; j < r->clients.size(); ++j) {
                // check if the user is in this room:
                if (r->clients[j] == client) {
                    // the user is in this room, broadcast message:
                    char tmp[CONNECTION_BUFFER_SIZE];
                    snprintf(tmp, CONNECTION_BUFFER_SIZE, "%s<%s>: %s\n", usernames[client]->c_str(), r->roomName.c_str(), message.c_str());
                    server->sendMessageToClients(tmp, r->clients);
                }
            }
        }
    }
}

void Chatroom::newClientDidConnectToServer(void *client_t) {
    Client* client = (Client*)client_t;
    Server* server = (Server*)this->server;
    std::string initialLoginMessage = "===============================\nWelcome to the PWNED chat server!\nTIP: (type \"/help\" for help)\n";
    // ask for username if the user doesn't have one by default.
    if (!client->username || client->username->empty()) {
        initialLoginMessage.append("Username?: ");
    }
    // send the message
    server->sendMessageToClient(initialLoginMessage, client);
    server->startListeningToClient(client, this); // TODO: handle case that client doesn't want to have this service listen
    
    // since the user does have a username... we want to fake the message send
    if (client->username && !client->username->empty()) { // otherwise, fake message reciept for client setting up username
        recievedMessageFromClient(*client->username, client);
    }
}

void Chatroom::clientDidDisconnectFromServer(void *client_t) {
    Client* client = (Client*)client_t;
    Server* server = (Server*)this->server;
    
    server->sendMessageToClient("BYE!\n", client);
    
    // remove username
    usernames.erase(client);
    
    // remove client from all rooms
    for (int i = 0; i < rooms.size(); ++i) {
        struct Room* r = rooms[i];
        removeClientFromRoom(client, r);
    }
}

#pragma mark - user functions
void Chatroom::joinRoom(struct Room* room, Client *client) {
    Server *server = (Server *)this->server;

    // remove client from room (saftey percaution)
    removeClientFromRoom(client, room);
    
    // add user to room
    room->clients.push_back(client);
    
    std::string listOfUsersInChatroom;
    // make string of all rooms
    for (int i = 0; i < room->clients.size(); ++i) {
        Client* c = room->clients[i];
        
        std::string format;
        if (c == client) format = "%s\t* %s <-- this is you\n";
        else format = "%s\t* %s\n";
        
        const char *username = usernames[c]->c_str();
        char tmp[CONNECTION_BUFFER_SIZE];
        snprintf(tmp, CONNECTION_BUFFER_SIZE, format.c_str(), listOfUsersInChatroom.c_str(), username);
        listOfUsersInChatroom = std::string(tmp);
    }
    
    // send welcome message
    char buff[CONNECTION_BUFFER_SIZE];
    snprintf(buff, CONNECTION_BUFFER_SIZE, "%s: Welcome %s!\nUsers currently in this chatroom:\n%s\n", room->roomName.c_str(), usernames[client]->c_str(), listOfUsersInChatroom.c_str());
    server->sendMessageToClient(buff, client);
}

#pragma mark - helpers
struct Room* Chatroom::findRoomForName(std::string name) {
    int tmp = 0;
    return findRoomForName(name, tmp);
}

struct Room* Chatroom::findRoomForName(std::string name, int& index) {
    // find room for name
    struct Room *r = NULL;
    for (int i = 0; i < rooms.size(); ++i) {
        if (rooms[i]->roomName.compare(name) == 0) {
            // found the room!
            r = rooms[i];
            index = i;
            break;
        }
    }
    return r;
}

void Chatroom::removeClientFromRoom(Client* client, struct Room* room) {
    Server *server = (Server *)this->server;

    // show user left message to remaining users (and current client)
    // check if user exists in usernames...
    if (usernames[client] != NULL) {
        char buff[CONNECTION_BUFFER_SIZE];
        snprintf(buff, CONNECTION_BUFFER_SIZE, "\t* \"%s\" has left room.\n", usernames[client]->c_str());
        server->sendMessageToClients(buff, room->clients);
    }
    
    // now actually remove user...
    for (int i = 0; i < room->clients.size(); ++i) {
        if (client == room->clients[i]) {
            room->clients.erase(room->clients.begin() + i);
        }
    }
}

