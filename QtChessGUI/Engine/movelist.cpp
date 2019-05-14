//============================================================
// movelist.cpp
// ChessEngine
//============================================================

#include "movelist.h"
#include <algorithm>
#include <cassert>

using namespace BlendXChess;

//============================================================
// Constructor
//============================================================
MoveList::MoveList(void)
	: moveCnt(0), moveIdx(0)
{}

//============================================================
// Add a move to the list
//============================================================
void MoveList::add(Move move)
{
	assert(moveCnt < MAX_MOVECNT);
	moves[moveCnt++] = move;
}

//============================================================
// Get next move (in the order specified by move scores)
//============================================================
Move MoveList::getNext(void)
{
	return moveIdx < moveCnt ? moves[moveIdx++] : MOVE_NONE;
}

//============================================================
// Access the move by index
//============================================================
Move& MoveList::operator[](int idx)
{
	assert(0 <= idx && idx < moveCnt);
	return moves[idx];
}

//============================================================
// Get AN representation of moves
//============================================================
std::vector<std::string> MoveList::toAN(void) const
{
	std::vector<std::string> ret;
	for (int i = 0; i < moveCnt; ++i)
		ret.push_back(moves[i].toAN());
	return std::move(ret);
}
