#include "InkyController.h"
#include <cmath>
#include <algorithm>

InkyController::InkyController(std::shared_ptr<Character> character):
	Controller(character)	{}

InkyController::~InkyController() {}

//encuentra el movimiento legal que mas se acerque al objetivo
Move InkyController::getClosestMove(const GameState& game, std::pair<int,int> target) const 
{
	int minDist = 10000000;
	Move minMove = character->getDirection();
	std::vector<Move> moves = game.getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
	
	if(moves.empty()) return PASS;

	for(Move m : moves)
	{
		int vecino = game.getMaze().getNeighbour(character->getPos(), m);
		if(vecino < 0) continue;
		auto vecinoCoords = game.getMaze().getNodePos(vecino);
		int sqDist = euclid2(vecinoCoords, target);
		if(sqDist < minDist)
		{
			minDist = sqDist;
			minMove = m;
		}
	}
	return minMove;
}

//encuentra el movimiento "legal" que mas se aleje del objetivo
Move InkyController::getFarthestMove(const GameState& game, std::pair<int,int> target) const 
{
	int maxDist = -1;
	Move maxMove = character->getDirection();
	std::vector<Move> moves = game.getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
	
	if(moves.empty()) return PASS;

	for(Move m : moves)
	{
		int vecino = game.getMaze().getNeighbour(character->getPos(), m);
		if(vecino < 0) continue;
		auto vecinoCoords = game.getMaze().getNodePos(vecino);
		int sqDist = euclid2(vecinoCoords, target);
		if(sqDist > maxDist)
		{
			maxDist = sqDist;
			maxMove = m;
		}
	}
	return maxMove;
}

float InkyController::getDistanceToPacman(const GameState& game) const 
{
	return sqrt(euclid2(
		game.getMaze().getNodePos(character->getPos()),
		game.getMaze().getNodePos(game.getPacmanPos())));
}

//funcion campana (Gaussiana): Inky prefiere emboscar cuando no esta ni muy cerca ni muy lejos.
float InkyController::calculateAmbushUtility(float distance) const 
{
	//centrada en una distancia ideal de 16 celdas
	return exp(-pow(distance - 16.0f, 2) / (2 * pow(8.0f, 2)));
}

//funcion logistica: el miedo a Ms. Pac-Man sube drasticamente si rompe el umbral de 10 celdas.
float InkyController::calculateRetreatUtility(float distance) const 
{
	return 1.0f / (1.0f + exp(0.5f * (distance - 10.0f)));
}

Move InkyController::getMove(const GameState& game) 
{
	//se busca el ID de Inky en el juego (tipicamente indice 2)
	int myGhostIndex = 2; 
	
	int myNode = character->getPos();
	auto myCoords = game.getMaze().getNodePos(myNode);
	auto pacmanCoords = game.getMaze().getNodePos(game.getPacmanPos());
	
	//requerimiento especial: si esta asustado (azul), la utilidad de huir es absoluta (1.0)
	if (game.isGhostEdible(myGhostIndex)) 
	{
		return getFarthestMove(game, pacmanCoords);
	}

	float distance = getDistanceToPacman(game);

	//calcula utilidades basandonos en nuestras curvas justificadas
	float ambushUtility = calculateAmbushUtility(distance);
	float retreatUtility = calculateRetreatUtility(distance);

	//determina la accion con el valor de utilidad mas alto
	if (retreatUtility > ambushUtility) 
	{
		//accion A: huye a su esquina designada de dispersion de forma segura (Esquina inferior derecha)
		std::pair<int, int> safetyCorner = {27, 31};
		return getClosestMove(game, safetyCorner);
	} else 
	{
		//accion B: Emboscada Clasica
		//apunta al doble del vector que va desde Blinky (ID 0) hasta 2 celdas delante de Pac-Man
		auto blinkyCoords = game.getMaze().getNodePos(game.getGhostsPos(0));
		Move pacmanDir = static_cast<Move>(game.getPacmanDir());
		
		std::pair<int, int> frontCells = pacmanCoords;
		if (pacmanDir == UP) frontCells.second -= 2;
		else if (pacmanDir == DOWN) frontCells.second += 2;
		else if (pacmanDir == LEFT) frontCells.first -= 2;
		else if (pacmanDir == RIGHT) frontCells.first += 2;

		std::pair<int, int> ambushTarget = 
		{
			frontCells.first + (frontCells.first - blinkyCoords.first),
			frontCells.second + (frontCells.second - blinkyCoords.second)
		};

		return getClosestMove(game, ambushTarget);
	}
}