//
//  game_logic.cpp
//  xiangqi
//
//  Created by Colin Xie on 7/9/24.
//

#include "game_logic.hpp"

bool Position::operator==(Position other) const{
    return x == other.x && y == other.y;
}

bool Piece::operator==(Piece other) const{
    return pos == other.pos && type == other.type && side == other.side && captured == other.captured;
}

GameState::TempPiecesState::TempPiecesState(GameState* stateP, Piece* pieceP, Position move) : _gameState(stateP){
    _prevPiecesState = _gameState->_pieces;
    _prevPieceGridState = _gameState->_pieceGrid;
    _gameState->performMove(pieceP, move, true);
}

GameState::TempPiecesState::~TempPiecesState(){
    _gameState->_pieces = _prevPiecesState;
    _gameState->_pieceGrid = _prevPieceGridState;
}

void GameState::switchTurns(){
    _currentTurn = _currentTurn == Side::Red? Side::Black:Side::Red;
}

GameState::GameState(){
    reset();
}

Piece* GameState::findPiece(Position pos){
    if (pos.x < 0 || pos.x > 8 || pos.y < 0 || pos.y > 9){ // out of bounds
        return nullptr;
    }
    return _pieceGrid[pos.x][pos.y];
}

bool GameState::lineOfSight(Piece* pieceP, Position move){
    TempPiecesState temp(this, pieceP, move);
    if (_pieces[0].pos.x != _pieces[1].pos.x){
        return false;
    }
    // check for pieces between
    for (int i=_pieces[1].pos.y; i<=_pieces[0].pos.y; ++i){
        if (_pieceGrid[_pieces[0].pos.x][i] == nullptr || _pieceGrid[_pieces[0].pos.x][i]->type == PieceType::Shuai){ // ignore if not a piece that can block line of sight
            continue;
        }
        return false;
    }
    return true;
}

bool GameState::selfDanger(Piece* pieceP, Position move){
    TempPiecesState temp(this, pieceP, move);
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

std::vector<Position> GameState::getMoves(Piece* pieceP, bool doDangerCheck){
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
    }
    // remove invalid moves
    auto pred = [this, pieceP, doDangerCheck](Position move){
        if (move.x < 0 || move.x > 8 || move.y < 0 || move.y > 9){ // out of bounds
            return true;
        }
        if (lineOfSight(pieceP, move)){ // check if move causes line of sight
            return true;
        }
        Piece* otherP = findPiece(move);
        if (doDangerCheck && selfDanger(pieceP, move)){ // check if move endangers own shuai
            return !(otherP != nullptr && otherP->type == PieceType::Shuai && otherP->side != pieceP->side); // only allowed if targeting enemy shuai b/c that would end the game
        }
        return otherP != nullptr && otherP->side == pieceP->side; // check if capturing piece of same side
    };
    moves.erase(std::remove_if(moves.begin(), moves.end(), pred), moves.end());
    return moves;
}

void GameState::performMove(Piece* pieceP, Position move, bool temporary){
    // captured a piece?
    Piece* otherP = findPiece(move);
    if (otherP != nullptr){
        otherP->captured = true;
    }
    _pieceGrid[pieceP->pos.x][pieceP->pos.y] = nullptr;
    _pieceGrid[move.x][move.y] = pieceP;
    pieceP->pos = move;
    if (!temporary){
        // check if checking enemy shuai
        _check = false;
        for (Piece& piece : _pieces){
            if (piece.captured || piece.side != pieceP->side){ // ignore captured/pieces on enemy side
                continue;
            }
            std::vector<Position> moves = getMoves(&piece);
            if (std::find(moves.begin(), moves.end(), _pieces[pieceP->side == Side::Red? 1:0].pos) != moves.end()){
                _check = true;
                _checking = pieceP->side;
                break;
            }
        }
        if (_check){
            // check if leads to checkmate
            _checkmate = true;
            for (Piece& piece : _pieces){
                if (piece.captured || piece.side == pieceP->side){ // ignore captured/pieces on same side
                    continue;
                }
                if (!getMoves(&piece).empty()){ // opponent still has valid moves
                    _checkmate = false;
                    break;
                }
            }
        }
        switchTurns();
    }
}

void GameState::reset(){
    _pieces.clear();
    _pieces.assign(_defaultSetup, _defaultSetup+_defaultSetupSize);
    _pieceGrid.assign(9, std::vector<Piece*>(10, nullptr));
    for (Piece& piece : _pieces){
        _pieceGrid[piece.pos.x][piece.pos.y] = &piece;
    }
    _currentTurn = Side::Red;
    _check = false;
    _checkmate = false;
}

std::vector<Piece>& GameState::pieces(){
    return _pieces;
}

Side GameState::currentTurn() const{
    return _currentTurn;
}

bool GameState::check() const{
    return _check;
}

bool GameState::checkmate() const{
    return _checkmate;
}

Side GameState::checking() const{
    return _checking;
}
