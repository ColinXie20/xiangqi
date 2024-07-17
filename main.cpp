//
//  main.cpp
//  xiangqi
//
//  Created by Colin Xie on 7/9/24.
//

#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <map>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

class Game{
    SDL_Window* _window;
    SDL_Renderer* _renderer;
    SDL_Texture* _boardImg;
    TTF_Font* _font;
    
    constexpr static char _imgPath[] = "/Users/colinxie/Documents/C++/xiangqi/xiangqi/xiangqi_board.png";
    constexpr static char _fontPath[] = "/Users/colinxie/Documents/C++/xiangqi/xiangqi/NotoSerifSC-Bold.ttf";
    constexpr static int _fontSize = 30;
    
    constexpr static SDL_Color _boardSpaceColor{241, 203, 157, SDL_ALPHA_OPAQUE};
    constexpr static SDL_Color _borderColor{0, 0, 0, SDL_ALPHA_OPAQUE};
    constexpr static SDL_Color _sidebarColor = _boardSpaceColor;
    constexpr static SDL_Color _bottomBarColor = _boardSpaceColor;//{215, 205, 185, SDL_ALPHA_OPAQUE};
    constexpr static SDL_Color _redPieceBorderColor{220, 30, 0, SDL_ALPHA_OPAQUE};
    constexpr static SDL_Color _redPieceInnerColor{255, 240, 200, SDL_ALPHA_OPAQUE};
    constexpr static SDL_Color _blackPieceBorderColor{20, 20, 20, SDL_ALPHA_OPAQUE};
    constexpr static SDL_Color _blackPieceInnerColor{220, 210, 200, SDL_ALPHA_OPAQUE};
    constexpr static SDL_Color _selectedPieceInnerColor{250, 250, 250, SDL_ALPHA_OPAQUE};
    constexpr static SDL_Color _highlightColor{50, 50, 50, 75};
    constexpr static SDL_Color _winTextFlashColor{250, 250, 250,
        SDL_ALPHA_OPAQUE};
    
    constexpr static int _pieceRadius = 25;
    constexpr static int _borderWidth = 3;
    constexpr static int _moveCircleRadius = 25;
    constexpr static int _boardWidth = 571; // max 2560
    constexpr static int _boardHeight = 640; // max 1440
    constexpr static int _sidebarWidth = 180;
    constexpr static int _bottomBarHeight = 80;
    constexpr static int _resetButtonWidth = 160;
    constexpr static int _resetButtonHeight = 60;
    constexpr static int _screenWidth = _boardWidth+_sidebarWidth;
    constexpr static int _screenHeight = _boardHeight+_bottomBarHeight;
    constexpr static int _boxWidth = 63;
    constexpr static int _boxHeight = 65;
    constexpr static int _marginLeft = 32; // must be > piece radius
    constexpr static int _marginTop = 29; // must be > piece radius
    constexpr static int _winTextFlashRate = 3; // # of refreshes between each flash
    constexpr static int _winTextFlashPeriod = _winTextFlashRate*10; // # of refreshes to flash for
    
    bool _resetButtonState = false; // true if already pressed once
    
    struct Position{
        int x, y;
        
        bool operator==(Position other){
            return x == other.x && y == other.y;
        }
    };
    
    enum class PieceType{
        Shuai,
        Shi,
        Xiang,
        Ma,
        Ju,
        Pao,
        Bing,
        Qiang,
    };

    enum class Side{
        Red,
        Black,
    };

    struct Piece{
        Position pos;
        PieceType type;
        Side side;
        bool captured;
        
        bool operator==(Piece other) const{
            return pos == other.pos && type == other.type && side == other.side && captured == other.captured;
        }
    };
    
    std::vector<Piece> _pieces;
    Piece* _selectedPiece; // in _pieces
    std::vector<Position> _moves; // possible moves of selected piece
    Side _currentTurn;
    
    bool _gameOver;
    Side _winner;
    
    struct PixelPos{
        int x, y;
        
        PixelPos(int x, int y) : x(x), y(y){}
        
        PixelPos(Position pos) : x(_marginLeft + pos.x*_boxWidth), y(_marginTop + pos.y*_boxHeight){}
        
