//
//  main.cpp
//  xiangqi
//
//  Created by Colin Xie on 7/9/24.
//
#include <iostream>

#include "game.hpp"

/*
void testSockets(){
    struct Test{
        int a{};
        double b{};
        bool c{};
    };
    
    std::thread serverProcess;
    {
        serverProcess = std::thread([]{
            Server server(50000);
            server.start();
            
            std::atomic<bool> received = false;
            while (!received){
                Test receivedData;
                server.receive(sizeof(Test), &receivedData, [&received](void* buffer, int length){
                    if (length == sizeof(Test)){
                        Test& receivedData = * reinterpret_cast<Test*>(buffer);
                        std::cout << "Server received: " << receivedData.a << receivedData.b << receivedData.c << " (" << length << " bytes)\n";
                        received = true;
                    }else if (length == 0){
                        std::cout << "Client disconnected from server\n";
                    }else if (length == -1){
                        std::cout << "Socket error\n";
                    }else{
                        assert(false);
                    }
                });
            }
        });
    }

    {
        Client client;
        client.connect("0:0:0:0:0:0:0:1", 50000); // localhost
        if (client.connected()){
            Test test{0, 1, true};
            std::cout << "Client sending:  " << test.a << test.b << test.c << " (" << sizeof(Test) << " bytes)\n";
            client.send(&test, sizeof(Test));
        }
    }
    
    if (serverProcess.joinable()){
        serverProcess.join();
    }
}
*/

// ./main [port] [is_IPv6] [IP_address]
int main(int argc, const char* argv[]) {
    //testSockets(); return 0;
    if (argc < 2){ // offline ver.
        Game game;
        game.run();
    }else if (argc < 4){ // self host server
        int port = std::atoi(argv[1]);
        bool ipv6 = (strcmp(argv[2], "true") == 0);
        Game game(nullptr, port, ipv6);
        game.run();
    }else if (argc == 4){ // connect to server at given IP adress
        int port = std::atoi(argv[1]);
        bool ipv6 = (strcmp(argv[2], "true") == 0);
        Game game(argv[3], port, ipv6);
        game.run();
    }else{
        throw std::runtime_error("Expected 4 arguments to run as client");
    }
    return 0;
}
