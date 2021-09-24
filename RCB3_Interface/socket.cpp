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
#include <stdafx.h>
#include "socket.h"

Sock connect(char* address, int port)
{
    // ********************************************************
    // Déclaration des variables
    // ********************************************************
    WSADATA initialisation_win32; // Variable permettant de récupérer la structure d'information sur l'initialisation

    Sock info;
    info.valid = true;
    int erreur=0; // Variable permettant de récupérer la valeur de retour des fonctions utilisées

    // ********************************************************
    // Initialisation de Winsock
    // ********************************************************
    erreur=WSAStartup(MAKEWORD(2,2),&initialisation_win32);

    if (erreur!=0)
    {
        //cerr << "Desole, je ne peux pas initialiser Winsock du a l'erreur : "<< erreur << " " << WSAGetLastError() << endl;
        info.valid = false;
        return info;
    }

    // ********************************************************
    // Ouverture d'une Socket
    // ********************************************************
    info.id=socket(AF_INET,SOCK_STREAM,0);

    if (info.id==INVALID_SOCKET)
    {
        //cerr << "Desole, je ne peux pas creer la socket du a l'erreur : " << WSAGetLastError() << endl;
        info.valid = false;
        return info;
    }

    // ********************************************************
    // Ouverture de la session
    // ********************************************************
    info.server.sin_family=AF_INET;
    info.server.sin_addr.s_addr=inet_addr(address); // @ip serveur
    info.server.sin_port=htons(port); // Port écouté du serveur

    erreur=connect(info.id,(struct sockaddr*)&info.server,sizeof(info.server));

    if (erreur!=0)
    {
        //cerr <<"Desole, je n'ai pas pu ouvrir la session TCP : " << erreur << " " << WSAGetLastError() << endl;
        info.valid = false;
    }

    return info;
}


int send(Sock info, char *buffer, int size)
{
    // ********************************************************
    // Déclaration des variables
    // ********************************************************
    int nb;

    // ********************************************************
    // Envoi des données
    // ********************************************************
    if (size > 65535)
    {
        //cerr << "Message trop long!" << endl;
        return -1;
    }

    nb=send(info.id,buffer,size,0);

    if (nb==SOCKET_ERROR)
    {
        //cerr << "Desole, je n'ai pas envoyer les donnees du a l'erreur : " << WSAGetLastError() << endl;
        return -2;
    }

    return nb;
}


int recv(Sock info,char *buffer,int size)
{
    int nb=0;

    nb=recv(info.id,buffer,size,0);

    return nb;
}


int disconnect(Sock info)
{
    // ********************************************************
    // Déclaration des variables
    // ********************************************************
    int erreur=0, ret=0;

    // ********************************************************
    // Fermeture de la session TCP Correspondant à la commande connect()
    // ********************************************************
    erreur=shutdown(info.id,2);

    if (erreur!=0)
    {
        //cerr << "Desole, je ne peux pas fermer la session TCP du a l'erreur : " << erreur << " " << WSAGetLastError() << endl;
        ret = -1;
    }

    // ********************************************************
    // Fermeture de la socket correspondant à la commande socket()
    // ********************************************************
    erreur=closesocket(info.id);

    if (erreur!=0)
    {
        //cerr << "Desole, je ne peux pas liberer la socket du a l'erreur : " << erreur << " " << WSAGetLastError() << endl;
        ret = -2;
    }

    // ********************************************************
    // Quitte le winsock ouvert avec la commande WSAStartup
    // ********************************************************
    erreur=WSACleanup();

    if (erreur!=0)
    {
        //cerr << "Desole, je ne peux pas liberer winsock du a l'erreur : " << erreur << " " <<WSAGetLastError() << endl;
        ret= -3;
    }
    return ret;
}
