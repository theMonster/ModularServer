//
//  main.c
//  ChatRoom
//
//  Created by Caleb Jonassaint on 10/20/14.
//  Copyright (c) 2014 Caleb Jonassaint. All rights reserved.
//

#include "Server.h"
#include "Chatroom.h"

//void killServ(int signal) {
//}

int main(int argc, char *argv[])
{
    // setup listeners
//    signal(SIGKILL, killServ);
//    signal(SIGTERM, killServ);
//    signal(SIGQUIT, killServ);
//    signal(SIGCHLD, killServ);
//    signal(SIGINT, killServ);

    
    Server serv = Server();
    serv.startServer();
    // add all services to server here:
    serv.addServiceToServer((Service*)new Chatroom(&serv));
    serv.joinServerThread();
    
    return 0;
}
