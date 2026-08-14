#pragma once
#include <vector>
#include "common/profiler/Profiler.hpp"
#include "common/math/Vector2.hpp"

class Minecraft;
class ProfilerFlameGraph;

class ProfilerRenderer
{
private:
	Minecraft* m_minecraft;
	double m_lastRebuild;
	ProfilerFlameGraph* m_flameGraphContainer;
	ProfilerFrame m_profilerFrame;

public:
	ProfilerRenderer(Minecraft* minecraft);

private:
	void buildFlameGraphUI(const ProfilerFunctionFrame& frame, const Vector2I& parentPosition, const Vector2I& parentSize);
	void rebuildFlameGraphUI();
	void renderFlameGraphUI(const MenuPointer& pointer);
	void dumpFrame(ProfilerContextRoot* contextRoot);
	void dumpCurrentThreadFrame();
	void updateContainerPosition();

public:
	void step();
	void render(const MenuPointer& pointer);
};