        bool operator==(PixelPos other) const{
            return x == other.x && y == other.y;
        }
    };
    
    class TempState{
        Game* _game;
        std::vector<Piece> _prevState;
        
    public:
        TempState(Game* gameP, Piece* pieceP, Position move) : _game(gameP){
            _prevState = _game->_pieces;
            _game->performMove(pieceP, move, true);
        }
        
        ~TempState(){
            _game->_pieces = _prevState;
        }
    };
    
    class Button{
        SDL_Rect _rect;
        std::u16string _text;
        std::function<void()> _action;
        
        std::u16string _confirmText;
        bool _confirmState;
        
        SDL_Color _borderColor, _fillColor, _highlightColor;
        int _borderWidth;
        
    public:
        Button(SDL_Rect rect, std::function<void()> action, std::u16string_view text, SDL_Color borderColor, SDL_Color fillColor, SDL_Color highlightColor, int borderWidth, std::u16string_view confirmText = {}) : _rect(rect), _action(action), _text(text), _borderColor(borderColor), _fillColor(fillColor), _highlightColor(highlightColor), _borderWidth(borderWidth), _confirmText(confirmText), _confirmState(false){}
        
        // check if mouse is on button when clicking, if so then performs action
        void click(PixelPos mousePos){
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
        
        void render(SDL_Renderer* renderer, TTF_Font* font) const{
            // border
            SDL_SetRenderDrawColor(renderer, _borderColor.r, _borderColor.g, _borderColor.b, _borderColor.a);
            SDL_RenderFillRect(renderer, &_rect);
            // inside
            SDL_Rect fillRect{_rect.x+_borderWidth, _rect.y+_borderWidth, _rect.w-_borderWidth*2, _rect.h-_borderWidth*2};
            SDL_SetRenderDrawColor(renderer, _fillColor.r, _fillColor.g, _fillColor.b, _fillColor.a);
            SDL_RenderFillRect(renderer, &fillRect);
            if (_confirmState){ // highlighted if on confirmation
                SDL_SetRenderDrawColor(renderer, _highlightColor.r, _highlightColor.g, _highlightColor.b, _highlightColor.a);
                SDL_RenderFillRect(renderer, &fillRect);
            }
            drawText(renderer, font, _confirmState? _confirmText.c_str():_text.c_str(), PixelPos{_rect.x + _rect.w/2, _rect.y + _rect.h/2}, _borderColor);
        }
    };
    
    std::vector<Button> _buttons;
    
    constexpr static Piece _pieceInitialSetup[32]{
        Piece{Position{4, 9}, PieceType::Shuai, Side::Red, false},
        Piece{Position{4, 0}, PieceType::Shuai, Side::Black, false},
        Piece{Position{0, 9}, PieceType::Ju, Side::Red, false},
        Piece{Position{8, 9}, PieceType::Ju, Side::Red, false},
        Piece{Position{0, 0}, PieceType::Ju, Side::Black, false},
        Piece{Position{8, 0}, PieceType::Ju, Side::Black, false},
        Piece{Position{1, 7}, PieceType::Pao, Side::Red, false},
        Piece{Position{7, 7}, PieceType::Pao, Side::Red, false},
        Piece{Position{1, 2}, PieceType::Pao, Side::Black, false},
        Piece{Position{7, 2}, PieceType::Pao, Side::Black, false},
        Piece{Position{1, 9}, PieceType::Ma, Side::Red, false},
        Piece{Position{7, 9}, PieceType::Ma, Side::Red, false},
        Piece{Position{1, 0}, PieceType::Ma, Side::Black, false},
        Piece{Position{7, 0}, PieceType::Ma, Side::Black, false},
        Piece{Position{2, 9}, PieceType::Xiang, Side::Red, false},
        Piece{Position{6, 9}, PieceType::Xiang, Side::Red, false},
        Piece{Position{2, 0}, PieceType::Xiang, Side::Black, false},
        Piece{Position{6, 0}, PieceType::Xiang, Side::Black, false},
        Piece{Position{3, 9}, PieceType::Shi, Side::Red, false},
        Piece{Position{5, 9}, PieceType::Shi, Side::Red, false},
        Piece{Position{3, 0}, PieceType::Shi, Side::Black, false},
        Piece{Position{5, 0}, PieceType::Shi, Side::Black, false},
        Piece{Position{0, 6}, PieceType::Bing, Side::Red, false},
        Piece{Position{2, 6}, PieceType::Bing, Side::Red, false},
        Piece{Position{4, 6}, PieceType::Bing, Side::Red, false},
        Piece{Position{6, 6}, PieceType::Bing, Side::Red, false},
        Piece{Position{8, 6}, PieceType::Bing, Side::Red, false},
        Piece{Position{0, 3}, PieceType::Bing, Side::Black, false},
        Piece{Position{2, 3}, PieceType::Bing, Side::Black, false},
        Piece{Position{4, 3}, PieceType::Bing, Side::Black, false},
        Piece{Position{6, 3}, PieceType::Bing, Side::Black, false},
        Piece{Position{8, 3}, PieceType::Bing, Side::Black, false},
    };
    
public:
    Game() : _window(nullptr), _renderer(nullptr), _selectedPiece(nullptr), _currentTurn(Side::Red), _gameOver(false){
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
        _boardImg = IMG_LoadTexture(_renderer, _imgPath);
        if (_boardImg == nullptr){
            std::string errorMessage("Image could not be loaded: ");
            errorMessage.append(SDL_GetError());
            throw std::runtime_error(errorMessage);
        }
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
        resetState();
        SDL_Rect resetButtonRect{_boardWidth+(_sidebarWidth-_resetButtonWidth)/2, _boardHeight+(_bottomBarHeight-_resetButtonHeight)/2, _resetButtonWidth, _resetButtonHeight};
        _buttons.emplace_back(resetButtonRect, [this](){ resetState(); }, u"重新开始", _borderColor, _boardSpaceColor, _highlightColor, _borderWidth, u"确定?");
    }
    
