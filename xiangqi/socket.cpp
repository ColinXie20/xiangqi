//
//  socket.cpp
//  xiangqi
//
//  Created by Colin Xie on 7/9/24.
//

#include "socket.hpp"

#include <stdexcept>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

Server::Server(int port, bool ipv6) : _client(-1){
    _socket = socket(ipv6? AF_INET6:AF_INET, SOCK_STREAM, 0);
    if (_socket == -1){
        throw std::runtime_error("Server: Failed to create socket");
    }
    int bindResult;
    if (ipv6){
        struct sockaddr_in6 serverA = {
            .sin6_len = sizeof(struct sockaddr_in6),
            .sin6_family = AF_INET6,
            .sin6_addr = in6addr_any,
            .sin6_port = htons(port),
        };
        bindResult = bind(_socket, (struct sockaddr*)&serverA, sizeof(serverA));
    }else{
        struct sockaddr_in serverA = {
            .sin_len = sizeof(struct sockaddr_in),
            .sin_family = AF_INET,
            .sin_addr = INADDR_ANY,
            .sin_port = htons(port),
        };
        bindResult = bind(_socket, (struct sockaddr*)&serverA, sizeof(serverA));
    }
    if (bindResult == -1){
        close(_socket);
        throw std::runtime_error("Server: Failed to bind address to socket");
    }
    if (listen() == -1){
        close(_socket);
        throw std::runtime_error("Server: Failed to enter listening state");
    }
}

Server::~Server(){
    close(_socket);
}

void Server::start(){
    _client = accept(_socket, nullptr, nullptr);
}

bool Server::connected() const{
    return _client != -1;
}

void Server::send(void* message, size_t length) const{
    if (_client == -1){
        return;
    }
    ::send(_client, message, length, 0);
}

void Server::receive(size_t maxLength, void* buffer, const std::function<void(void*, long)>& callback) const{
    if (_client == -1){
        return;
    }
    long length = recv(_client, buffer, maxLength, 0);
    callback(buffer, length);
}

int Server::listen() const{
    return ::listen(_socket, SOMAXCONN);
}

Client::Client(bool ipv6) : _connected(false), _ipv6(ipv6){
    _socket = socket(ipv6? AF_INET6:AF_INET, SOCK_STREAM, 0);
    if (_socket == -1){
        throw std::runtime_error("Client: Failed to create socket");
    }
}

Client::~Client(){
    close(_socket);
}

void Client::connect(const char* address, int port){
    if (_ipv6){
        struct sockaddr_in6 serverA = {
            .sin6_len = sizeof(struct sockaddr_in6),
            .sin6_family = AF_INET6,
            .sin6_port = htons(port),
        };
        inet_pton(AF_INET6, address, &(serverA.sin6_addr));
        _connected = ::connect(_socket, (struct sockaddr*)&serverA, sizeof(serverA)) == 0;
    }else{
        struct sockaddr_in serverA = {
            .sin_len = sizeof(struct sockaddr_in),
            .sin_family = AF_INET,
            .sin_port = htons(port),
        };
        inet_pton(AF_INET, address, &(serverA.sin_addr));
        _connected = ::connect(_socket, (struct sockaddr*)&serverA, sizeof(serverA)) == 0;
    }
}

bool Client::connected() const{
    return _connected;
}

void Client::send(void* message, size_t length) const{
    ::send(_socket, message, length, 0);
}

void Client::receive(size_t maxLength, void* buffer, const std::function<void(void*, long)>& callback) const{
    long length = recv(_socket, buffer, maxLength, 0);
    callback(buffer, length);
}
