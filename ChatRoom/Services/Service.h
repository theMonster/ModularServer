//
//  Service.h
//  ChatRoom
//
//  Created by Caleb Jonassaint on 10/20/14.
//  Copyright (c) 2014 Caleb Jonassaint. All rights reserved.
//

#ifndef __ChatRoom__Service__
#define __ChatRoom__Service__

#include <iostream>
#include <string.h>
#include <vector>

class Service {
public:
    virtual void recievedCommand(std::string command, std::vector<std::string> params, std::string parameters, void* client) = 0;
    virtual void recievedMessageFromClient(std::string message, void* client) = 0;
    virtual void newClientDidConnectToServer(void *client) = 0;
    virtual void clientDidDisconnectFromServer(void *client) = 0;
};

#endif /* defined(__ChatRoom__Service__) */
