#pragma once
#include <list>
#include <string>

#include "IWindow.h"

#include "Game/CharData.h"

class FrameDataWindow : public IWindow
{
public:
	FrameDataWindow(const std::string& windowTitle, bool windowClosable,
		ImGuiWindowFlags windowFlags = 0)
		: IWindow(windowTitle, windowClosable, windowFlags) {}
	~FrameDataWindow() override = default;

protected:
	void Draw() override;

private:
	bool isDoingActionInList(const char currentAction[], const std::list<std::string>& listOfActions);
	bool isIdle(CharData& player, class Player& playerClass);
	bool isBlocking(CharData& player);
	bool isInHitstun(CharData& player);
	void getFrameAdvantage(CharData& player1, CharData& player2);
	void computeGaps(CharData& player, int& gapCounter, int& gapResult);
	bool hasWorldTimeMoved();
	void computeFramedataInteractions();
	bool HasNullptrInData();
};

struct PlayersInteractionState
{
	// World Time
	int prevFrameCount = 0;

	// Frame advantage
	int timer = 0;
	int frameAdvantageToDisplay = 0;
	bool started = false;

	// Gap
	bool inBlockstring = false;
	int p1Gap = -1;
	int p2Gap = -1;
	int p1GapDisplay = -1;
	int p2GapDisplay = -1;
};

extern PlayersInteractionState interaction;