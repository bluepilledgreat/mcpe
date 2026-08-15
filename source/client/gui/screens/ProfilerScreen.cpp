#include "client/gui/screens/ProfilerScreen.hpp"

ProfilerScreen::ProfilerScreen()
	: m_bIsFrozen(false)
{
	m_bRenderPointer = true;
}

ProfilerScreen::~ProfilerScreen()
{
}

ProfilerRenderer& ProfilerScreen::_getProfilerRenderer()
{
	return m_pMinecraft->m_pGameRenderer->getProfilerRenderer();
}

void ProfilerScreen::_renderInfoText()
{
	static std::string text = "PROFILER MODE";

	int heightOffset = 0;

	drawString(*m_pMinecraft->m_pFont, text, 0, m_height - Font::LINE_HEIGHT - heightOffset, Color::RED);
	heightOffset += Font::LINE_HEIGHT;

	if (m_bIsFrozen)
	{
		static std::string frozenText = "FRAME FROZEN";

		drawString(*m_pMinecraft->m_pFont, frozenText, 0, m_height - Font::LINE_HEIGHT - heightOffset, Color::BLUE);
		heightOffset += Font::LINE_HEIGHT;
	}
}

void ProfilerScreen::render(float partialTicks)
{
	_renderInfoText();
	if (!m_bIsFrozen)
		_getProfilerRenderer().step();
	_getProfilerRenderer().render(m_menuPointer);
}

void ProfilerScreen::tick()
{
	Screen::tick();

	Options& options = *m_pMinecraft->getOptions();

	if (!options.m_debugProfiler.get())
	{
		m_pMinecraft->handleBack(false);
	}
}

void ProfilerScreen::handleUserAction(const ActionInfo& action)
{
	Options& options = *m_pMinecraft->getOptions();

	if (options.isAction(AID_PROFILERMODE_FREEZE, action))
	{
		m_bIsFrozen = !m_bIsFrozen;
		return;
	}
	else if (options.isAction(AID_TOGGLEPROFILERMODE, action))
	{
		m_pMinecraft->handleBack(false);
		return;
	}

	Screen::handleUserAction(action);
}
