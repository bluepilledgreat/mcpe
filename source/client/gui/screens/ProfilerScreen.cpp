#include "client/gui/screens/ProfilerScreen.hpp"

ProfilerScreen::ProfilerScreen()
	: m_isFrozen(false)
{
}

ProfilerScreen::~ProfilerScreen()
{
}

ProfilerRenderer& ProfilerScreen::getProfilerRenderer()
{
	return m_pMinecraft->m_pGameRenderer->getProfilerRenderer();
}

void ProfilerScreen::renderInfoText()
{
	static std::string text = "PROFILER MODE";

	int heightOffset = 0;

	drawString(*m_pMinecraft->m_pFont, text, 0, m_height - Font::LINE_HEIGHT - heightOffset, Color::RED);
	heightOffset += Font::LINE_HEIGHT;

	if (m_isFrozen)
	{
		static std::string frozenText = "FRAME FROZEN";

		drawString(*m_pMinecraft->m_pFont, frozenText, 0, m_height - Font::LINE_HEIGHT - heightOffset, Color::BLUE);
		heightOffset += Font::LINE_HEIGHT;
	}
}

void ProfilerScreen::render(float partialTicks)
{
	renderInfoText();
	if (!m_isFrozen)
		getProfilerRenderer().step();
	getProfilerRenderer().render(m_menuPointer);
}

void ProfilerScreen::handleUserAction(const ActionInfo& action)
{
	if (m_pMinecraft->getOptions()->isKey(AID_PROFILERMODE_FREEZE, Keyboard::getEventKey()))
	{
		m_isFrozen = !m_isFrozen;
		return;
	}

	Screen::handleUserAction(action);
}