    ~Game(){
        SDL_DestroyWindow(_window);
        SDL_DestroyRenderer(_renderer);
        SDL_Quit();
        TTF_CloseFont(_font);
        TTF_Quit();
    }
    
    void redraw(bool flash = false){
        refreshBoard(flash);
        int redCaptures = 0, blackCaptures = 0; // # pieces captured by each side
        for (Piece piece : _pieces){
            if (piece.captured){ // if captured draw in sidebar instead
                if (piece.side == Side::Red){ // red piece captured by black
                    piece.pos.x = 9 + blackCaptures/5;
                    piece.pos.y = blackCaptures%5;
                    ++blackCaptures;
                }else{
                    piece.pos.x = 9 + redCaptures/5;
                    piece.pos.y = 5 + redCaptures%5;
                    ++redCaptures;
                }
            }
            drawPiece(piece);
        }
        if (_selectedPiece != nullptr){ // a piece is selected
            // draw valid moves
            _moves = getMoves(_selectedPiece);
            for (Position move : _moves){
                drawCircle(move, _moveCircleRadius, _highlightColor);
            }
        }
        updateWindow();
    }
    
    bool receiveEvent(SDL_Event& event){
        return SDL_PollEvent(&event);
    }
    
    void select(int mouseX, int mouseY){
        // mouse on reset button?
        for (Button& button : _buttons){
            button.click(PixelPos{mouseX, mouseY});
        }
        PixelPos mousePos{mouseX, mouseY};
        // mouse on any move?
        for (Position move : _moves){
            PixelPos pPos(move);
            if (withinDist(mousePos, pPos, _moveCircleRadius)){
                // can only move if your turn
                if (_selectedPiece != nullptr && _selectedPiece->side == _currentTurn){
                    performMove(_selectedPiece, move);
                    switchTurns();
                }
                deselect();
                return;
            }
        }
        // mouse on any piece?
        for (Piece& piece : _pieces){
            if (piece.captured){
                continue;
            }
            PixelPos pPos(piece.pos);
            if (withinDist(mousePos, pPos, _pieceRadius)){
                _selectedPiece = &piece;
                return;
            }
        }
        deselect();
    }
    
