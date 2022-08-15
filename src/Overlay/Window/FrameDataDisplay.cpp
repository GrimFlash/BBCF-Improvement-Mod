#include "FrameDataDisplay.h"

#include "Game/gamestates.h"

//Credit to mactonya for the initial framedata display build

bool IsNeutral = false;

void FrameDataDisplay::Update()
{
	if (HasNullptrInData() || !m_windowOpen)
	{
		return;
	}

	if (!isTrainingToolsEnabledInCurrentState())
	{
		return;
	}

	ImGui::Begin("##FrameDataDisplay", nullptr, m_overlayWindowFlags);

	Draw();

	ImGui::End();
}

void FrameDataDisplay::Draw() 
{
	CompareNStates(g_interfaces.player1);
	CompareNStates(g_interfaces.player2);
}

void FrameDataDisplay::CompareNStates(class Player& player)
{
	// Netural states 
	char* Nstates[7] = { "CmnActStand", "_NEUTRAL", "CmnActStand2Crouch", "CmnActCrouch2Stand", "CmnActCrouch", "CmnActFWalk", "CmnActBWalk" };
	IsNeutral = false;

	for (int i = 0; i < 7; i++)
	{
		IsNeutral = IsNeutral || (strcmp(player.GetData()->currentAction, Nstates[i]) == 0);
		if (IsNeutral)
		{
			GetLastNeutral(player, IsNeutral);
			break;
		}
		else if (i == 6)
			IsGuardEnd(player);
	}
}

void FrameDataDisplay::IsGuardEnd(class Player& player)
{
	char* output = NULL;
	output = strstr(player.GetData()->currentAction, "GuardEnd");
	IsNeutral = IsNeutral || output;
	GetLastNeutral(player, IsNeutral);
}

void FrameDataDisplay::GetLastNeutral(class Player& player, bool isneutral)
{
	if (isneutral && player.last_neutral < 0)
		player.last_neutral = *g_gameVals.pFrameCount;
	else if (!isneutral)
		player.last_neutral = -1;
}

bool FrameDataDisplay::HasNullptrInData()
{
	return !g_gameVals.pEntityList;
}