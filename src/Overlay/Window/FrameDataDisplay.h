#pragma once
#include "IWindow.h"

#include "Core/interfaces.h"

class FrameDataDisplay : public IWindow
{
public:
	FrameDataDisplay(const std::string& windowTitle, bool windowClosable,
		ImGuiWindowFlags windowFlags)
		: IWindow(windowTitle, windowClosable, windowFlags) {}
	void Update() override;
	bool HasNullptrInData();

protected:
	void Draw() override;

private:
	void CompareNStates(class Player& player);
	void IsGuardEnd(class Player& player);
	void GetLastNeutral(class Player& player, bool isneutral);


	ImGuiWindowFlags m_overlayWindowFlags = ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoInputs
		| ImGuiWindowFlags_NoBringToFrontOnFocus
		| ImGuiWindowFlags_NoFocusOnAppearing;
};


