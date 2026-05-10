#include "PinkyController.h"


PinkyController::PinkyController(std::shared_ptr<Character> character) 
    : Controller(character), root(std::make_shared<Selector>()) 
{
    
    // Powerpill -> Frightened
    auto filterFrightened = std::make_shared<Filter>();
    filterFrightened->addCondition(std::make_shared<Powerpill>());
    filterFrightened->addAction(std::make_shared<Frightened>());
    root->addChild(filterFrightened);

	// Scatter -> Scatter
	auto filterScatter = std::make_shared<Filter>();
    filterScatter->addCondition(std::make_shared<TimeOut>());
    filterScatter->addAction(std::make_shared<Scatter>());
    root->addChild(filterScatter);

    // Chase -> Chase
    root->addChild(std::make_shared<PinkyChase>());
}

PinkyController::~PinkyController() 
{
}

Move PinkyController::getMove(const GameState& game)
{
	Info::getInfo()->in_character = character;
    Info::getInfo()->in_gamestate = &game;
    
    root->tick();

    return Info::getInfo()->out_move;
}

Status PinkyChase::update() 
{
    auto character = Info::getInfo()->in_character;
    auto gs = Info::getInfo()->in_gamestate;
    
    //obtiene posicion del nodo objetivo (Pacman)
    auto target = gs->getMaze().getNodePos(gs->getPacmanPos());
    
    float min = 1000000000;
    Move minMove = PASS;
    std::vector<Move> moves;

    //obtiene movimientos validos segun la direccion actual
    if(character->getDirection() == PASS) 
    {
        moves = gs->getMaze().getPossibleMoves(character->getPos());
    } else 
    {
        moves = gs->getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
    }

    for(auto move : moves) 
    {
        if(move == PASS) break;
        
        //calcula distancia
        float dist = euclid2(target, gs->getMaze().getNodePos(gs->getMaze().getNeighbour(character->getPos(), move)));
        if(dist < min) 
        {
            min = dist;
            minMove = move;
        }
    }

    Info::getInfo()->out_move = minMove;
    return BH_SUCCESS;
}