    void run(){
        redraw();
        // event handler
        SDL_Event event;
        int flashCounter = 0;
        while (true){
            while (receiveEvent(event)){
                switch (event.type){
                    case (SDL_QUIT):
                        return;
                    case (SDL_MOUSEBUTTONDOWN):
                        int mouseX, mouseY;
                        SDL_GetMouseState(&mouseX, &mouseY);
                        select(mouseX, mouseY);
                        redraw();
                        break;
                }
            }
            if (_gameOver){ // victory text flashing effect
                if (flashCounter < _winTextFlashPeriod && flashCounter%_winTextFlashRate == 0){
                    redraw(flashCounter % (_winTextFlashRate*2) == 0);
                }
                ++flashCounter;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds{5}); //prevent busy loop
        }
    }
    
private:
    void deselect(){
        _selectedPiece = nullptr;
        _moves.clear();
    }
    
    void switchTurns(){
        _currentTurn = _currentTurn == Side::Red? Side::Black:Side::Red;
    }
    
    bool withinDist(PixelPos a, PixelPos b, int dist){
        return ((a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y)) <= dist*dist;
    }
    
    Piece* findPiece(Position pos){
        for (Piece& piece : _pieces){
            if (piece.pos == pos && !piece.captured){
                return &piece;
            }
        }
        return nullptr;
    }
    
    // moving a piece would cause line of sight?
    bool lineOfSight(Piece* pieceP, Position move){
        TempState temp(this, pieceP, move);
        if (_pieces[0].pos.x != _pieces[1].pos.x){
            return false;
        }
        // check for pieces between
        for (int i=2; i<_pieces.size(); ++i){
            if (_pieces[i].captured || (_pieces[i].pos == move && &_pieces[i] != pieceP)){ // this piece is captured?
                continue;
            }else if (_pieces[i].pos.x == _pieces[0].pos.x && _pieces[i].pos.y <= _pieces[0].pos.y && _pieces[i].pos.y >= _pieces[1].pos.y){
                return false;
            }
        }
        return true;
    }
    
    // moving a piece would endanger own shuai
    bool selfDanger(Piece* pieceP, Position move){
        TempState temp(this, pieceP, move);
        Position ownShuaiPos = _pieces[pieceP->side == Side::Red? 0:1].pos;
        // check for pieces that will be able to capture shuai
        for (Piece& piece : _pieces){
            if (piece.captured || piece.side == pieceP->side){ // ignore pieces that cannot capture
                continue;
            }
            std::vector<Position> moves = getMoves(&piece, false);
            if (std::find(moves.begin(), moves.end(), ownShuaiPos) != moves.end()){ // own shuai becomes a target
                return true;
            }
        }
        return false;
    }
    
