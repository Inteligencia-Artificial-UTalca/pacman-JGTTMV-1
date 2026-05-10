#pragma once

#include "Controller.h"
#include "BehaviorTree.h"
#include "BTGhostController.h" // Para usar la clase Info y utilidades

// Detecta si hay algún fantasma no comestible cerca
class GhostNear : public Behavior {
public:
    virtual Status update() override;
};

// Se mueve en dirección opuesta al fantasma más cercano
class Escape : public Behavior {
public:
    virtual Status update() override;
};

// Se dirige hacia la píldora más cercana
class EatPills : public Behavior {
public:
    virtual Status update() override;
};

class MsPacmanController : public Controller {
private:
    std::shared_ptr<Composite> root;
public:
    MsPacmanController(std::shared_ptr<Character> character);
    virtual ~MsPacmanController();
    virtual Move getMove(const GameState& game) override;
};