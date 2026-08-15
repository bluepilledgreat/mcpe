#include "client/app/Minecraft.hpp"
#include "client/renderer/ProfilerRenderer.hpp"
#include "common/Mth.hpp"
#include "client/gui/GuiElement.hpp"
#include "common/math/Color.hpp"
#include "renderer/ShaderConstants.hpp"

//#define PROFILE_OURSELVES
#ifdef PROFILE_OURSELVES
#define PROFILE_MYSELF PROFILE_FUNCTION // me when i profile myself
#else
#define PROFILE_MYSELF()
#endif

const double INTERVAL_BETWEEN_REBUILDS = 0.1;

const int ROW_HEIGHT = 12;
const float TEXT_SCALE = 1.0f;

class ProfilerFlameGraph;

class ProfilerGuiElement : public GuiComponent
{
	friend class ProfilerFlameGraph;

protected:
	ProfilerGuiElement* m_parent;
	Vector2I m_position;
	Vector2I m_size;

public:
	ProfilerGuiElement()
		: m_parent(nullptr)
	{
	}

	ProfilerGuiElement(
		const Vector2I& position,
		const Vector2I& size)
		: m_parent(nullptr)
		, m_position(position)
		, m_size(size)
	{
	}

	virtual ~ProfilerGuiElement()
	{
	}

public:
	const Vector2I& getSize() const
	{
		return m_size;
	}

	const Vector2I& getPosition() const
	{
		return m_position;
	}

	void setSize(const Vector2I& size)
	{
		m_size = size;
	}

	void setPosition(const Vector2I& position)
	{
		m_position = position;
	}

	bool isVisible() const
	{
		const Vector2I& size = getSize();
		if (size.x <= 0 || size.y <= 0)
			return false;

		return true;
	}
};

class ProfilerFlameGraphRow
{
private:
	// Border on all sides
	static const int BORDER_SIZE = 1;

	// Padding on all sides
	static const int PADDING = 1;

public:
	static const Color BORDER_COLOUR;
	static const Color BACKGROUND_COLOUR;
	static const Color BORDER_HIGHLIGHT_COLOUR;
	static const Color BACKGROUND_HIGHLIGHT_COLOUR;
	static const Color TOOLTIP_BACKGROUND_COLOUR;

private:
	std::string m_name;
	ScreenRenderer* m_renderer;
	Vector2I m_position;
	Vector2I m_size;
	Rect2DI m_rect;

public:
	ProfilerFlameGraphRow(
		const std::string& name,
		const Vector2I& position,
		const Vector2I& size)
		: m_name(name)
		, m_renderer(nullptr)
		, m_position(position)
		, m_size(size)
	{
	}

private:
	Rect2DI getRect() const
	{
		const Vector2I& size = getSize();
		const Vector2I& position = getPosition();

		return Rect2DI(position, position + size);
	}

	void renderText(Font& font, const std::string& text, const Rect2DI& rect, float scale)
	{
		PROFILE_MYSELF();

		if (text.empty())
			return;

		int maxWidth = rect.width();
		if (maxWidth <= 0)
			return;

		int width = static_cast<int>(font.width(text) * scale);
		if (maxWidth >= width)
		{
			font.drawSlowV2(text, rect.x0(), rect.y0(), Color::WHITE, false); // draw that supports batching
			return;
		}

		std::string newStr = font.getStringThatFitsInWidth(text, maxWidth);
		if (!newStr.empty())
		{
			font.drawSlowV2(newStr, rect.x0(), rect.y0(), Color::WHITE, false); // draw that supports batching
		}
	}