    std::vector<Position> getMoves(Piece* pieceP, bool doDangerCheck = true){
        std::vector<Position> moves;
        Piece piece = *pieceP;
        switch (piece.type){ // different movement pattern for each piece
            case PieceType::Shuai:
                // within the palace
                if (piece.pos.x < 5){
                    moves.push_back(Position{piece.pos.x+1, piece.pos.y});
                }
                if (piece.pos.x > 3){
                    moves.push_back(Position{piece.pos.x-1, piece.pos.y});
                }
                if (piece.side == Side::Red){
                    if (piece.pos.y > 7){
                        moves.push_back(Position{piece.pos.x, piece.pos.y-1});
                    }
                    moves.push_back(Position{piece.pos.x, piece.pos.y+1});
                }else{
                    if (piece.pos.y < 2){
                        moves.push_back(Position{piece.pos.x, piece.pos.y+1});
                    }
                    moves.push_back(Position{piece.pos.x, piece.pos.y-1});
                }
                break;
            case PieceType::Shi:
                // diagonals within the palace
                if (piece.pos.x == 4){
                    moves.push_back(Position{5, piece.pos.y+1});
                    moves.push_back(Position{5, piece.pos.y-1});
                    moves.push_back(Position{3, piece.pos.y+1});
                    moves.push_back(Position{3, piece.pos.y-1});
                }else{
                    moves.push_back(Position{4, (piece.side == Side::Red? 8:1)});
                }
                break;
            case PieceType::Xiang:
                // moves 2 boxes diagonally if not blocked, can't cross river
                if (piece.side != Side::Black || piece.pos.y < 3){
                    if (findPiece(Position{piece.pos.x+1, piece.pos.y+1}) == nullptr){
                        moves.push_back(Position{piece.pos.x+2, piece.pos.y+2});
                    }
                    if (findPiece(Position{piece.pos.x-1, piece.pos.y+1}) == nullptr){
                        moves.push_back(Position{piece.pos.x-2, piece.pos.y+2});
                    }
                }
                if (piece.side != Side::Red || piece.pos.y > 6){
                    if (findPiece(Position{piece.pos.x+1, piece.pos.y-1}) == nullptr){
                        moves.push_back(Position{piece.pos.x+2, piece.pos.y-2});
                    }
                    if (findPiece(Position{piece.pos.x-1, piece.pos.y-1}) == nullptr){
                        moves.push_back(Position{piece.pos.x-2, piece.pos.y-2});
                    }
                }
                break;
            case PieceType::Ma:
                // moves in L shape if direct adjacent paths not blocked
                if (findPiece(Position{piece.pos.x+1, piece.pos.y}) == nullptr){
                    moves.push_back(Position{piece.pos.x+2, piece.pos.y+1});
                    moves.push_back(Position{piece.pos.x+2, piece.pos.y-1});
                }
                if (findPiece(Position{piece.pos.x-1, piece.pos.y}) == nullptr){
                    moves.push_back(Position{piece.pos.x-2, piece.pos.y+1});
                    moves.push_back(Position{piece.pos.x-2, piece.pos.y-1});
                }
                if (findPiece(Position{piece.pos.x, piece.pos.y+1}) == nullptr){
                    moves.push_back(Position{piece.pos.x+1, piece.pos.y+2});
                    moves.push_back(Position{piece.pos.x-1, piece.pos.y+2});
                }
                if (findPiece(Position{piece.pos.x, piece.pos.y-1}) == nullptr){
                    moves.push_back(Position{piece.pos.x+1, piece.pos.y-2});
                    moves.push_back(Position{piece.pos.x-1, piece.pos.y-2});
                }
                break;
            case PieceType::Ju:
                // continues horizontally/vertically until collision then can capture
                for (int i=1; i<10; ++i){
                    moves.push_back(Position{piece.pos.x+i, piece.pos.y});
                    if (findPiece(Position{piece.pos.x+i, piece.pos.y}) != nullptr){
                        break;
                    }
                }
                for (int i=1; i<10; ++i){
                    moves.push_back(Position{piece.pos.x-i, piece.pos.y});
                    if (findPiece(Position{piece.pos.x-i, piece.pos.y}) != nullptr){
                        break;
                    }
                }
                for (int i=1; i<10; ++i){
                    moves.push_back(Position{piece.pos.x, piece.pos.y+i});
                    if (findPiece(Position{piece.pos.x, piece.pos.y+i}) != nullptr){
                        break;
                    }
                }
                for (int i=1; i<10; ++i){
                    moves.push_back(Position{piece.pos.x, piece.pos.y-i});
                    if (findPiece(Position{piece.pos.x, piece.pos.y-i}) != nullptr){
                        break;
                    }
                }
                break;
            case PieceType::Pao:
                // continues horizontally/vertically until collision then can capture the next piece
                int i;
                for (i=1; i<10 && findPiece(Position{piece.pos.x+i, piece.pos.y}) == nullptr; ++i){
                    moves.push_back(Position{piece.pos.x+i, piece.pos.y});
                }
                for (++i; i<10; ++i){
                    Piece* pieceP = findPiece(Position{piece.pos.x+i, piece.pos.y});
                    if (pieceP != nullptr){
                        moves.push_back(pieceP->pos);
                        break;
                    }
                }
                for (i=1; i<10 && findPiece(Position{piece.pos.x-i, piece.pos.y}) == nullptr; ++i){
                    moves.push_back(Position{piece.pos.x-i, piece.pos.y});
                }
                for (++i; i<10; ++i){
                    Piece* pieceP = findPiece(Position{piece.pos.x-i, piece.pos.y});
                    if (pieceP != nullptr){
                        moves.push_back(pieceP->pos);
                        break;
                    }
                }
                for (i=1; i<10 && findPiece(Position{piece.pos.x, piece.pos.y+i}) == nullptr; ++i){
                    moves.push_back(Position{piece.pos.x, piece.pos.y+i});
                }
                for (++i; i<10; ++i){
                    Piece* pieceP = findPiece(Position{piece.pos.x, piece.pos.y+i});
                    if (pieceP != nullptr){
                        moves.push_back(pieceP->pos);
                        break;
                    }
                }
                for (i=1; i<10 && findPiece(Position{piece.pos.x, piece.pos.y-i}) == nullptr; ++i){
                    moves.push_back(Position{piece.pos.x, piece.pos.y-i});
                }
                for (++i; i<10; ++i){
                    Piece* pieceP = findPiece(Position{piece.pos.x, piece.pos.y-i});
                    if (pieceP != nullptr){
                        moves.push_back(pieceP->pos);
                        break;
                    }
                }
                break;
            case PieceType::Bing:
                if (piece.side == Side::Red){
                    moves.push_back(Position{piece.pos.x, piece.pos.y-1});
                    // crossed the river?
                    if (piece.pos.y < 5){
                        moves.push_back(Position{piece.pos.x-1, piece.pos.y});
                        moves.push_back(Position{piece.pos.x+1, piece.pos.y});
                    }
                }else{
                    moves.push_back(Position{piece.pos.x, piece.pos.y+1});
                    if (piece.pos.y > 4){
                        moves.push_back(Position{piece.pos.x-1, piece.pos.y});
                        moves.push_back(Position{piece.pos.x+1, piece.pos.y});
                    }
                }
                break;
            // modded pieces
            case PieceType::Qiang:
                moves.push_back(Position{piece.pos.x+2, piece.pos.y});
                moves.push_back(Position{piece.pos.x-2, piece.pos.y});
                moves.push_back(Position{piece.pos.x, piece.pos.y+2});
                moves.push_back(Position{piece.pos.x, piece.pos.y-2});
                break;
        }
        // remove invalid moves
        auto pred = [this, pieceP, doDangerCheck](Position move){
            if (move.x < 0 || move.x > 8 || move.y < 0 || move.y > 9){ // out of bounds
                return true;
            }else if (lineOfSight(pieceP, move)){ // check if move causes line of sight
                return true;
            }else if (doDangerCheck && selfDanger(pieceP, move)){ // check if move endangers own shuai
                return true;
            }else{ // check if capturing piece of same side
                Piece* otherP = findPiece(move);
                return otherP != nullptr && pieceP->side == otherP->side;
            }
        };
        moves.erase(std::remove_if(moves.begin(), moves.end(), pred), moves.end());
        return moves;
    }
    
