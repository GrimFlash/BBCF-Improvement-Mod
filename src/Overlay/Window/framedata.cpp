#include "framedata.h"
#include "Game/CharData.h"
#include "Core/interfaces.h"
#include "Game/gamestates.h"

PlayersInteractionState interaction;

const std::list<std::string> idleWords =
{ "_NEUTRAL", "CmnActStand", "CmnActStandTurn", "CmnActStand2Crouch",
"CmnActCrouch", "CmnActCrouchTurn", "CmnActCrouch2Stand",
"CmnActFWalk", "CmnActBWalk",
"CmnActFDash", "CmnActFDashStop",
"CmnActJumpLanding", "CmnActLandingStiffEnd",
"CmnActUkemiLandNLanding", "CmnActUkemiStagger",
// Proxi block is triggered when an attack is closing in without being actually blocked
// If the player.blockstun is = 0, then those animations are still considered idle
"CmnActCrouchGuardPre", "CmnActCrouchGuardLoop", "CmnActCrouchGuardEnd",                 // Crouch
"CmnActCrouchHeavyGuardPre", "CmnActCrouchHeavyGuardLoop", "CmnActCrouchHeavyGuardEnd",  // Crouch Heavy
"CmnActMidGuardPre", "CmnActMidGuardLoop", "CmnActMidGuardEnd",                          // Mid
"CmnActMidHeavyGuardPre", "CmnActMidHeavyGuardLoop", "CmnActMidHeavyGuardEnd",           // Mid Heavy
"CmnActHighGuardPre", "CmnActHighGuardLoop", "CmnActHighGuardEnd",                       // High
"CmnActHighHeavyGuardPre", "CmnActHighHeavyGuardLoop", "CmnActHighHeavyGuardEnd",        // High Heavy
"CmnActAirGuardPre", "CmnActAirGuardLoop", "CmnActAirGuardEnd",                          // Air
// Character specifics
"com3_kamae" // Mai 5xB stance
};

void FrameDataWindow::Draw()
{
    ImVec4 color;
    ImVec4 white(1.0f, 1.0f, 1.0f, 1.0f);
    ImVec4 red(1.0f, 0.0f, 0.0f, 1.0f);
    ImVec4 green(0.0f, 1.0f, 0.0f, 1.0f);

    if (!isTrainingToolsEnabledInCurrentState() || HasNullptrInData())
    {
        Close();
        return;
    }

    computeFramedataInteractions();

    ImGui::SetWindowSize(ImVec2(220, 100), ImGuiCond_FirstUseEver);
    ImGui::SetWindowPos(ImVec2(350, 250), ImGuiCond_FirstUseEver);

    ImGui::Columns(2, "columns_layout", true);

    // First column
    if (interaction.frameAdvantageToDisplay > 0)
        color = green;
    else if (interaction.frameAdvantageToDisplay < 0)
        color = red;
    else
        color = white;

    ImGui::Text("Player 1");
    ImGui::TextUnformatted("Gap:");
    ImGui::SameLine();
    ImGui::TextUnformatted(((interaction.p1GapDisplay != -1) ? std::to_string(interaction.p1GapDisplay) : "").c_str());

    ImGui::TextUnformatted("Advantage:");
    ImGui::SameLine();
    std::string str = std::to_string(interaction.frameAdvantageToDisplay);
    if (interaction.frameAdvantageToDisplay > 0)
        str = "+" + str;

    ImGui::TextColored(color, "%s", str.c_str());

    // Next column
    if (interaction.frameAdvantageToDisplay > 0)
        color = red;
    else if (interaction.frameAdvantageToDisplay < 0)
        color = green;
    else
        color = white;

    ImGui::NextColumn();
    ImGui::Text("Player 2");
    ImGui::TextUnformatted("Gap:");
    ImGui::SameLine();
    ImGui::TextUnformatted(((interaction.p2GapDisplay != -1) ? std::to_string(interaction.p2GapDisplay) : "").c_str());

    ImGui::TextUnformatted("Advantage:");
    ImGui::SameLine();
    std::string str2 = std::to_string(-interaction.frameAdvantageToDisplay);
    if (interaction.frameAdvantageToDisplay < 0)
        str2 = "+" + str2;
    ImGui::TextColored(color, "%s", str2.c_str());
}

