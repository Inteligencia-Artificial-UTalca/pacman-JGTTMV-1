#pragma once
#include "Controller.h"
#include "Ghost.h"

class DTGhostController : public Controller 
{
public:
    virtual ~DTGhostController();
    virtual Move getMove(const GameState& game) override;
    DTGhostController(std::shared_ptr<Character> character);
};