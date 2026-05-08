/*
 * FSMController.h
 *
 *  Created on: Apr 23, 2018
 *      Author: nbarriga
 */

#ifndef FSMCONTROLLER_H_
#define FSMCONTROLLER_H_

#include "Controller.h"
#include <random>
#include "FSM.h"

class BlinkyStateMachine;

class FSMController: public Controller {
	std::mt19937 e;
	std::uniform_int_distribution<int> uniform_dist;
	std::shared_ptr<BlinkyStateMachine> fsm;
public:
	FSMController(std::shared_ptr<Character> character);
	virtual ~FSMController();
	virtual Move getMove(const GameState& game)override;
};

class PillTransition:public FSMTransition{
	int last;
	std::shared_ptr<FSMState> _next;
public:
	PillTransition(std::shared_ptr<FSMState> next);
	bool isValid(const GameState& gs)override;
	std::shared_ptr<FSMState> getNextState()override;
};

class ChaseState:public FSMState{

public:
	ChaseState(std::shared_ptr<Character> _character);
	Move onUpdate(const GameState& gs) override;
	void onEnter(const GameState& gs) override;
	~ChaseState();

};

class BlinkyStateMachine: public FiniteStateMachine{

public:
	BlinkyStateMachine(std::shared_ptr<Character> _character);
	Move update(const GameState& gs) override;
	~BlinkyStateMachine();

};

class ScatterState : public FSMState {
public:
    ScatterState(std::shared_ptr<Character> _character);
    Move onUpdate(const GameState& gs) override;
};

class FrightenedState : public FSMState {
public:
    FrightenedState(std::shared_ptr<Character> _character);
    Move onUpdate(const GameState& gs) override;
};

//Se crea una clase para el estado jerarquico no asustado, que contiene a los estados de Scatter y Chase
class NonFrightenedState : public FSMState {
    std::shared_ptr<FSMState> activeSubState;
    std::shared_ptr<FSMState> chase;
    std::shared_ptr<FSMState> scatter;
    std::chrono::high_resolution_clock::time_point lastSwitch;
    bool isScatter;
public:
    NonFrightenedState(std::shared_ptr<Character> _character);
    Move onUpdate(const GameState& gs) override;
    void onEnter(const GameState& gs) override;
};
class PowerPillTransition : public FSMTransition {
    std::shared_ptr<FSMState> _next;
public:
    PowerPillTransition(std::shared_ptr<FSMState> next);
    bool isValid(const GameState& gs) override;
    std::shared_ptr<FSMState> getNextState() override;
};

class TimerTransition : public FSMTransition {
    std::shared_ptr<FSMState> _next;
    double _duration;
    std::chrono::high_resolution_clock::time_point _start;
    bool _running;
public:
    TimerTransition(std::shared_ptr<FSMState> next, double duration);
    bool isValid(const GameState& gs) override;
    std::shared_ptr<FSMState> getNextState() override;
    void onTransition(const GameState& gs) override;
};

#endif /* FSMCONTROLLER_H_ */
