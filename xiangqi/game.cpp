//
//  game.cpp
//  xiangqi
//
//  Created by Colin Xie on 7/9/24.
//

#include "game.hpp"

/* RENDERING */
// constexpr static char _imgPath[] = "assets/xiangqi_board.png";
constexpr static char _fontPath[] = "assets/WeiBei.ttf";
constexpr static int _fontSize = 36;

constexpr static SDL_Color _boardSpaceColor{241, 203, 157, SDL_ALPHA_OPAQUE};
constexpr static SDL_Color _boardLineColor{75, 40, 20, SDL_ALPHA_OPAQUE};
constexpr static SDL_Color _borderColor{0, 0, 0, SDL_ALPHA_OPAQUE};
constexpr static SDL_Color _sidebarColor = _boardSpaceColor;
constexpr static SDL_Color _bottomBarColor = _boardSpaceColor;//{215, 205, 185, SDL_ALPHA_OPAQUE};
constexpr static SDL_Color _redPieceBorderColor{220, 30, 0, SDL_ALPHA_OPAQUE};
constexpr static SDL_Color _redPieceInnerColor{255, 240, 200, SDL_ALPHA_OPAQUE};
constexpr static SDL_Color _blackPieceBorderColor{20, 20, 20, SDL_ALPHA_OPAQUE};
constexpr static SDL_Color _blackPieceInnerColor{225, 215, 205, SDL_ALPHA_OPAQUE};
constexpr static SDL_Color _shadedColor{0, 0, 0, 50};
constexpr static SDL_Color _moveHighlightColor{255, 235, 0, SDL_ALPHA_OPAQUE};
constexpr static SDL_Color _moveAfterimageColor{255, 245, 120, 100};
constexpr static SDL_Color _checkHighlightColor{255, 150, 0, SDL_ALPHA_OPAQUE};
constexpr static SDL_Color _checkmateHighlightColor{170, 0, 0, SDL_ALPHA_OPAQUE};
constexpr static SDL_Color _connectingOverlayBGColor{0, 0, 0, 150};
constexpr static SDL_Color _connectingOverlayTextColor{250, 250, 250, SDL_ALPHA_OPAQUE};

constexpr static int _pieceRadius = 25;
constexpr static int _borderWidth = 2;
constexpr static int _moveCircleRadius = _pieceRadius;
constexpr static int _sidebarWidth = 180;
constexpr static int _bottomBarHeight = 80;
constexpr static int _textPos = 70;
constexpr static int _buttonWidth = 160;
constexpr static int _buttonHeight = 60;
constexpr static int _buttonMargin = (_bottomBarHeight-_buttonHeight) / 2;
constexpr static int _boxSize = 65;
constexpr static int _boardMargin = 35;
constexpr static int _boardWidth = _boxSize*8 + _boardMargin*2;
constexpr static int _boardHeight = _boxSize*9 + _boardMargin*2;
constexpr static int _screenWidth = _boardWidth+_sidebarWidth;
constexpr static int _screenHeight = _boardHeight+_bottomBarHeight;

Game::PixelPos::PixelPos() = default;

Game::PixelPos::PixelPos(int x, int y) : x(x), y(y){}

bool Game::PixelPos::operator==(PixelPos other) const{
    return x == other.x && y == other.y;
}

Game::Button::Button(SDL_Rect rect, const std::function<void()>& action, std::u16string_view text, SDL_Color borderColor, SDL_Color fillColor, int borderWidth, std::u16string_view confirmText, SDL_Color highlightColor) : _rect(rect), _action(action), _text(text), _borderColor(borderColor), _fillColor(fillColor), _selectedFill(highlightColor), _borderWidth(borderWidth), _confirmText(confirmText), _confirmState(false){}

// check if mouse is on button when clicking, if so then performs action
void Game::Button::click(PixelPos mousePos){
    if (mousePos.x >= _rect.x && mousePos.x <= _rect.x+_rect.w &&
        mousePos.y >= _rect.y && mousePos.y <= _rect.y+_rect.h){
        if (_confirmState || _confirmText.empty()){ // confirmed/doesn't need confirmation?
            _action();
            _confirmState = false;
        }else{
            _confirmState = true;
        }
    }else{
        _confirmState = false;
    }
}

