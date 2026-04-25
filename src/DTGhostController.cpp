#include "DTGhostController.h"

DTGhostController::DTGhostController(std::shared_ptr<Character> character)
: Controller(character) {}

DTGhostController::~DTGhostController() {}

Move DTGhostController::getMove(const GameState& game) 
{

    std::vector<Move> moves;
    std::vector<Move> filteredMoves;

    auto ghost = std::dynamic_pointer_cast<Ghost>(character);
    auto pacmanPos = game.getMaze().getNodePos(game.getPacmanPos());

    if(character->getDirection() == PASS)
    {
        moves = game.getMaze().getPossibleMoves(character->getPos());
    } else 
    {
        moves = game.getMaze().getGhostLegalMoves(character->getPos(), character->getDirection()
        );
    }

    for(auto m : moves)
    {
        if(m != PASS)
        {
            filteredMoves.push_back(m);
        }
    }

    moves = filteredMoves;

    if(moves.empty())
    {
        return PASS;
    }

    Move bestMove = moves[0];

    auto firstPos = game.getMaze().getNodePos(game.getMaze().getNeighbour(character->getPos(), bestMove));

    float bestValue = euclid2(pacmanPos, firstPos);

    for(auto move : moves)
    {

        auto nextPos = game.getMaze().getNodePos( game.getMaze().getNeighbour(character->getPos(), move));

        float dist = euclid2(pacmanPos, nextPos);

        if(ghost->isEdible())
        {
            if(dist > bestValue)
            {
                bestValue = dist;
                bestMove = move;
            }
        } else 
        {
            if(dist < bestValue)
            {
                bestValue = dist;
                bestMove = move;
            }
        }
    }

    return bestMove;
}