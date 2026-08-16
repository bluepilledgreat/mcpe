/********************************************************************
	Minecraft: Pocket Edition - Decompilation Project
	Copyright (C) 2023 iProgramInCpp
	
	The following code is licensed under the BSD 1 clause license.
	SPDX-License-Identifier: BSD-1-Clause
 ********************************************************************/

#include "Font.hpp"
#include "client/renderer/renderer/RenderMaterialGroup.hpp"
#include "renderer/ShaderConstants.hpp"
#include "renderer/MatrixStack.hpp"
#include "common/Util.hpp"
#include "common/utility/hashing/HashCombine.hpp"
#include <sstream>
#include <utf8proc.h>

constexpr int MAX_CACHE_SIZE = 500;

constexpr char COLOR_START_CHAR = '\xa7';

constexpr uint8_t SPACE_WIDTH = 2;
constexpr float NEW_LINE_SPACING = 2.0f; // spacing on the Y-axis created by new lines

// character to use for characters not in our valid ranges
constexpr int UNK_CHAR = 65533;

template<>
struct HashFunction<Color>
{
	size_t operator()(const Color& key) const
	{
		return key.toUInt32();
	}
};

size_t HashFunction<FontCacheKey>::operator()(const FontCacheKey& key) const
{
	size_t hash = HashFunction<std::string>()(key.str);
	hash_combine(hash, key.color);
	return hash;
}

Font::Materials::Materials()
{
	MATERIAL_PTR(common, ui_text);
}

Font::GlyphQuad::GlyphQuad(int c, float x, float y, bool isAscii)
	: c(c)
	, x(x)
	, y(y)
	, isAscii(isAscii)
{
}

void Font::GlyphQuad::append(Tesselator& t)
{
	const int mapGlyphSize = isAscii ? ASCII_MAP_GLYPH_SIZE : UNICODE_MAP_GLYPH_SIZE;

	const int u = (c % COMMON_MAP_DIMENSION) * mapGlyphSize;
	const int v = (c / COMMON_MAP_DIMENSION) * mapGlyphSize;

	const int mapSize = isAscii ? ASCII_MAP_PIXEL_DIMENSION : UNICODE_MAP_PIXEL_DIMENSION;
	const float D = (1.0f / mapSize);

	t.vertexUV(x,                     y + RENDER_GLYPH_SIZE, 0.0f, u * D,                  (v + mapGlyphSize) * D);
	t.vertexUV(x + RENDER_GLYPH_SIZE, y + RENDER_GLYPH_SIZE, 0.0f, (u + mapGlyphSize) * D, (v + mapGlyphSize) * D);
	t.vertexUV(x + RENDER_GLYPH_SIZE, y,                     0.0f, (u + mapGlyphSize) * D, v * D);
	t.vertexUV(x,                     y,                     0.0f, u * D,                  v * D);
}

Font::TextObject::TextObject()
{
}

Font::TextObject::~TextObject()
{
}

void Font::TextObject::addPage(mce::Mesh& mesh, TextureData* textureData)
{
	assert(textureData);
	pages.push_back(Page(mesh, textureData));
}

void Font::TextObject::render(const mce::MaterialPtr& material)
{
	for (std::vector<Page>::iterator it = pages.begin(); it != pages.end(); it++)
	{
		Page& page = *it;
		page.textureData->bind();
		page.mesh.render(material);
	}
}

Font::TextObject::Page::Page(mce::Mesh& mesh, TextureData* textureData)
	: mesh(mesh)
	, textureData(textureData)
{
}

Font::Font(Options* options, const std::string& fileName, Textures* textures)
	: m_asciiFileName(fileName)
	, m_options(options)
	, m_textures(textures)
	, m_cachingEnabled(true)
{
	m_recentTextObjectCaches.reserve(MAX_CACHE_SIZE);
	_init(options);
}

void Font::_init(Options* pOpts)
{
	_computeAsciiSizes();
	_readUnicodeSizes("assets/font/glyphs/glyph_sizes.bin");
}