void Game::Button::render(SDL_Renderer* renderer, TTF_Font* font) const{
    // border
    SDL_SetRenderDrawColor(renderer, _borderColor.r, _borderColor.g, _borderColor.b, _borderColor.a);
    SDL_RenderFillRect(renderer, &_rect);
    // inside
    SDL_Rect fillRect{_rect.x+_borderWidth, _rect.y+_borderWidth, _rect.w-_borderWidth*2, _rect.h-_borderWidth*2};
    SDL_SetRenderDrawColor(renderer, _fillColor.r, _fillColor.g, _fillColor.b, _fillColor.a);
    SDL_RenderFillRect(renderer, &fillRect);
    if (_confirmState){ // highlighted if on confirmation
        SDL_SetRenderDrawColor(renderer, _selectedFill.r, _selectedFill.g, _selectedFill.b, _selectedFill.a);
        SDL_RenderFillRect(renderer, &fillRect);
    }
    drawText(renderer, font, _confirmState? _confirmText.c_str():_text.c_str(), PixelPos{_rect.x + _rect.w/2, _rect.y + _rect.h/2}, _borderColor);
}

Game::Game(const char* address, int port, bool ipv6) : _window(nullptr), _renderer(nullptr), _address(address), _port(port), _online(port != -1){
    if (SDL_Init(SDL_INIT_VIDEO) != 0){
        std::string errorMessage("SDL could not initialize: ");
        errorMessage.append(SDL_GetError());
        throw std::runtime_error(errorMessage);
    }
    _window = SDL_CreateWindow("\u8C61\u68CB", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, _screenWidth, _screenHeight, SDL_WINDOW_SHOWN);
    if (_window == nullptr){
        std::string errorMessage("Window could not be initialized: ");
        errorMessage.append(SDL_GetError());
        throw std::runtime_error(errorMessage);
    }
    _renderer = SDL_CreateRenderer(_window, -1, 0);
    if (_renderer == nullptr){
        std::string errorMessage("Renderer could not be initialized: ");
        errorMessage.append(SDL_GetError());
        throw std::runtime_error(errorMessage);
    }
    SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND);
    /*
    _boardImg = IMG_LoadTexture(_renderer, _imgPath);
    if (_boardImg == nullptr){
        std::string errorMessage("Image could not be loaded: ");
        errorMessage.append(SDL_GetError());
        throw std::runtime_error(errorMessage);
    }
     */
    if (TTF_Init() != 0){
        std::string errorMessage("SDL_TTF could not initialize: ");
        errorMessage.append(TTF_GetError());
        throw std::runtime_error(errorMessage);
    }
    _font = TTF_OpenFont(_fontPath, _fontSize);
    if (_font == nullptr){
        std::string errorMessage("Font could not opened: ");
        errorMessage.append(TTF_GetError());
        throw std::runtime_error(errorMessage);
    }
    SDL_Rect resetButtonRect{_boardWidth + (_sidebarWidth-_buttonWidth)/2, _boardHeight + (_bottomBarHeight-_buttonHeight)/2, _buttonWidth, _buttonHeight};
    auto resetter = [this](){
        resetState();
        if (_online){
            _toSend.type = Message::Type::Restart;
            sendMessage();
        }
    };
    SDL_Rect switchButtonRect{resetButtonRect.x - _buttonWidth - _buttonMargin, _boardHeight + (_bottomBarHeight-_buttonHeight)/2, _buttonWidth, _buttonHeight};
    _buttons.emplace_back(resetButtonRect, resetter, u"重新開始", _borderColor, _boardSpaceColor, _borderWidth, u"确定?", _shadedColor);
    if (_online){
        // cannot switch sides in online play
        _buttons.emplace_back(switchButtonRect, [](){ return; }, u"換邊", _borderColor, _boardSpaceColor, _borderWidth, u"線上;停用", _shadedColor);
    }else{
        auto switcher = [this](){
            _playingAs = _playingAs == Side::Red? Side::Black:Side::Red;
        };
        _buttons.emplace_back(switchButtonRect, switcher, u"換邊", _borderColor, _boardSpaceColor, _borderWidth);
    }
    if (_online){
        if (_address == nullptr){
            _server.emplace(_port, ipv6);
            _playingAs = Side::Red;
        }else{
            _client.emplace(ipv6);
            _playingAs = Side::Black;
        }
        _connecter = std::async(std::launch::async, [this](){
            if (_server){
                _server->start();
            }else{
                while (!_client->connected()){
                    _client->connect(_address, _port);
                    std::this_thread::sleep_for(std::chrono::milliseconds{100}); // prevent busy loop
                }
            }
        });
    }else{
        _playingAs = Side::Red;
    }
    resetState();
}

