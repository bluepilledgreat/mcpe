#pragma once
#include "client/gui/Screen.hpp"

// The sole purpose of this screen is to unlock the mouse temporarily and allow interaction with the profiler
class ProfilerScreen : public Screen
{
private:
	bool m_isFrozen;

public:
	ProfilerScreen();
	virtual ~ProfilerScreen();

private:
	ProfilerRenderer& getProfilerRenderer();

	void renderInfoText();

public:
	void render(float partialTicks) override;
	void handleUserAction(const ActionInfo& action) override;
	bool isPauseScreen() override
	{
		return false;
	}
};
