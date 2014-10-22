//
//  main.c
//  ChatRoom
//
//  Created by Caleb Jonassaint on 10/20/14.
//  Copyright (c) 2014 Caleb Jonassaint. All rights reserved.
//

#include "Server.h"
#include "Chatroom.h"

int main(int argc, char *argv[])
{
    Server serv = Server();
    serv.startServer();
    // add all services to server here:
    serv.addServiceToServer((Service*)new Chatroom(&serv));
    serv.joinServerThread();
    return 0;
}