void Font::_computeAsciiSizes()
{
	TextureData* defaultTexture = m_textures->getTextureData(m_asciiFileName, false);
	if (!defaultTexture)
		throw std::runtime_error("Missing ASCII font image");

	if (defaultTexture->m_imageData.m_width != ASCII_MAP_PIXEL_DIMENSION && defaultTexture->m_imageData.m_height != ASCII_MAP_PIXEL_DIMENSION)
		throw std::runtime_error("Bad ASCII font image: wrong dimensions");

	for (int i = 0; i < NUM_ASCII_CHARS; ++i)
	{
		uint8_t c = static_cast<uint8_t>(i);
		uint8_t widthMax = 0;

		if (c == ' ')
		{
			widthMax = SPACE_WIDTH;
		}
		else
		{
			int x = c % COMMON_MAP_DIMENSION;
			int y = c / COMMON_MAP_DIMENSION;

			int pixelDataIndexBase = (ASCII_MAP_GLYPH_SIZE * x) + (ASCII_MAP_PIXEL_DIMENSION * ASCII_MAP_GLYPH_SIZE * y);

			for (int xOffset = ASCII_MAP_GLYPH_SIZE - 1; xOffset >= 0; --xOffset)
			{
				for (int yOffset = 0; yOffset < ASCII_MAP_GLYPH_SIZE; ++yOffset)
				{
					uint32_t pixelData = defaultTexture->getData()[pixelDataIndexBase + xOffset + (ASCII_MAP_PIXEL_DIMENSION * yOffset)];
					if (static_cast<uint8_t>(pixelData) != 0) // check for channel data
					{
						widthMax = xOffset;
						goto done;
					}
				}
			}

		done:
			;
		}

		m_asciiCharWidth[c] = widthMax + 2;
	}
}

void Font::_readUnicodeSizes(const std::string& filePath)
{
	memset(m_unicodeCharWidth, 0, sizeof(m_unicodeCharWidth));

	std::string fileData = AppPlatform::singleton()->readAssetFileStr(filePath, false);

	if (fileData.size() != NUM_GLYPHS)
		throw std::runtime_error("Bad glyph sizes file");

	for (int i = 0; i < NUM_GLYPHS; ++i)
	{
		// these widths are for font size 16
		// we render at font size 8
		// +1 is for better spacing between characters
		m_unicodeCharWidth[i] = static_cast<uint8_t>(fileData[i] / (COMMON_MAP_DIMENSION / RENDER_GLYPH_SIZE)) + 1;
	}
}

TextureData* Font::_getAsciiTextureData()
{
	return m_textures->getTextureData(m_asciiFileName, false);
}

TextureData* Font::_getUnicodeTextureData(int id)
{
	std::string fileName = "font/glyphs/glyph_" + Util::toString(id) + ".png";
	return m_textures->getTextureData(fileName, false);
}

TextureData* Font::_getTextureData(int id)
{
	// id == 0 uses the ascii/default texture map
	return id == 0 ? _getAsciiTextureData() : _getUnicodeTextureData(id);
}

float Font::_buildChar(int c, float x, float y)
{
	assert(c < NUM_GLYPHS);

	// ignore space characters (they are always empty so they don't need to be rendered)
	if (c == ' ')
		return static_cast<float>(SPACE_WIDTH);

	bool isAscii = _IsAsciiCharacter(c);
	float width = static_cast<float>(isAscii ? m_asciiCharWidth[c] : m_unicodeCharWidth[c]);

	int glyphMapId = _GetGlyphMapId(c);
	std::vector<GlyphQuad>& quads = m_glyphMapQuads[glyphMapId];
	quads.push_back(GlyphQuad(c, x, y, isAscii));
	m_usedGlyphMapQuads.insert(glyphMapId);

	return width;
}

