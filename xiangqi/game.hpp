//
//  game.hpp
//  xiangqi
//
//  Created by Colin Xie on 7/9/24.
//

#pragma once

#include <string>
#include <vector>
#include <map>
#include <stack>
#include <functional>
#include <optional>
#include <future>
#include <atomic>
#include <mutex>
#include <stdexcept>
#include <cassert>
#include <cstdlib>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "game_logic.hpp"
#include "socket.hpp"

class Game{
    /* RENDERING */
    SDL_Window* _window;
    SDL_Renderer* _renderer;
    // SDL_Texture* _boardImg;
    TTF_Font* _font;
    
    struct PixelPos{
        int x, y;
        
        PixelPos();
        
        PixelPos(int x, int y);
        
        bool operator==(PixelPos other) const;
    };
    
    class Button{
        SDL_Rect _rect;
        std::u16string _text;
        std::function<void()> _action;
        
        std::u16string _confirmText;
        bool _confirmState;
        
        SDL_Color _borderColor, _fillColor, _selectedFill;
        int _borderWidth;
        
    public:
        Button(SDL_Rect rect, const std::function<void()>& action, std::u16string_view text, SDL_Color borderColor, SDL_Color fillColor, int borderWidth, std::u16string_view confirmText = {}, SDL_Color highlightColor = {0, 0, 0, 0});
        
        // check if mouse is on button when clicking, if so then performs action
        void click(PixelPos mousePos);
        
        void render(SDL_Renderer* renderer, TTF_Font* font) const;
    };
    
    std::vector<Button> _buttons;
    
    /* ONLINE */
    /*
     protocol:
     server plays as red, client plays as black
     (0, 0) is the top left of the board (should be one of black's ju)
     message format: 5 bytes
     - 0: message type (0: Move, 1: Restart, 2: Quit)
     - 1: x of position to move from
     - 2: y of position to move from
     - 3: x of position to move to
     - 4: y of position to move to
     message types
     - Move: move piece at source position to destination position
     - Restart: reset all pieces and state to original (ignores bytes 1-4)
     - Quit: exit game (ignores bytes 1-4)
     Move should be processed once it is the turn of the side that sent it, verifying the legality of the Move is optional
     Restart and Quit should be processed immediately upon receiving
     when a move results in checkmate, no Restart or Quit is sent automatically
     */
    bool _online;
    std::atomic<bool> _quit;
    std::optional<Server> _server;
    std::optional<Client> _client;
    std::future<void> _connecter;
    const char* _address; // IP
    int _port;
    std::future<void> _onlineEventHandler;
    
    struct Message{
        enum class Type : uint8_t{
            Move = 0,
            Restart = 1,
            Quit = 2,
        };
        
        Type type;
        uint8_t xFrom, yFrom;
        uint8_t xTo, yTo;
    };
    
    Message _toSend, _lastReceived;
    std::atomic<bool> _lastProcessed;
    
    /* GAME LOGIC */
    GameState _state;
    // related to how player interacts with game
    Side _playingAs;
    Piece* _selectedPiece; // in state._pieces
    std::vector<Position> _moves; // possible moves of selected piece
    Piece* _lastMovedPiece; // in state._pieces
    Position _lastMovedFrom;
    
    std::mutex _mutex;
    
public:
    Game(const char* address = nullptr, int port = -1, bool ipv6 = false);
    
    ~Game();
    
    void run();
    
private:
    bool receiveEvent(SDL_Event& event);
    
    PixelPos toPixel(Position pos);
    
    void select(int mouseX, int mouseY);
    
    /* RENDERING */
    bool withinDist(PixelPos a, PixelPos b, double dist);
    
    void redraw();
    
    void drawCircle(PixelPos centerPos, int radius, SDL_Color color, bool antiAliasing = true, double antiAliasingThickness = 1.5);
    
    void drawBorder(PixelPos centerPos, int radius, int thickness, SDL_Color color, bool antiAliasing = true, double antiAliasingThickness = 1.5);
    
    void drawText(const char16_t* text, PixelPos pPos, SDL_Color color){
        drawText(_renderer, _font, text, pPos, color);
    }
    
    static void drawText(SDL_Renderer* renderer, TTF_Font* font, const char16_t* text, PixelPos pPos, SDL_Color color);
    
    void drawPiece(Piece piece, PixelPos specifyPos = {-1, -1});
    
    // clears pieces leaving only board and other guis
    void refreshBoard();
    
    // loading screen for when waiting on server-client connection
    void drawConnectingOverlay();
    
    void updateWindow();
    
    /* ONLINE */
    // if function is thread-unsafe, must be wrapped by threadsafeCall for thread safety
    template<typename ThreadUnsafeCallable>
    inline void threadsafeCall(ThreadUnsafeCallable&& callable);
    
    bool connected();
    
    void sendMessage();

    void receiveMessage(const std::function<void(void*, long)>& callback);
    
    /* GAME LOGIC */
    void deselect();
    
    void updateLastMoved(Piece* movedPiece);
    
    void resetState();
};
