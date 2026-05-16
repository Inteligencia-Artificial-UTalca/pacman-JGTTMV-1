#include "SimplePacmanController.h"
#include <cmath>
#include <algorithm>
#include <SDL2/SDL.h>

SimplePacmanController::SimplePacmanController(std::shared_ptr<Character> character) : Controller(character) {}
SimplePacmanController::~SimplePacmanController() {}

//metodos de distancia y movimiento basicos

float SimplePacmanController::getDistanceToNode(const GameState& game, int nodeIndex) const 
{
    if (nodeIndex < 0) return 1000000.0f;
    return sqrt(euclid2(
        game.getMaze().getNodePos(character->getPos()),
        game.getMaze().getNodePos(nodeIndex)));
}

Move SimplePacmanController::getClosestMove(const GameState& game, std::pair<int,int> target) const 
{
    int minDist = 10000000;
    Move minMove = character->getDirection();
    std::vector<Move> moves = game.getMaze().getPossibleMoves(character->getPos());
    for(Move m : moves)
{
        int vecino = game.getMaze().getNeighbour(character->getPos(), m);
        if(vecino < 0) continue;
        int sqDist = euclid2(game.getMaze().getNodePos(vecino), target);
        if(sqDist < minDist)
		{
            minDist = sqDist;
            minMove = m;
        }
    }
    return minMove;
}

Move SimplePacmanController::getFarthestMove(const GameState& game, std::pair<int,int> target) const 
{
    int maxDist = -1;
    Move maxMove = character->getDirection();
    std::vector<Move> moves = game.getMaze().getPossibleMoves(character->getPos());
    for(Move m : moves)
	{
        int vecino = game.getMaze().getNeighbour(character->getPos(), m);
        if(vecino < 0) continue;
        int sqDist = euclid2(game.getMaze().getNodePos(vecino), target);
        if(sqDist > maxDist)
		{
            maxDist = sqDist;
            maxMove = m;
        }
    }
    return maxMove;
}

//metodos para buscar objetivos cercanos

std::pair<int,int> SimplePacmanController::getClosestPillCoords(const GameState& game) const 
{
    const auto& pills = game.getMaze().getPillPositions();
    const auto& powerPills = game.getMaze().getPowerPillPositions();
    
    std::pair<int,int> closestCoords = {-1, -1};
    float minDist = 1000000.0f;

    for (const auto &p : pills) {

        float d = euclid2(game.getMaze().getNodePos(character->getPos()), p);
        if (d < minDist) 
		{
            minDist = d;
            closestCoords = p;
        }
    }
    for (const auto &pp : powerPills) 
	{
        float d = euclid2(game.getMaze().getNodePos(character->getPos()), pp);
        if (d < minDist) 
		{
            d = d * 0.8f; //prioriza los power pills un 20% más
            minDist = d;
            closestCoords = pp;
        }
    }
    return closestCoords;
}

int SimplePacmanController::getClosestGhostIndex(const GameState& game, bool edible) const 
{
    int targetGhost = -1;
    float minDist = 1000000.0f;
    for (int i = 0; i < 4; i++) 
	{
        if (game.isGhostEdible(i) == edible) 
		{
            float d = getDistanceToNode(game, game.getGhostsPos(i));
            if (d < minDist) { minDist = d; targetGhost = i; }
        }
    }
    return targetGhost;
}


//curva exponencial inversa: El miedo se dispara criticamente cuando estan a menos de 8 celdas
float SimplePacmanController::calculateSurvivalUtility(float distance) const 
{
    if (distance <= 0.1f) return 1.0f;
    return 1.0f / (1.0f + pow(distance / 6.0f, 4));
}

//curva polinomial cuadratica: Las ganas de cazar disminuyen gradualmente con la distancia
float SimplePacmanController::calculateHuntUtility(float distance) const 
{
    if (distance > 30.0f) return 0.0f;
    return pow(1.0f - (distance / 30.0f), 2);
}

Move SimplePacmanController::getMove(const GameState& game) 
{
    //mantiene funcionalidad de cierre de ventana
    SDL_Event ev;
    if (SDL_PollEvent(&ev) != 0) 
	{
        if (ev.type == SDL_QUIT || (ev.type == SDL_KEYDOWN && (ev.key.keysym.sym == SDLK_ESCAPE || ev.key.keysym.sym == SDLK_q))) 
		{
            SDL_Quit();
            exit(0);
        }
    }

    int dangerousGhostIdx = getClosestGhostIndex(game, false);
    int edibleGhostIdx = getClosestGhostIndex(game, true);
    auto targetPillCoords = getClosestPillCoords(game);

    float distToDanger = (dangerousGhostIdx != -1) ? getDistanceToNode(game, game.getGhostsPos(dangerousGhostIdx)) : 1000.0f;
    float distToFood = (edibleGhostIdx != -1) ? getDistanceToNode(game, game.getGhostsPos(edibleGhostIdx)) : 1000.0f;

    float uSurvive = calculateSurvivalUtility(distToDanger);
    float uHunt = (edibleGhostIdx != -1) ? calculateHuntUtility(distToFood) : 0.0f;
    float uClearPills = 0.25f; //utilidad base fija: desea constantemente limpiar el mapa si todo esta tranquilo

    float maxUtility = std::max({uSurvive, uHunt, uClearPills});

    if (maxUtility == uSurvive && dangerousGhostIdx != -1) 
	{
        //Accion A: Huye del peligro
        auto ghostCoords = game.getMaze().getNodePos(game.getGhostsPos(dangerousGhostIdx));
        return getFarthestMove(game, ghostCoords);
    } 
    else if (maxUtility == uHunt && edibleGhostIdx != -1) 
	{
        //Accion B: Persigue fantasma azul
        auto ghostCoords = game.getMaze().getNodePos(game.getGhostsPos(edibleGhostIdx));
        return getClosestMove(game, ghostCoords);
    } 
    else if (targetPillCoords.first != -1) 
	{
        //Accion C: Va por la pildora mas cercana
        return getClosestMove(game, targetPillCoords);
    }

    return PASS;
}