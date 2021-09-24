// Socket library -*- C++ -*-

// Copyright (C) 2006 Christophe Tournayre
//
// This library is free software; you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2, or
// (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// As a special exception, you may use this file as part of a free software
// library without restriction.  Specifically, if other files instantiate
// templates or use macros or inline functions from this file, or you compile
// this file and link it with other files to produce an executable, this
// file does not by itself cause the resulting executable to be covered by
// the GNU General Public License.  This exception does not however
// invalidate any other reasons why the executable file might be covered by
// the GNU General Public License.


#ifndef __SOCKET_HPP__
#define __SOCKET_HPP__

// Type de la connexion (TCP ou UDP)
#define TCP


// ********************************************************
// Les includes
// ********************************************************
#if defined(WIN32)
    #include <winsock2.h> // pour les fonctions socket
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <linux/socket.h>
#endif

#include <string>
#include <iostream> //Pour cout, cin, cerr

// ********************************************************
// Les librairies
// ********************************************************
#pragma comment(lib,"ws2_32.lib")

// ********************************************************
// Déclaration des structures
// ********************************************************

// Structure pour l'inforamtion de la socket
typedef struct
{
    SOCKET id;          // Déclaration de l'identifiant e la socket
    SOCKADDR_IN server; // Déclaration de la structure des informations lié au serveur
    bool valid;
} Sock;


Sock connect(char* address, int port);
int send(Sock info, char *buffer, int size);
int recv(Sock info,char *buffer,int size);
int disconnect(Sock info);







#endif
