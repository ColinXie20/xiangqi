//
//  game_logic.hpp
//  xiangqi
//
//  Created by Colin Xie on 7/9/24.
//

#pragma once

#include <vector>

enum class PieceType{
    Shuai,
    Shi,
    Xiang,
    Ma,
    Ju,
    Pao,
    Bing,
};

enum class Side{
    Red,
    Black,
};

struct Position{
    int x, y;
    
    bool operator==(Position other) const;
};

struct Piece{
    Position pos;
    PieceType type;
    Side side;
    bool captured;
    
    bool operator==(Piece other) const;
};

class GameState{
    std::vector<Piece> _pieces;
    std::vector<std::vector<Piece*>> _pieceGrid; // nullptr if position on grid does not have a piece
    Side _currentTurn;
    bool _check, _checkmate;
    Side _checking;
    
    constexpr static int _defaultSetupSize = 32;
    constexpr static Piece _defaultSetup[_defaultSetupSize]{
        Piece{Position{4, 9}, PieceType::Shuai, Side::Red, false},
        Piece{Position{4, 0}, PieceType::Shuai, Side::Black, false},
        Piece{Position{0, 9}, PieceType::Ju, Side::Red, false},
        Piece{Position{8, 9}, PieceType::Ju, Side::Red, false},
        Piece{Position{1, 7}, PieceType::Pao, Side::Red, false},
        Piece{Position{7, 7}, PieceType::Pao, Side::Red, false},
        Piece{Position{1, 9}, PieceType::Ma, Side::Red, false},
        Piece{Position{7, 9}, PieceType::Ma, Side::Red, false},
        Piece{Position{2, 9}, PieceType::Xiang, Side::Red, false},
        Piece{Position{6, 9}, PieceType::Xiang, Side::Red, false},
        Piece{Position{3, 9}, PieceType::Shi, Side::Red, false},
        Piece{Position{5, 9}, PieceType::Shi, Side::Red, false},
        Piece{Position{0, 6}, PieceType::Bing, Side::Red, false},
        Piece{Position{2, 6}, PieceType::Bing, Side::Red, false},
        Piece{Position{4, 6}, PieceType::Bing, Side::Red, false},
        Piece{Position{6, 6}, PieceType::Bing, Side::Red, false},
        Piece{Position{8, 6}, PieceType::Bing, Side::Red, false},
        Piece{Position{0, 0}, PieceType::Ju, Side::Black, false},
        Piece{Position{8, 0}, PieceType::Ju, Side::Black, false},
        Piece{Position{1, 2}, PieceType::Pao, Side::Black, false},
        Piece{Position{7, 2}, PieceType::Pao, Side::Black, false},
        Piece{Position{1, 0}, PieceType::Ma, Side::Black, false},
        Piece{Position{7, 0}, PieceType::Ma, Side::Black, false},
        Piece{Position{2, 0}, PieceType::Xiang, Side::Black, false},
        Piece{Position{6, 0}, PieceType::Xiang, Side::Black, false},
        Piece{Position{3, 0}, PieceType::Shi, Side::Black, false},
        Piece{Position{5, 0}, PieceType::Shi, Side::Black, false},
        Piece{Position{0, 3}, PieceType::Bing, Side::Black, false},
        Piece{Position{2, 3}, PieceType::Bing, Side::Black, false},
        Piece{Position{4, 3}, PieceType::Bing, Side::Black, false},
        Piece{Position{6, 3}, PieceType::Bing, Side::Black, false},
        Piece{Position{8, 3}, PieceType::Bing, Side::Black, false},
    };
    
    class TempPiecesState{
        GameState* _gameState;
        std::vector<Piece> _prevPiecesState;
        std::vector<std::vector<Piece*>> _prevPieceGridState;
        
    public:
        TempPiecesState(GameState* stateP, Piece* pieceP, Position move);
        
        ~TempPiecesState();
    };
    
    void switchTurns();
public:
    GameState();
    
    Piece* findPiece(Position pos);
    
    // moving a piece would cause line of sight?
    bool lineOfSight(Piece* pieceP, Position move);
    
    // moving a piece would endanger own shuai
    bool selfDanger(Piece* pieceP, Position move);
    
    std::vector<Position> getMoves(Piece* pieceP, bool doDangerCheck = true);
    
    void performMove(Piece* pieceP, Position move, bool temporary = false);
    
    void reset();
    
    std::vector<Piece>& pieces();
    
    Side currentTurn() const;
    
    bool check() const;
    
    bool checkmate() const;
    
    Side checking() const;
};