Game::~Game(){
    SDL_DestroyWindow(_window);
    SDL_DestroyRenderer(_renderer);
    SDL_Quit();
    TTF_CloseFont(_font);
    TTF_Quit();
    if (_server){
        _server->~Server();
    }else{
        _client->~Client();
    }
    if (_connecter.valid()){
        _connecter.get();
    }
    if (_onlineEventHandler.valid()){
        _onlineEventHandler.get();
    }
}

void Game::run(){
    _quit = false;
    redraw();
    if (_online){
        drawConnectingOverlay();
        updateWindow();
        // online event handler in parallel thread
        _lastProcessed = true;
        _onlineEventHandler = std::async(std::launch::async, [this](){
            if (_connecter.valid()){
                _connecter.get(); // ensure connected to client/server before performing any socket operation
            }
            if (!_quit){
                threadsafeCall([this](){
                    redraw();
                    updateWindow();
                });
            }
            while (!_quit){
                std::this_thread::sleep_for(std::chrono::milliseconds{5}); // prevent busy loop
                if (_lastProcessed){
                    receiveMessage([this](void*, long){ _lastProcessed = false; });
                }else{
                    threadsafeCall([this](){
                        switch (_lastReceived.type){
                            case (Message::Type::Move):
                                if (_state.currentTurn() != _playingAs){
                                    Piece* pieceP = _state.findPiece(Position{_lastReceived.xFrom, _lastReceived.yFrom});
                                    updateLastMoved(pieceP);
                                    _state.performMove(pieceP, Position{_lastReceived.xTo, _lastReceived.yTo});
                                    _lastProcessed = true;
                                    redraw();
                                    updateWindow();
                                }
                                break;
                            case (Message::Type::Restart):
                                resetState();
                                _lastProcessed = true;
                                redraw();
                                updateWindow();
                                break;
                            case (Message::Type::Quit):
                                _quit = true;
                                break;
                        }
                    });
                }
            }
        });
    }else{
        updateWindow();
    }
    // main thread event handler
    SDL_Event event;
    while (!_quit){
        while (receiveEvent(event)){
            switch (event.type){
                case (SDL_QUIT):
                    _quit = true;
                    if (_online){
                        // tell other player to quit
                        threadsafeCall([this](){
                            _toSend.type = Message::Type::Quit;
                            sendMessage();
                        });
                    }
                    return;
                case (SDL_MOUSEBUTTONDOWN):
                    if (_online && !connected()){
                        break; // ignore if still connecting
                    }
                    int mouseX, mouseY;
                    SDL_GetMouseState(&mouseX, &mouseY);
                    threadsafeCall([this, mouseX, mouseY](){
                        select(mouseX, mouseY);
                        redraw();
                        updateWindow();
                    });
                    break;
            }
        }
        //std::this_thread::sleep_for(std::chrono::milliseconds{5}); // prevent busy loop
    }
}

bool Game::receiveEvent(SDL_Event& event){
    return SDL_WaitEvent(&event);
}

Game::PixelPos Game::toPixel(Position pos){
    if (_playingAs == Side::Black){
        pos.x = 8-pos.x;
        pos.y = 9-pos.y;
    }
    return PixelPos{_boardMargin + pos.x*_boxSize, _boardMargin + pos.y*_boxSize};
}

