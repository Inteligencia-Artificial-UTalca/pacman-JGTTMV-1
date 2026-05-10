#include "MsPacmanController.h"

MsPacmanController::MsPacmanController(std::shared_ptr<Character> character) 
    : Controller(character), root(std::make_shared<Selector>()) {
    
    // Supervivencia (Si hay fantasmas cerca -> Escapar)
    auto survivalFilter = std::make_shared<Filter>();
    survivalFilter->addCondition(std::make_shared<GhostNear>());
    survivalFilter->addAction(std::make_shared<Escape>());
    root->addChild(survivalFilter);

    // Recolectar (Si no hay peligro -> Comer píldoras)
    root->addChild(std::make_shared<EatPills>());
}

MsPacmanController::~MsPacmanController() {}

Move MsPacmanController::getMove(const GameState& game) {
    Info::getInfo()->in_character = character;
    Info::getInfo()->in_gamestate = &game;
    root->tick();
    return Info::getInfo()->out_move;
}

// Comportamientos

Status GhostNear::update() {
    auto character = Info::getInfo()->in_character;
    auto gs = Info::getInfo()->in_gamestate;
    auto myPos = gs->getMaze().getNodePos(character->getPos());

    for (int i = 0; i < 4; i++) {
        // Solo asustarse si el fantasma NO es comestible
        if (gs->isGhostEdible(i)) continue;

        auto ghostPos = gs->getMaze().getNodePos(gs->getGhostsPos(i));
        // Si el fantasma está a menos de una distancia prudente
        if (euclid2(myPos, ghostPos) < 150.0f) {
            return BH_SUCCESS;
        }
    }
    return BH_FAILURE;
}

Status Escape::update() {
    auto character = Info::getInfo()->in_character;
    auto gs = Info::getInfo()->in_gamestate;
    
    std::vector<Move> moves = gs->getMaze().getPossibleMoves(character->getPos());
    Move bestMove = PASS;
    float maxDist = -1.0f;

    for (auto m : moves) {
        int nextPos = gs->getMaze().getNeighbour(character->getPos(), m);
        auto nextCoords = gs->getMaze().getNodePos(nextPos);
        
        // Calculamos distancia al primer fantasma para simplificar
        auto ghostCoords = gs->getMaze().getNodePos(gs->getGhostsPos(0));
        float d = euclid2(nextCoords, ghostCoords);
        
        if (d > maxDist) {
            maxDist = d;
            bestMove = m;
        }
    }
    Info::getInfo()->out_move = bestMove;
    return BH_SUCCESS;
}

Status EatPills::update() {
    auto character = Info::getInfo()->in_character;
    auto gs = Info::getInfo()->in_gamestate;
    
    auto pills = gs->getMaze().getPillPositions();
    if (pills.empty()) return BH_FAILURE;

    auto myPos = gs->getMaze().getNodePos(character->getPos());
    std::pair<int,int> closestPill = pills[0];
    float minDist = 1e10;

    // Buscar la píldora más cercana en el mapa
    for (auto p : pills) {
        float d = euclid2(myPos, p);
        if (d < minDist) {
            minDist = d;
            closestPill = p;
        }
    }

    // Elegir el movimiento que nos acerque a esa píldora
    std::vector<Move> moves = gs->getMaze().getPossibleMoves(character->getPos());
    Move bestMove = PASS;
    float minMoveDist = 1e10;

    for (auto m : moves) {
        int nextPos = gs->getMaze().getNeighbour(character->getPos(), m);
        float d = euclid2(gs->getMaze().getNodePos(nextPos), closestPill);
        if (d < minMoveDist) {
            minMoveDist = d;
            bestMove = m;
        }
    }
    Info::getInfo()->out_move = bestMove;
    return BH_SUCCESS;
}