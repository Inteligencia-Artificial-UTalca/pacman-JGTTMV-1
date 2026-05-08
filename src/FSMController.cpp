/*
 * FSMController.cpp
 *
 *  Created on: Apr 23, 2018
 *      Author: nbarriga
 */

#include "FSMController.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include "GameState.h"


FSMController::FSMController(std::shared_ptr<Character> character):
	Controller(character),
	e(rand()),
	uniform_dist(0,3),
	fsm(std::make_shared<BlinkyStateMachine>(character)) {
}

FSMController::~FSMController() {
}

ScatterState::ScatterState(std::shared_ptr<Character> _character) : FSMState(_character) {}

Move ScatterState::onUpdate(const GameState& game) {
    const auto targetCoord = std::make_pair(27, 0); //Se definen coordenadas para blinky
    const auto myPos = character->getPos();
    auto moves = game.getMaze().getGhostLegalMoves(myPos, character->getDirection());
    
    if (moves.empty()) return PASS;

    //Logica de distancia minima similar a ChaseState pero hacia targetCoord
    float min = euclid2(
        game.getMaze().getNodePos(game.getMaze().getNeighbour(myPos, moves[0])),
        targetCoord);
    int minI = 0;
    for(unsigned int i=1; i<moves.size(); i++){
        auto dist = euclid2(
            game.getMaze().getNodePos(game.getMaze().getNeighbour(myPos, moves[i])),
            targetCoord);
        if(dist < min){
            min = dist;
            minI = i;
        }
    }
    return moves[minI];
}

NonFrightenedState::NonFrightenedState(std::shared_ptr<Character> _character) : FSMState(_character) {
    chase = std::make_shared<ChaseState>(_character);
    scatter = std::make_shared<ScatterState>(_character);
    activeSubState = scatter;
    lastSwitch = std::chrono::high_resolution_clock::now();
    isScatter = true;
}

Move NonFrightenedState::onUpdate(const GameState& gs) {
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = now - lastSwitch;
    if (isScatter && diff.count() > 7.0) {
        activeSubState = chase;
        isScatter = false;
        lastSwitch = now;
    } else if (!isScatter && diff.count() > 20.0) {
        activeSubState = scatter;
        isScatter = true;
        lastSwitch = now;
    }
    return activeSubState->onUpdate(gs);
}

void NonFrightenedState::onEnter(const GameState& gs) {
    lastSwitch = std::chrono::high_resolution_clock::now();
    isScatter = true;
    activeSubState = scatter;
}

BlinkyStateMachine::BlinkyStateMachine(std::shared_ptr<Character> _character) : FiniteStateMachine(_character) {
    auto nonFrightened = std::make_shared<NonFrightenedState>(character);
    auto frightened = std::make_shared<FrightenedState>(character);

    initialState = nonFrightened;
    activeState = initialState;

    //Transicion a Frightened cuando Pac-Man come una PowerPill
    nonFrightened->addTransition(std::make_shared<PowerPillTransition>(frightened));
    frightened->addTransition(std::make_shared<TimerTransition>(nonFrightened, 10.0)); 
}

Move BlinkyStateMachine::update(const GameState& gs) {

    auto t = activeState->getActiveTransition(gs);

    if (t != nullptr) {

        activeState->onExit(gs);

        t->onTransition(gs);

        activeState = t->getNextState();

        activeState->onEnter(gs);

    }

    return activeState->onUpdate(gs);

}

BlinkyStateMachine::~BlinkyStateMachine() {}

Move
FSMController::getMove(const GameState& game){
	return fsm->update(game);
}


///////////////////////////////////PillTransition///////////////////////////////
PillTransition::PillTransition(std::shared_ptr<FSMState> next):last(0),_next(next){

}

bool PillTransition::isValid(const GameState& gs){
	int quedan=gs.getMaze().getPillPositions().size();
	if(last!=quedan && quedan%20==0){
		last =quedan;
		return true;
	}
	return false;
}
std::shared_ptr<FSMState> PillTransition::getNextState(){
	return _next;
}



///////////////////////////////ChaseState///////////////////////////////////////
ChaseState::ChaseState(std::shared_ptr<Character> _character):FSMState(_character){

}
void ChaseState::onEnter(const GameState& ){
	std::dynamic_pointer_cast<Ghost>(character)->revert();
}
Move ChaseState::onUpdate(const GameState& game){
	std::vector<Move> moves;
	const auto pacmanCoord=game.getMaze().getNodePos(game.getPacmanPos());
	const auto myPos=character->getPos();
	//const auto myCoord=game.getMaze().getNodePos(myPos);

	if(character->getDirection()==PASS){
		moves=game.getMaze().getPossibleMoves(myPos);
	}else{
		moves=game.getMaze().getGhostLegalMoves(myPos,character->getDirection());
	}

	float min=euclid2(
		game.getMaze().getNodePos(game.getMaze().getNeighbour(myPos,moves[0])),
			pacmanCoord);
	int minI=0;
	for(unsigned int i=1;i<moves.size();i++){
		auto dist=euclid2(
			game.getMaze().getNodePos(game.getMaze().getNeighbour(myPos,moves[i])),
			pacmanCoord);
		if(dist<min){
			min=dist;
			minI=i;
		}
	}
	return moves[minI];
}
ChaseState::~ChaseState(){

}

////////////////////////// PowerPillTransition //////////////////////////
PowerPillTransition::PowerPillTransition(std::shared_ptr<FSMState> next) : _next(next) {}

bool PowerPillTransition::isValid(const GameState& gs) {
    return gs.isGhostEdible(0);
}

std::shared_ptr<FSMState> PowerPillTransition::getNextState() {
    return _next;
}

//////////////////////////// TimerTransition ////////////////////////////
TimerTransition::TimerTransition(std::shared_ptr<FSMState> next, double duration = 10.0) 
    : _next(next), _duration(duration), _running(false) {}

void TimerTransition::onTransition(const GameState& gs) {
    _start = std::chrono::high_resolution_clock::now();
    _running = true;
}

bool TimerTransition::isValid(const GameState& gs) {
    if (!_running) return false;
    
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = now - _start;
    
    if (diff.count() >= _duration) {
        _running = false; //Reset para el proximo uso
        return true;
    }
    return false;
}

std::shared_ptr<FSMState> TimerTransition::getNextState() {
    return _next;
}

FrightenedState::FrightenedState(std::shared_ptr<Character> _character) : FSMState(_character) {}

Move FrightenedState::onUpdate(const GameState& game) {

    auto moves = game.getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());

    if (moves.empty()) return PASS;

    return moves[rand() % moves.size()];

}