Font::TextObject Font::_createTextObject(const std::string& str, const Color& color)
{
	TextObject textObject;

	const uint8_t* data = reinterpret_cast<const uint8_t*>(str.c_str());
	utf8proc_ssize_t len = str.size();

	float x = 0.0f;
	float y = 0.0f;

	utf8proc_ssize_t charLen;
	int c;
	while ((charLen = utf8proc_iterate(data, len, &c)) > 0) // NOTE: negative values are errors
	{
		data += charLen;
		len -= charLen;

		if (c >= NUM_GLYPHS)
			c = UNK_CHAR;

		if (c == COLOR_START_CHAR)
		{
			if (len > 0)
			{
				// TODO: implement formatting
				// for now, just ignore

				// format code should always be ascii
				data++;
				len--;
			}
		}
		else if (c == '\n')
		{
			x = 0.0f;
			y += RENDER_GLYPH_SIZE + NEW_LINE_SPACING;
		}
		else
		{
			x += _buildChar(c, x, y);
		}
	}

	// build meshes
	Tesselator& t = Tesselator::instance;

	for (std::set<int>::const_iterator it = m_usedGlyphMapQuads.begin(); it != m_usedGlyphMapQuads.end(); it++)
	{
		int id = *it;
		std::vector<GlyphQuad>& quads = m_glyphMapQuads[id];

		TextureData* textureData = _getTextureData(id);
		if (textureData) // there is a glyph map available for this
		{
			t.begin(quads.size() * 4);
			t.color(color);

			for (std::vector<GlyphQuad>::iterator it = quads.begin(); it != quads.end(); it++)
				(*it).append(t);

			mce::Mesh mesh = t.end();
			TextureData* textureData = _getTextureData(id);

			textObject.addPage(mesh, textureData);
		}

		// cleanup
		m_glyphMapQuads[id].clear();
	}

	// cleanup
	m_usedGlyphMapQuads.clear();

	return textObject;
}

void Font::drawCached(const std::string& str, int x, int y, const Color& color, bool isShadow)
{
	if (str.empty())
		return;

	const mce::MaterialPtr& material = m_materials.ui_text;

	if (isShadow)
		currentShaderDarkColor = Color(0.25f, 0.25f, 0.25f);
	else
		currentShaderDarkColor = Color::WHITE;

	Color finalColor = color;
	// For hex colors which don't specify an alpha
	if (finalColor.a == 0.0f)
		finalColor.a = 1.0f;

#ifndef FEATURE_GFX_SHADERS
	finalColor *= currentShaderDarkColor;
#endif

	MatrixStack::Ref mtx = MatrixStack::World.push();
	mtx->translate(Vec3(x, y, 0));

	if (m_cachingEnabled)
	{
		FontCacheKey key(str, finalColor);

		{
			TextObjectCacheMap::iterator it = m_textObjectCache.find(key);
			if (it != m_textObjectCache.end())
			{
				it->second.render(material);
				return;
			}
		}

		if (m_recentTextObjectCaches.size() > MAX_CACHE_SIZE)
		{
			const FontCacheKey& oldestKey = *m_recentTextObjectCaches.begin();
			m_textObjectCache.erase(oldestKey);
			m_recentTextObjectCaches.erase(m_recentTextObjectCaches.begin());
		}

		TextObject textObject = _createTextObject(str, finalColor);
		m_textObjectCache.insert(key, textObject);
		m_recentTextObjectCaches.push_back(key);
		textObject.render(material);
	}
	else
	{
		TextObject textObject = _createTextObject(str, finalColor);
		textObject.render(material);
	}
}

void Font::_buildCharSimple(uint8_t c, float x, float y)
{
	Tesselator& t = Tesselator::instance;

	float u = float((c % COMMON_MAP_DIMENSION) * ASCII_MAP_GLYPH_SIZE);
	float v = float((c / COMMON_MAP_DIMENSION) * ASCII_MAP_GLYPH_SIZE);
	
	constexpr float D = (1.0f / ASCII_MAP_PIXEL_DIMENSION);

#define CO (ASCII_MAP_GLYPH_SIZE - 0.01f)

	t.vertexUV(x,      y + CO, 0.0f,  u       * D, (v + CO) * D);
	t.vertexUV(x + CO, y + CO, 0.0f, (u + CO) * D, (v + CO) * D);
	t.vertexUV(x + CO, y,      0.0f, (u + CO) * D,  v       * D);
	t.vertexUV(x,      y,      0.0f,  u       * D,  v       * D);

#undef CO
}