void Game::select(int mouseX, int mouseY){
    // mouse on a button?
    for (Button& button : _buttons){
        button.click(PixelPos{mouseX, mouseY});
    }
    PixelPos mousePos{mouseX, mouseY};
    // mouse on any move?
    for (Position move : _moves){
        if (withinDist(mousePos, toPixel(move), _moveCircleRadius)){
            // can only move if your turn
            if (!_state.checkmate() && _selectedPiece != nullptr && _selectedPiece->side == _state.currentTurn()){
                if (!_online){
                    updateLastMoved(_selectedPiece);
                    _state.performMove(_selectedPiece, move);
                }else if (_online && _selectedPiece->side == _playingAs){ // if online, can only move your own pieces
                    updateLastMoved(_selectedPiece);
                    _toSend.type = Message::Type::Move;
                    _toSend.xFrom = _selectedPiece->pos.x;
                    _toSend.yFrom = _selectedPiece->pos.y;
                    _toSend.xTo = move.x;
                    _toSend.yTo = move.y;
                    _state.performMove(_selectedPiece, move);
                    sendMessage();
                }
            }
            deselect();
            return;
        }
    }
    // mouse on any piece?
    for (Piece& piece : _state.pieces()){
        if (piece.captured){
            continue;
        }
        if (withinDist(mousePos, toPixel(piece.pos), _pieceRadius)){
            _selectedPiece = &piece;
            return;
        }
    }
    deselect();
}

/* RENDERING */
bool Game::withinDist(PixelPos a, PixelPos b, double dist){
    return ((a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y)) <= dist*dist;
}

void Game::redraw(){
    refreshBoard();
    int redCaptures = 0, blackCaptures = 0; // # pieces captured by each side
    for (Piece piece : _state.pieces()){
        bool selected = _selectedPiece != nullptr && *_selectedPiece == piece;
        bool wasMoved = _lastMovedPiece != nullptr && *_lastMovedPiece == piece;
        if (piece.captured){
            // draw piece in sidebar instead
            PixelPos pPos;
            if (piece.side == Side::Red){ // red piece captured by black
                pPos.x = _boardWidth + _sidebarWidth/6 + (redCaptures/5) * _sidebarWidth/3;
                pPos.y = (_playingAs == Side::Red? 0 : _boardHeight/2) + _boardHeight/20 + (redCaptures%5) * _boardHeight/10;
                ++blackCaptures;
            }else{
                pPos.x = _boardWidth + _sidebarWidth/6 + (redCaptures/5) * _sidebarWidth/3;
                pPos.y = (_playingAs == Side::Red? _boardHeight/2 : 0) + _boardHeight/20 + (redCaptures%5) * _boardHeight/10;
                ++redCaptures;
            }
            drawPiece(piece, pPos);
        }else{
            drawPiece(piece);
        }
        if (selected){
            // shade it in
            drawCircle(toPixel(piece.pos), _pieceRadius, _shadedColor);
        }
        if (wasMoved){
            // draw a border around it
            drawBorder(toPixel(piece.pos), _pieceRadius+_borderWidth+1, _borderWidth, _moveHighlightColor);
        }
    }
    if (_selectedPiece != nullptr){ // a piece is selected
        // draw valid moves
        _moves = _state.getMoves(_selectedPiece);
        for (Position move : _moves){
            drawCircle(toPixel(move), _moveCircleRadius, _shadedColor);
        }
    }
    if (_lastMovedPiece != nullptr){ // a move was just made
        // draw afterimage of last move made
        drawCircle(toPixel(_lastMovedFrom), _pieceRadius, _moveAfterimageColor);
    }
    if (_state.check()){ // .check is true if in check or checkmate
        // draw highlight on enemy shuai
        Position oppShuaiPos = _state.pieces()[_state.checking() == Side::Red? 1:0].pos;
        if (_state.checkmate()){
            drawBorder(toPixel(oppShuaiPos), _pieceRadius+_borderWidth, _borderWidth, _checkmateHighlightColor);
        }else{
            drawBorder(toPixel(oppShuaiPos), _pieceRadius+_borderWidth, _borderWidth, _checkHighlightColor);
        }
    }
}