	void renderTooltip(Minecraft* minecraft, const std::string& text, const Vector2I& pointerPos)
	{
		PROFILE_MYSELF();

		const int TOOLTIP_PADDING = 3; // Padding on each side of the tooltip
		const int TEXT_X_OFFSET = 8; // Text offset from mouse on the X dimension
		const int TEXT_Y_OFFSET = 10; // Text offset from mouse on the Y dimension

		int textWidth = minecraft->m_pFont->width(text);
		int textHeight = minecraft->m_pFont->height(text, INT32_MAX);

		// calculate where to put the tooltip
		Vector2I textPos(pointerPos.x, pointerPos.y);

		Rect2DI drawRect;

		if ((pointerPos.x + textWidth + TEXT_X_OFFSET + TOOLTIP_PADDING) <= Gui::GuiWidth)
		{
			drawRect.min.x = pointerPos.x + TEXT_X_OFFSET - TOOLTIP_PADDING;
			drawRect.max.x = pointerPos.x + textWidth + TEXT_X_OFFSET + TOOLTIP_PADDING;
			textPos.x += TEXT_X_OFFSET;
		}
		else
		{
			drawRect.min.x = pointerPos.x - textWidth - TEXT_X_OFFSET - TOOLTIP_PADDING;
			drawRect.max.x = pointerPos.x - TEXT_X_OFFSET + TOOLTIP_PADDING;
			textPos.x -= TEXT_X_OFFSET;
			textPos.x -= textWidth;
		}

		if ((pointerPos.y - TEXT_Y_OFFSET - TOOLTIP_PADDING) >= 0)
		{
			drawRect.min.y = pointerPos.y - TEXT_Y_OFFSET - TOOLTIP_PADDING;
			drawRect.max.y = pointerPos.y + textHeight - TEXT_Y_OFFSET + TOOLTIP_PADDING;
			textPos.y -= TEXT_Y_OFFSET;
		}
		else
		{
			drawRect.min.y = pointerPos.y - textHeight + TEXT_Y_OFFSET - TOOLTIP_PADDING;
			drawRect.max.y = pointerPos.y + TEXT_Y_OFFSET + TOOLTIP_PADDING;
			textPos.y += TEXT_Y_OFFSET;
			textPos.y -= textHeight;
		}

		m_renderer->fill(drawRect, TOOLTIP_BACKGROUND_COLOUR);
		m_renderer->drawString(*minecraft->m_pFont, text, textPos.x, textPos.y);
	}

	void drawRect(const Rect2DI& rect)
	{
		Tesselator& t = Tesselator::instance;

		t.vertex(rect.x0(), rect.y1(), 0.0f);
		t.vertex(rect.x1(), rect.y1(), 0.0f);
		t.vertex(rect.x1(), rect.y0(), 0.0f);
		t.vertex(rect.x0(), rect.y0(), 0.0f);
	}

public:
	const std::string& getName() const
	{
		return m_name;
	}

	const Vector2I& getSize() const
	{
		return m_size;
	}

	const Vector2I& getPosition() const
	{
		return m_position;
	}

	void setSize(const Vector2I& size)
	{
		m_size = size;
	}

	void setPosition(const Vector2I& position)
	{
		m_position = position;
	}

	void setRenderer(ScreenRenderer* renderer)
	{
		m_renderer = renderer;
	}

	void preload()
	{
		m_rect = getRect();
	}

	bool isHovering(const Vector2I& pointer) const
	{
		Rect2DI rect = m_rect;
		// we need to offset the rectangle to prevent multiple rows being hovered
		//rect.min += Vector2I(1, 1);
		rect.max -= Vector2I(1, 1);

		return rect.contains(Vector2I(pointer.x, pointer.y));
	}

	bool renderBorder()
	{
		if (m_rect.width() > 0 && m_rect.height() > 0)
		{
			drawRect(m_rect);
			return true;
		}

		return false;
	}

	bool renderBackground()
	{
		Rect2DI backgroundRect = m_rect;
		backgroundRect.min += Vector2I(1, 1) * BORDER_SIZE;
		backgroundRect.max -= Vector2I(1, 1) * BORDER_SIZE;

		if (backgroundRect.width() > 0 && backgroundRect.height() > 0)
		{
			drawRect(backgroundRect);
			return true;
		}

		return false;
	}

	bool renderName(Font& font)
	{
		Rect2DI textRect = m_rect;
		textRect.min += Vector2I(1, 1) * BORDER_SIZE;
		textRect.max -= Vector2I(1, 1) * BORDER_SIZE;
		textRect.min += Vector2I(1, 1) * PADDING;
		textRect.max -= Vector2I(6, 1) * PADDING; // -6 on x so it stops rendering out the row (Font::width is returning bad results?)

		if (textRect.width() > 0 && textRect.height() > 0)
		{
			renderText(font, m_name, textRect, TEXT_SCALE);
			return true;
		}

		return false;
	}

	void renderHovering(Minecraft* pMinecraft, const Vector2I& pointerPos)
	{
		PROFILE_MYSELF();

		renderTooltip(pMinecraft, m_name, pointerPos);
	}
};

class ProfilerFlameGraph : public ProfilerGuiElement
{
private:
	std::vector<ProfilerFlameGraphRow*> m_children;

public:
	ProfilerFlameGraph(
		const Vector2I& position,
		const Vector2I& size)
		: ProfilerGuiElement(position, size)
	{
	}