bool FrameDataWindow::isDoingActionInList(const char currentAction[], const std::list<std::string>& listOfActions)
{
    const std::string currentActionString = currentAction;

    for (auto word : listOfActions)
    {
        if (currentActionString == word)
        {
            return true;
        }
    }
    return false;
}

bool FrameDataWindow::isIdle(CharData& player, class Player& playerClass)
{
    if (isBlocking(player) || isInHitstun(player))
        return false;

    if (isDoingActionInList(player.currentAction, idleWords))
    {
        playerClass.whiffBlockCancel = false;
        return true;
    }
    else if (playerClass.whiffBlockCancel == true)
    {
        return true;
    }
    
    return false;
}

bool FrameDataWindow::isBlocking(CharData& player)
{
    if (player.blockstun > 0 || player.moveSpecialBlockstun > 0)
        return true;
    return false;
}

bool FrameDataWindow::isInHitstun(CharData& player)
{
    if (player.hitstun > 0 && !isDoingActionInList(player.currentAction, idleWords))
        return true;
    return false;
}

void FrameDataWindow::getFrameAdvantage(CharData& player1, CharData& player2)
{
    bool isIdle1 = isIdle(player1, g_interfaces.player1);
    bool isIdle2 = isIdle(player2, g_interfaces.player2);
    bool isStunned = isBlocking(player2) || isInHitstun(player2);

    if (!isIdle1 && !isIdle2)
    {
        interaction.started = true;
        interaction.timer = 0;
    }
    if (interaction.started)
    {
        if (isIdle1 && isIdle2)
        {
            interaction.started = false;
            interaction.frameAdvantageToDisplay = interaction.timer;
        }
        if (!isIdle1)
        {
            interaction.timer -= 1;
        }
        if (!isIdle2)
        {
            interaction.timer += 1;
        }
    }
}

void FrameDataWindow::computeGaps(CharData& player, int& gapCounter, int& gapResult)
{
    interaction.inBlockstring = isBlocking(player) || isInHitstun(player);
 
    if (interaction.inBlockstring)
    {
        if (gapCounter > 0 && gapCounter <= 30)
        {
            gapResult = gapCounter;
        }
        gapCounter = 0; //resets everytime you are in block or hit stun
    }
    else if (!interaction.inBlockstring)
    {
        ++gapCounter;
        gapResult = -1;
    }
}

bool FrameDataWindow::hasWorldTimeMoved()
{
    if (interaction.prevFrameCount < *g_gameVals.pFrameCount)
    {
        interaction.prevFrameCount = *g_gameVals.pFrameCount;
        return true;
    }

    interaction.prevFrameCount = *g_gameVals.pFrameCount;
    return false;
}

void FrameDataWindow::computeFramedataInteractions()
{
    if (!(*g_gameVals.pGameMode == GameMode_Training || *g_gameVals.pGameMode == GameMode_ReplayTheater))
        return;

    if (!g_interfaces.player1.IsCharDataNullPtr() && !g_interfaces.player2.IsCharDataNullPtr())
    {
        CharData& player1 = *g_interfaces.player1.GetData();
        CharData& player2 = *g_interfaces.player2.GetData();

        if (hasWorldTimeMoved())
        {
            computeGaps(player1, interaction.p1Gap, interaction.p1GapDisplay);
            computeGaps(player2, interaction.p2Gap, interaction.p2GapDisplay);

            getFrameAdvantage(player1, player2);
        }
    }
}

bool FrameDataWindow::HasNullptrInData()
{
    return g_interfaces.player1.IsCharDataNullPtr() ||
        g_interfaces.player2.IsCharDataNullPtr();
}