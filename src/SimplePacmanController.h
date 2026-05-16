#pragma once

#include "Controller.h"
#include <utility>
#include <vector>

class SimplePacmanController : public Controller 
{
private:
    //metodos de movimiento y distancias
    Move getClosestMove(const GameState& game, std::pair<int,int> target) const;
    Move getFarthestMove(const GameState& game, std::pair<int,int> target) const;
    float getDistanceToNode(const GameState& game, int nodeIndex) const;
    
    //nodos/objetivos de interes
    std::pair<int,int> getClosestPillCoords(const GameState& game) const;
    int getClosestGhostIndex(const GameState& game, bool edible) const;

    //funciones de curvas de utilidad (Normalizadas entre 0.0 y 1.0)
    float calculateSurvivalUtility(float distance) const;
    float calculateHuntUtility(float distance) const;

public:
    SimplePacmanController(std::shared_ptr<Character> character);
    virtual ~SimplePacmanController();
    virtual Move getMove(const GameState& game) override;
};
