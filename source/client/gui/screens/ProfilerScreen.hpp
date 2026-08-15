#pragma once
#include "client/gui/Screen.hpp"

// The sole purpose of this screen is to unlock the mouse temporarily and allow interaction with the profiler
class ProfilerScreen : public Screen
{
public:
	ProfilerScreen();
	virtual ~ProfilerScreen();

private:
	ProfilerRenderer& _getProfilerRenderer();

	void _renderInfoText();

public:
	void render(float partialTicks) override;
	void tick() override;
	void handleUserAction(const ActionInfo& action) override;
	bool isPauseScreen() override
	{
		return false;
	}

private:
	bool m_bIsFrozen;
};