	virtual ~ProfilerFlameGraph()
	{
	}

public:
	void render(Minecraft* pMinecraft, const MenuPointer& pointer)
	{
		PROFILE_MYSELF();

		ProfilerFlameGraphRow* hoveringElement = nullptr;

		{
			Vector2I pointerPos(-1, -1);
			if (pointer.x != -1 && pointer.y != -1)
			{
				// scale back to normal window dimensions
				// this is not a perfect solution as we can't hover over certain ui elements
				pointerPos.x = static_cast<int>(floorf(pointer.x / Gui::GuiScale));
				pointerPos.y = static_cast<int>(floorf(pointer.y / Gui::GuiScale));
			}

			MatrixStack::Ref projRef = MatrixStack::Projection.pushIdentity();
			projRef->setOrtho(0, Minecraft::width, Minecraft::height, 0, 1000.0f, 3000.0f);

			Tesselator& t = Tesselator::instance;

			size_t maxTextVertices = 0;

			// DRAW BORDERS + COLLECT DATA
			t.begin(4 * m_children.size());

			currentShaderColor = ProfilerFlameGraphRow::BORDER_COLOUR;

			for (std::vector<ProfilerFlameGraphRow*>::const_iterator it = m_children.begin(); it != m_children.end(); it++)
			{
				ProfilerFlameGraphRow* row = *it;
				row->preload();

				if (row->isHovering(pointerPos))
				{
					assert(!hoveringElement);
					hoveringElement = row;
				}

				if (row != hoveringElement)
					row->renderBorder();
			}

			t.draw(m_materials.ui_fill_color);

			// DRAW BACKGROUND
			t.begin(4 * m_children.size());

			currentShaderColor = ProfilerFlameGraphRow::BACKGROUND_COLOUR;

			for (std::vector<ProfilerFlameGraphRow*>::const_iterator it = m_children.begin(); it != m_children.end(); it++)
			{
				ProfilerFlameGraphRow* row = *it;

				if (row != hoveringElement)
				{
					bool rendered = row->renderBackground();
					if (rendered) // we only want to account for the vertices of rows that are capable of rendering the background
						maxTextVertices += 4 * row->getName().size() + 4 * 2; // 8 extra for ellipsis (for cases where only one character is cut off)
				}
			}

			t.draw(m_materials.ui_fill_color);

			if (hoveringElement)
			{
				// draw separately as we need to use different colours

				// DRAW BORDER
				t.begin(4);
				currentShaderColor = ProfilerFlameGraphRow::BORDER_HIGHLIGHT_COLOUR;
				hoveringElement->renderBorder();
				t.draw(m_materials.ui_fill_color);

				// DRAW BACKGROUND
				t.begin(4);
				currentShaderColor = ProfilerFlameGraphRow::BACKGROUND_HIGHLIGHT_COLOUR;
				bool rendered = hoveringElement->renderBackground();
				if (rendered)
					maxTextVertices += 4 * hoveringElement->getName().size() + 4 * 2;
				t.draw(m_materials.ui_fill_color);
			}

			// DRAW NAMES
			t.begin(maxTextVertices);
			t.voidBeginAndEndCalls(true);

			currentShaderColor = Color::WHITE;

			for (std::vector<ProfilerFlameGraphRow*>::const_iterator it = m_children.begin(); it != m_children.end(); it++)
			{
				ProfilerFlameGraphRow* row = *it;
				row->renderName(*pMinecraft->m_pFont);
			}

			t.voidBeginAndEndCalls(false);
			t.draw(pMinecraft->m_pFont->getTextMaterial());
		}

		// have tooltips not clip by rendering last
		if (hoveringElement)
		{
			Vector2I pointerPos(
				floorf(pointer.x),
				floorf(pointer.y)
			);
			hoveringElement->renderHovering(pMinecraft, pointerPos);
		}
	}

	void nukeAllChildren()
	{
		for (std::vector<ProfilerFlameGraphRow*>::const_iterator it = m_children.begin(); it != m_children.end(); it++)
			delete (*it);

		m_children.clear();
	}

	void addChild(ProfilerFlameGraphRow* child)
	{
		m_children.push_back(child);
		child->setRenderer(this);
	}
};