void Game::drawCircle(PixelPos centerPos, int radius, SDL_Color color, bool antiAliasing, double antiAliasingThickness){
    for (int w=0; w<=radius*2; ++w){
        for (int h=0; h<=radius*2; ++h){
            PixelPos offset{radius-w, radius-h};
            PixelPos pPos{centerPos.x+offset.x, centerPos.y+offset.y};
            if (antiAliasing && withinDist(pPos, centerPos, radius+antiAliasingThickness) && !withinDist(pPos, centerPos, radius)){ // outer anti-aliasing by transparency
                SDL_SetRenderDrawColor(_renderer, color.r, color.g, color.b, color.a/2);
                SDL_RenderDrawPoint(_renderer, pPos.x, pPos.y);
            }else if (withinDist(pPos, centerPos, radius)){ // within the circle
                SDL_SetRenderDrawColor(_renderer, color.r, color.g, color.b, color.a);
                SDL_RenderDrawPoint(_renderer, pPos.x, pPos.y);
            }
        }
    }
}

void Game::drawBorder(PixelPos centerPos, int radius, int thickness, SDL_Color color, bool antiAliasing, double antiAliasingThickness){
    for (int w=0; w<=radius*2; ++w){
        for (int h=0; h<=radius*2; ++h){
            PixelPos offset{radius-w, radius-h};
            PixelPos pPos{centerPos.x+offset.x, centerPos.y+offset.y};
            if (antiAliasing && withinDist(pPos, centerPos, radius+antiAliasingThickness) && !withinDist(pPos, centerPos, radius)){ // outer anti-aliasing by transparency
                SDL_SetRenderDrawColor(_renderer, color.r, color.g, color.b, color.a/2);
                SDL_RenderDrawPoint(_renderer, pPos.x, pPos.y);
            }else if (antiAliasing && withinDist(pPos, centerPos, radius-thickness+1) && !withinDist(pPos, centerPos, radius-thickness-antiAliasingThickness+1)){ // inner anti-aliasing
                SDL_SetRenderDrawColor(_renderer, color.r, color.g, color.b, color.a/2);
                SDL_RenderDrawPoint(_renderer, pPos.x, pPos.y);
            }else if (withinDist(pPos, centerPos, radius) && !withinDist(pPos, centerPos, radius-thickness)){ // on the border
                SDL_SetRenderDrawColor(_renderer, color.r, color.g, color.b, color.a);
                SDL_RenderDrawPoint(_renderer, pPos.x, pPos.y);
            }
        }
    }
}

void Game::drawText(SDL_Renderer* renderer, TTF_Font* font, const char16_t* text, PixelPos pPos, SDL_Color color){
    SDL_Surface* sur = TTF_RenderUNICODE_Blended(font, (Uint16*)text, color);
    SDL_Rect textRect{pPos.x-(sur->w)/2, pPos.y-(sur->h)/2, sur->w, sur->h}; // centered on pPos
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, sur);
    SDL_RenderCopy(renderer, tex, nullptr, &textRect);
    SDL_FreeSurface(sur);
    SDL_DestroyTexture(tex);
}