void Font::drawSimple(const std::string& str, int x, int y, const Color& color, bool bShadow)
{
	if (str.empty())
		return;

	if (bShadow)
	{
		currentShaderDarkColor = Color(0.25f, 0.25f, 0.25f);
	}
	else
	{
		currentShaderDarkColor = Color::WHITE;
	}

	m_textures->loadAndBindTexture(m_asciiFileName);

	Color finalColor = color;
	// For hex colors which don't specify an alpha
	if (finalColor.a == 0.0f)
		finalColor.a = 1.0f;

#ifndef FEATURE_GFX_SHADERS
	finalColor *= currentShaderDarkColor;
#endif

	MatrixStack::Ref mtx = MatrixStack::World.push();
	mtx->translate(Vec3(x, y, 0));

	Tesselator& t = Tesselator::instance;
	t.begin(4 * str.size());

	t.color(finalColor);

	float cXPos = 0.0f, cYPos = 0.0f;

	for (size_t i = 0; i < str.size(); i++)
	{
		if (str[i] == '\n')
		{
			cYPos += RENDER_GLYPH_SIZE + NEW_LINE_SPACING;
			cXPos = 0;
			continue;
		}

		uint8_t x = uint8_t(str[i]);

		_buildCharSimple(x, cXPos, cYPos);

		cXPos += static_cast<float>(m_asciiCharWidth[x]);
	}

	t.draw(m_materials.ui_text);
}

void Font::draw(const std::string& str, int x, int y, const Color& color, bool bShadow)
{
	drawCached(str, x, y, color, bShadow);
}

void Font::draw(const std::string& str, int x, int y, const Color& color)
{
	draw(str, x, y, color, false);
}

void Font::drawShadow(const std::string& str, int x, int y, const Color& color)
{
	draw(str, x + 1, y + 1, color, true);
	draw(str, x, y, color, false);
}

void Font::drawScalable(const std::string& str, int x, int y, const Color& color, float scale, bool shadow)
{
	MatrixStack::Ref matrix = MatrixStack::World.push();
	matrix->translate(Vec3(x, y, 0));
	matrix->scale(scale);
	draw(str, 0, 0, color, shadow);
}

void Font::drawScalableShadow(const std::string& str, int x, int y, const Color& color, float scale)
{
	drawScalable(str, x + 1, y + 1, color, scale, true);
	drawScalable(str, x, y, color, scale);
}

void Font::drawString(const std::string& str, int x, int y, const Color& color, bool hasShadow, bool isConsole)
{
	if (hasShadow)
	{
		if (isConsole)
			drawScalableShadow(str, x, y, color);
		else
			drawShadow(str, x, y, color);
	}
	else
	{
		if (isConsole)
			drawScalable(str, x, y, color);
		else
			draw(str, x, y, color);
	}
}

void Font::drawOutlinedString(const std::string& str, int x, int y, const Color& color, const Color& outlineColor, float scale, int thickness)
{
	int translations[] = {0, thickness, -thickness};
	for (int xi = 0; xi < 3; ++xi)
	{
		int t = translations[xi];
		for (int yi = 0; yi < 3; ++yi)
		{
			int t1 = translations[yi];
			if (t != 0 || t1 != 0)
			{
				MatrixStack::Ref matrix = MatrixStack::World.push();
				matrix->translate(Vec3(t, t1, 0));
				drawScalable(str, x, y, outlineColor, scale, false);
			}
		}
	}

	drawScalable(str, x, y, color, scale, false);
}

void Font::drawWordWrap(const std::string& str, int x, int y, const Color& color, int width, int lineHeight, bool shadow, bool isConsole)
{
	drawWordWrap(split(str, width), x, y, color, lineHeight, shadow, isConsole);
}

void Font::drawWordWrap(const std::vector<std::string>& lines, int x, int y, const Color& color, int lineHeight, bool shadow, bool isConsole)
{
	for (std::vector<std::string>::const_iterator it = lines.begin(); it != lines.end(); ++it)
	{
		drawString(*it, x, y, color, shadow, isConsole);
		y += lineHeight;
	}
}

void Font::drawSimple(const std::string& str, int x, int y, const Color& color)
{
	drawSimple(str, x, y, color, false);
}

void Font::drawSimpleShadow(const std::string& str, int x, int y, const Color& color)
{
	drawSimple(str, x + 1, y + 1, color, true);
	drawSimple(str, x, y, color, false);
}