const Color ProfilerFlameGraphRow::BORDER_COLOUR = Color::FromRGB(100, 0, 0);
const Color ProfilerFlameGraphRow::BACKGROUND_COLOUR = Color::FromRGB(200, 0, 0);
const Color ProfilerFlameGraphRow::BORDER_HIGHLIGHT_COLOUR = Color::FromRGB(150, 0, 0);
const Color ProfilerFlameGraphRow::BACKGROUND_HIGHLIGHT_COLOUR = Color::FromRGB(250, 0, 0);
const Color ProfilerFlameGraphRow::TOOLTIP_BACKGROUND_COLOUR = Color::FromRGBA(0, 0, 0, 192);

ProfilerRenderer::ProfilerRenderer(Minecraft* minecraft)
	: m_minecraft(minecraft)
	, m_lastRebuild(DBL_MIN)
	, m_flameGraphContainer(new ProfilerFlameGraph(Vector2I(0, 0), Vector2I(300, 300)))
{

}

void ProfilerRenderer::buildFlameGraphUI(const ProfilerFunctionFrame& frame, const Vector2I& parentPosition, const Vector2I& parentSize)
{
	if (frame.getElapsedTime() == 0.0)
		return;

	// build leafs
	int pixelOffset = 0;
	for (std::vector<ProfilerFunctionFrame>::const_iterator it = frame.getChildFrames().begin(); it != frame.getChildFrames().end(); it++)
	{
		const ProfilerFunctionFrame& childFrame = *it;
		float xScale = static_cast<float>(childFrame.getElapsedTime() / frame.getElapsedTime());

		// clamp
		int pixelSize = Mth::clamp(static_cast<int>(parentSize.x * xScale), 0, parentSize.x - pixelOffset);

		ProfilerFlameGraphRow* row = new ProfilerFlameGraphRow(
			childFrame.getName(),
			Vector2I(parentPosition.x + pixelOffset, frame.getDepth() * ROW_HEIGHT),
			Vector2I(pixelSize, ROW_HEIGHT)
		);

		m_flameGraphContainer->addChild(row);
		pixelOffset += pixelSize;

		buildFlameGraphUI(childFrame, row->getPosition(), row->getSize());

		if (pixelOffset == parentSize.x)
			break;
	}
}

void ProfilerRenderer::rebuildFlameGraphUI()
{
	m_flameGraphContainer->nukeAllChildren();

	buildFlameGraphUI(m_profilerFrame, m_flameGraphContainer->getPosition(), m_flameGraphContainer->getSize());
}

void ProfilerRenderer::dumpFrame(ProfilerContextRoot* contextRoot)
{
	contextRoot->dump(m_profilerFrame);
}

// TODO: remove once multiple thread rendering is supported
void ProfilerRenderer::dumpCurrentThreadFrame()
{
	ProfilerContextRoot* rootContext = nullptr;

	ProfilerThreadId_t currentThreadId = GetCurrentThreadId();
	
	{
		ProfilerContextRegistry& registry = ProfilerContextRegistry::singleton;

		SharedLock<SharedMutex> lock(registry.m_mutex);
		for (std::vector<ProfilerContextRoot*>::const_iterator it = registry.getContextRoots().begin(); it != registry.getContextRoots().end(); it++)
		{
			ProfilerContextRoot* context = *it;
			if (context->getThreadId() == currentThreadId)
			{
				rootContext = context;
				break;
			}
		}
	}

	if (rootContext)
		dumpFrame(rootContext);
}

void ProfilerRenderer::updateContainerPosition()
{
	m_flameGraphContainer->setSize(Vector2I(
		Minecraft::width / 2,
		m_flameGraphContainer->getSize().y
	));

	m_flameGraphContainer->setPosition(Vector2I(
		Minecraft::width - m_flameGraphContainer->getSize().x,
		m_flameGraphContainer->getPosition().y
	));
}

void ProfilerRenderer::renderFlameGraphUI(const MenuPointer& pointer)
{
	m_flameGraphContainer->render(m_minecraft, pointer);
}

void ProfilerRenderer::step()
{
	PROFILE_MYSELF();

	if (getTimeS() > m_lastRebuild + INTERVAL_BETWEEN_REBUILDS)
	{
		dumpCurrentThreadFrame();
		rebuildFlameGraphUI();
		m_lastRebuild = getTimeS();
	}
}

void ProfilerRenderer::render(const MenuPointer& pointer)
{
	PROFILE_MYSELF();

	updateContainerPosition();
	renderFlameGraphUI(pointer);
}