void Game::drawPiece(Piece piece, PixelPos specifyPos){
    PixelPos pPos = specifyPos == PixelPos{-1, -1}? toPixel(piece.pos):specifyPos;
    SDL_Color borderColor = piece.side == Side::Red? _redPieceBorderColor:_blackPieceBorderColor;
    SDL_Color innerColor = piece.side == Side::Red? _redPieceInnerColor:_blackPieceInnerColor;
    // inner circle
    drawCircle(pPos, _pieceRadius-_borderWidth, innerColor);
    // outer border
    drawBorder(pPos, _pieceRadius, _borderWidth, borderColor);
    // draw chinese character inside
    const static std::map<PieceType, char16_t> redCharacters{
        {PieceType::Shuai, u'帥'},
        {PieceType::Shi, u'士'},
        {PieceType::Xiang, u'相'},
        {PieceType::Ma, u'馬'},
        {PieceType::Ju, u'車'},
        {PieceType::Pao, u'炮'},
        {PieceType::Bing, u'兵'},
    };
    const static std::map<PieceType, char16_t> blackCharacters{
        {PieceType::Shuai, u'將'},
        {PieceType::Shi, u'仕'},
        {PieceType::Xiang, u'象'},
        {PieceType::Ma, u'馬'},
        {PieceType::Ju, u'車'},
        {PieceType::Pao, u'砲'},
        {PieceType::Bing, u'卒'},
    };
    if (piece.side == Side::Red){
        drawText(&redCharacters.at(piece.type), pPos, _redPieceBorderColor);
    }else{
        drawText(&blackCharacters.at(piece.type), pPos, _blackPieceBorderColor);
    }
}

void Game::refreshBoard(){
    SDL_RenderClear(_renderer);
    // draw board
    SDL_SetRenderDrawColor(_renderer, _boardSpaceColor.r, _boardSpaceColor.g, _boardSpaceColor.b, _boardSpaceColor.a);
    SDL_Rect boardRect{0, 0, _boardWidth, _boardHeight};
    SDL_RenderFillRect(_renderer, &boardRect);
    /*
    SDL_Rect imgRect{_imgMargin, _imgMargin, _imgWidth, _imgHeight};
    SDL_RenderCopy(_renderer, _boardImg, nullptr, &imgRect);
     */
    SDL_SetRenderDrawColor(_renderer, _boardLineColor.r, _boardLineColor.g, _boardLineColor.b, _boardLineColor.a);
    // horizontal grid lines
    for (int i=0; i<10; ++i){
        SDL_RenderDrawLine(_renderer, _boardMargin, _boardMargin+_boxSize*i, _boardMargin+_boxSize*8, _boardMargin+_boxSize*i);
    }
    // vertical grid lines
    SDL_RenderDrawLine(_renderer, _boardMargin, _boardMargin, _boardMargin, _boardMargin+_boxSize*9);
    SDL_RenderDrawLine(_renderer, _boardMargin+_boxSize*8, _boardMargin, _boardMargin+_boxSize*8, _boardMargin+_boxSize*9);
    for (int i=1; i<8; ++i){
        SDL_RenderDrawLine(_renderer, _boardMargin+_boxSize*i, _boardMargin, _boardMargin+_boxSize*i, _boardMargin+_boxSize*4);
        SDL_RenderDrawLine(_renderer, _boardMargin+_boxSize*i, _boardMargin+_boxSize*5, _boardMargin+_boxSize*i, _boardMargin+_boxSize*9);
    }
    // diagonal lines
    SDL_RenderDrawLine(_renderer, _boardMargin+_boxSize*3, _boardMargin, _boardMargin+_boxSize*5, _boardMargin+_boxSize*2);
    SDL_RenderDrawLine(_renderer, _boardMargin+_boxSize*5, _boardMargin, _boardMargin+_boxSize*3, _boardMargin+_boxSize*2);
    SDL_RenderDrawLine(_renderer, _boardMargin+_boxSize*3, _boardMargin+_boxSize*7, _boardMargin+_boxSize*5, _boardMargin+_boxSize*9);
    SDL_RenderDrawLine(_renderer, _boardMargin+_boxSize*5, _boardMargin+_boxSize*7, _boardMargin+_boxSize*3, _boardMargin+_boxSize*9);
    // draw sidebar
    SDL_SetRenderDrawColor(_renderer, _sidebarColor.r, _sidebarColor.g, _sidebarColor.b, _sidebarColor.a);
    SDL_Rect sidebarRect{_boardWidth, 0, _sidebarWidth, _boardHeight};
    SDL_RenderFillRect(_renderer, &sidebarRect);
    SDL_SetRenderDrawColor(_renderer, _borderColor.r, _borderColor.g, _borderColor.b, _borderColor.a);
    SDL_Rect sideBorderRect{_boardWidth-_borderWidth/2, 0, _borderWidth, _boardHeight};
    SDL_RenderFillRect(_renderer, &sideBorderRect);
    SDL_Rect middleBorderRect{_boardWidth, _boardHeight/2, _sidebarWidth, _borderWidth};
    SDL_RenderFillRect(_renderer, &middleBorderRect);
    // draw bottom bar
    SDL_SetRenderDrawColor(_renderer, _bottomBarColor.r, _bottomBarColor.g, _bottomBarColor.b, _bottomBarColor.a);
    SDL_Rect bottomBarRect{0, _boardHeight, _boardWidth+_sidebarWidth, _bottomBarHeight};
    SDL_RenderFillRect(_renderer, &bottomBarRect);
    SDL_SetRenderDrawColor(_renderer, _borderColor.r, _borderColor.g, _borderColor.b, _borderColor.a);
    SDL_Rect bottomBorderRect{0, _boardHeight, _boardWidth+_sidebarWidth, _borderWidth};
    SDL_RenderFillRect(_renderer, &bottomBorderRect);
    if (_state.checkmate() && _state.checking() == Side::Red){
        drawText(u"紅方贏", PixelPos{_textPos, _boardHeight+_bottomBarHeight/2}, _redPieceBorderColor);
    }else if (_state.checkmate() && _state.checking() == Side::Black){
        drawText(u"黑方贏", PixelPos{_textPos, _boardHeight+_bottomBarHeight/2}, _blackPieceBorderColor);
    }else if (_state.currentTurn() == Side::Red){
        drawText(u"紅方走", PixelPos{_textPos, _boardHeight+_bottomBarHeight/2}, _redPieceBorderColor);
    }else{
        drawText(u"黑方走", PixelPos{_textPos, _boardHeight+_bottomBarHeight/2}, _blackPieceBorderColor);
    }
    // draw buttons
    for (const Button& button : _buttons){
        button.render(_renderer, _font);
    }
}

