//
//  Client.h
//  ChatRoom
//
//  Created by Caleb Jonassaint on 10/20/14.
//  Copyright (c) 2014 Caleb Jonassaint. All rights reserved.
//

#ifndef __ChatRoom__Client__
#define __ChatRoom__Client__

#include <stdio.h>
#include <vector>
#include <netinet/in.h>

class Client {
    pthread_t clientThread;
public:
    /// variables:
    void* server;
    std::vector<Service*> services;
    int sock;
    struct sockaddr_in* sockAddress;
    /// functions:
    Client(int sock, struct sockaddr_in* sockAddress, void* server);
    // used to allow or dis-allow certain Services from hearing everything you say (security measure)
    bool shouldAuthenticateService(void* service);
    bool sendMessage(std::string message);
};

#endif /* defined(__ChatRoom__Client__) */