void Font::drawSimpleScalable(const std::string& str, int x, int y, const Color& color, float scale, bool shadow)
{
	MatrixStack::Ref matrix = MatrixStack::World.push();
	matrix->translate(Vec3(x, y, 0));
	matrix->scale(scale);
	drawSimple(str, 0, 0, color, shadow);
}

void Font::drawSimpleScalableShadow(const std::string& str, int x, int y, const Color& color, float scale)
{
	drawSimpleScalable(str, x + 1, y + 1, color, scale, true);
	drawSimpleScalable(str, x, y, color, scale);
}

void Font::onGraphicsReset()
{
	_init(m_options);
}

bool Font::containsUnicodeCharacters(const std::string& str)
{
	const uint8_t* data = reinterpret_cast<const uint8_t*>(str.c_str());
	utf8proc_ssize_t len = str.size();

	utf8proc_ssize_t charLen;
	int c;
	while ((charLen = utf8proc_iterate(data, len, &c)) > 0)
	{
		if (charLen != 1)
			return true;

		data += charLen;
		len -= charLen;
	}

	return false;
}

int Font::height(const std::string& str, int maxWidth)
{
	return split(str, maxWidth).size() * 8;
}

int Font::widthSimple(const std::string& str) const
{
	int maxLineWidth = 0, currentLineWidth = 0;

	for (int i = 0; i < int(str.size()); i++)
	{
		char chr = str[i];

		if (chr == COLOR_START_CHAR)
		{
			// skip the color code as well
			i++;
			continue;
		}
		if (chr == '\n')
		{
			if (maxLineWidth < currentLineWidth)
				maxLineWidth = currentLineWidth;
			currentLineWidth = 0;
		}

		currentLineWidth += m_asciiCharWidth[uint8_t(str[i])];
	}

	if (maxLineWidth < currentLineWidth)
		maxLineWidth = currentLineWidth;

	return maxLineWidth;
}

int Font::width(const std::string& str) const
{
	// TODO
	return widthSimple(str);
}

std::vector<std::string> Font::split(const std::string& text, int maxWidth)
{
	std::vector<std::string> lines;

	std::vector<std::string> paragraphs;
	size_t start = 0;
	size_t newlinePos = text.find('\n');
	while (newlinePos != std::string::npos)
	{
		paragraphs.push_back(text.substr(start, newlinePos - start));
		start = newlinePos + 1;
		newlinePos = text.find('\n', start);
	}
	paragraphs.push_back(text.substr(start));

	for (std::vector<std::string>::iterator it = paragraphs.begin(); it != paragraphs.end(); ++it)
	{
		std::string& paragraph = *it;

		if (paragraph.empty())
		{
			lines.push_back("");
			continue;
		}

		std::string currentLine;
		std::istringstream iss(paragraph);
		std::string word;

		while (iss >> word)
		{
			std::string testLine = currentLine.empty() ? word : currentLine + " " + word;

			if (width(testLine) <= maxWidth)
				currentLine = testLine;
			else
			{
				if (!currentLine.empty())
				{
					lines.push_back(currentLine);
					currentLine.clear();
				}

				while (!word.empty() && width(word) > maxWidth)
				{
					size_t breakPos = 0;
					for (size_t j = 1; j <= word.length(); ++j)
					{
						if (width(word.substr(0, j)) <= maxWidth)
							breakPos = j;
						else
							break;
					}

					if (breakPos == 0) breakPos = 1;

					std::string chunk = word.substr(0, breakPos);
					lines.push_back(chunk);
					word = word.substr(breakPos);
				}

				currentLine = word;
			}
		}

		if (!currentLine.empty())
			lines.push_back(currentLine);
	}

	while (!lines.empty() && lines.back().empty())
		lines.pop_back();

	if (lines.empty())
		lines.push_back("");

	return lines;
}

bool Font::_IsAsciiCharacter(int c)
{
	assert(c >= 0);
	return c < NUM_ASCII_CHARS;
}

int Font::_GetGlyphMapId(int c)
{
	assert(c < NUM_GLYPHS);
	return c / COMMON_MAP_TOTAL;
}