void Game::drawConnectingOverlay(){
    SDL_SetRenderDrawColor(_renderer, _connectingOverlayBGColor.r, _connectingOverlayBGColor.g, _connectingOverlayBGColor.b, _connectingOverlayBGColor.a);
    SDL_Rect screenRect{0, 0, _screenWidth, _screenHeight};
    SDL_RenderFillRect(_renderer, &screenRect);
    if (_server){
        drawText(u"正在等待...", PixelPos{_screenWidth/2, _screenHeight/2}, _connectingOverlayTextColor);
    }else{
        drawText(u"正在聯繫...", PixelPos{_screenWidth/2, _screenHeight/2}, _connectingOverlayTextColor);
    }
}

void Game::updateWindow(){
    SDL_RenderPresent(_renderer);
}

/* ONLINE */
template<typename ThreadUnsafeCallable>
inline void Game::threadsafeCall(ThreadUnsafeCallable&& callable){
    const std::lock_guard<std::mutex> lock(_mutex);
    callable();
}

bool Game::connected(){
    if (!_online){
        throw std::runtime_error("Called online function from offline");
    }
    return _server? _server->connected():_client->connected();
}

void Game::sendMessage(){
    if (!_online){
        throw std::runtime_error("Called online function from offline");
    }
    if (_server){
        _server->send(&_toSend, sizeof(Message));
    }else{
        _client->send(&_toSend, sizeof(Message));
    }
}

void Game::receiveMessage(const std::function<void(void*, long)>& callback){
    if (!_online){
        throw std::runtime_error("Called online function from offline");
    }
    if (_server){
        _server->receive(sizeof(Message), &_lastReceived, callback);
    }else{
        _client->receive(sizeof(Message), &_lastReceived, callback);
    }
}

/* GAME LOGIC */
void Game::deselect(){
    _selectedPiece = nullptr;
    _moves.clear();
}

void Game::updateLastMoved(Piece* movedPiece){
    _lastMovedPiece = movedPiece;
    _lastMovedFrom = movedPiece->pos;
}

void Game::resetState(){
    _state.reset();
    _selectedPiece = nullptr;
    _moves.clear();
    _lastMovedPiece = nullptr;
}