    void performMove(Piece* pieceP, Position move, bool temporary = false){
        // captured a piece?
        Piece* otherP = findPiece(move);
        if (otherP != nullptr){
            otherP->captured = true;
        }
        pieceP->pos = move;
        if (!temporary){ // check if leads to checkmate
            bool checkmate = true;
            for (Piece& piece : _pieces){
                if (piece.captured || piece.side == pieceP->side){ // ignore captured/pieces on same side
                    continue;
                }
                if (!getMoves(&piece).empty()){ // still has valid moves
                    checkmate = false;
                    break;
                }
            }
            if (checkmate){
                _gameOver = true;
                _winner = pieceP->side;
            }
        }
    }
    
    void drawCircle(Position pos, int radius, SDL_Color color, bool antiAliasing = true){
        PixelPos centerPos(pos);
        for (int w=0; w<=radius*2; ++w){
            for (int h=0; h<=radius*2; ++h){
                PixelPos offset{radius-w, radius-h};
                PixelPos pPos{centerPos.x+offset.x, centerPos.y+offset.y};
                if (withinDist(pPos, centerPos, radius+1) && !withinDist(pPos, centerPos, radius) && antiAliasing){ // anti-aliasing by combining colors
                    SDL_SetRenderDrawColor(_renderer, color.r, color.g, color.b, color.a/2);
                    SDL_RenderDrawPoint(_renderer, pPos.x, pPos.y);
                }else if (withinDist(pPos, centerPos, radius)){ // within the circle
                    SDL_SetRenderDrawColor(_renderer, color.r, color.g, color.b, color.a);
                    SDL_RenderDrawPoint(_renderer, pPos.x, pPos.y);
                }
            }
        }
    }
    
