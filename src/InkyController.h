#pragma once

#include "Controller.h"
#include <utility>
#include <vector>

class InkyController: public Controller {
private:
    //metodos auxiliares de movimiento basados en distancias euclidianas
    Move getClosestMove(const GameState& game, std::pair<int,int> target) const;
    Move getFarthestMove(const GameState& game, std::pair<int,int> target) const;
    float getDistanceToPacman(const GameState& game) const;

    //funciones matematicas de utilidad
    float calculateAmbushUtility(float distance) const;
    float calculateRetreatUtility(float distance) const;

public:
    InkyController(std::shared_ptr<Character> character);
    virtual ~InkyController();
    virtual Move getMove(const GameState& game) override;
};