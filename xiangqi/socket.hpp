//
//  socket.hpp
//  xiangqi
//
//  Created by Colin Xie on 7/9/24.
//

#pragma once

#include <atomic>
#include <functional>

class Server{
    int _socket;
    int _client; // -1 if not connected

public:
    Server(const Server&) = delete;
    
    Server(int port, bool ipv6 = false);
    
    ~Server();
    
    Server& operator=(const Server&) = delete;
    
    void start();
    
    bool connected() const;
    
    void send(void* message, size_t length) const;
    
    void receive(size_t maxLength, void* buffer, const std::function<void(void*, long)>& callback) const;
    
private:
    int listen() const;
};

class Client{
    int _socket;
    bool _connected;
    bool _ipv6;
    
public:
    Client(const Client&) = delete;
    
    Client(bool ipv6 = false);
    
    ~Client();
    
    Client& operator=(const Client&) = delete;
    
    void connect(const char* address, int port);
    
    bool connected() const;
    
    void send(void* message, size_t length) const;
    
    void receive(size_t maxLength, void* buffer, const std::function<void(void*, long)>& callback) const;
};