    void drawText(const char16_t* text, PixelPos pPos, SDL_Color color){
        drawText(_renderer, _font, text, pPos, color);
    }
    
    static void drawText(SDL_Renderer* renderer, TTF_Font* font, const char16_t* text, PixelPos pPos, SDL_Color color){
        SDL_Surface* sur = TTF_RenderUNICODE_Blended(font, (Uint16*)text, color);
        SDL_Rect textRect{pPos.x-(sur->w)/2, pPos.y-(sur->h)/2, sur->w, sur->h}; // centered on pPos
        SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, sur);
        SDL_RenderCopy(renderer, tex, nullptr, &textRect);
        SDL_FreeSurface(sur);
        SDL_DestroyTexture(tex);
    }
    
    void drawPiece(Piece piece){
        SDL_Color borderColor = piece.side == Side::Red? _redPieceBorderColor:_blackPieceBorderColor;
        SDL_Color innerColor = piece.side == Side::Red? _redPieceInnerColor:_blackPieceInnerColor;
        if (_selectedPiece != nullptr && *_selectedPiece == piece){
            innerColor = _selectedPieceInnerColor;
        }
        // outer border
        drawCircle(piece.pos, _pieceRadius, borderColor);
        // inner circle
        drawCircle(piece.pos, _pieceRadius-_borderWidth, innerColor);
        // draw chinese character inside
        const static std::map<PieceType, char16_t> redCharacters{
            {PieceType::Shuai, u'帅'},
            {PieceType::Shi, u'士'},
            {PieceType::Xiang, u'相'},
            {PieceType::Ma, u'马'},
            {PieceType::Ju, u'车'},
            {PieceType::Pao, u'炮'},
            {PieceType::Bing, u'兵'},
            {PieceType::Qiang, u'枪'},
        };
        const static std::map<PieceType, char16_t> blackCharacters{
            {PieceType::Shuai, u'将'},
            {PieceType::Shi, u'仕'},
            {PieceType::Xiang, u'象'},
            {PieceType::Ma, u'马'},
            {PieceType::Ju, u'车'},
            {PieceType::Pao, u'砲'},
            {PieceType::Bing, u'卒'},
            {PieceType::Qiang, u'枪'},
        };
        if (piece.side == Side::Red){
            drawText(&redCharacters.at(piece.type), PixelPos(piece.pos), _redPieceBorderColor);
        }else{
            drawText(&blackCharacters.at(piece.type), PixelPos(piece.pos), _blackPieceBorderColor);
        }
    }
    
    void resetState(){
        _pieces.clear();
        _pieces.assign(_pieceInitialSetup, _pieceInitialSetup+32);
        _selectedPiece = nullptr;
        _moves.clear();
        _currentTurn = Side::Red;
        _gameOver = false;
        _resetButtonState = false;
    }
    
    // clears pieces leaving only board and other guis
    void refreshBoard(bool flash = false){
        SDL_RenderClear(_renderer);
        SDL_Rect imgRect{0, 0, _boardWidth, _boardHeight};
        SDL_RenderCopy(_renderer, _boardImg, nullptr, &imgRect);
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
        if (_gameOver && _winner == Side::Red){
            drawText(u"红方赢", PixelPos{_boardWidth/2, _boardHeight+_bottomBarHeight/2}, flash? _winTextFlashColor:_redPieceBorderColor);
        }else if (_gameOver && _winner == Side::Black){
            drawText(u"黑方赢", PixelPos{_boardWidth/2, _boardHeight+_bottomBarHeight/2}, flash? _winTextFlashColor:_blackPieceBorderColor);
        }else if (_currentTurn == Side::Red){
            drawText(u"红方走", PixelPos{_boardWidth/2, _boardHeight+_bottomBarHeight/2}, _redPieceBorderColor);
        }else{
            drawText(u"黑方走", PixelPos{_boardWidth/2, _boardHeight+_bottomBarHeight/2}, _blackPieceBorderColor);
        }
        // draw buttons
        for (const Button& button : _buttons){
            button.render(_renderer, _font);
        }
    }
    
    void updateWindow(){
        SDL_RenderPresent(_renderer);
    }
};

int main(int argc, const char * argv[]) {
    Game game;
    game.run();
    return 0;
}
