/********************************************************************
	Minecraft: Pocket Edition - Decompilation Project
	Copyright (C) 2023 iProgramInCpp
	
	The following code is licensed under the BSD 1 clause license.
	SPDX-License-Identifier: BSD-1-Clause
 ********************************************************************/

#pragma once

#include "Textures.hpp"
#include "client/options/Options.hpp"
#include "renderer/MaterialPtr.hpp"
#include "client/renderer/renderer/Tesselator.hpp"
#include "client/renderer/texture/TextureData.hpp"
#include "common/utility/HashMap.hpp"
#include <vector>
#include <set>

struct FontCacheKey
{
public:
	std::string str;
	Color color;

public:
	FontCacheKey()
	{
	}

	FontCacheKey(const std::string& str, const Color& color)
		: str(str)
		, color(color)
	{
	}

	bool operator==(const FontCacheKey& other) const
	{
		return str == other.str && color == other.color;
	}

	bool operator!=(const FontCacheKey& other) const
	{
		return str != other.str || color != other.color;
	}
};

template<>
struct HashFunction<FontCacheKey>
{
	size_t operator()(const FontCacheKey& key) const;
};

class Font
{
public:
	static constexpr int NUM_ASCII_CHARS = 256; // Whole ASCII set
	static constexpr int NUM_GLYPHS = 0x1FFFF + 1; // Plane 0 to 1

	static constexpr float RENDER_GLYPH_SIZE = 8.0f;

	// COMMON //
	static constexpr int COMMON_MAP_DIMENSION = 16; // number of glyphs on one row/column
	static constexpr int COMMON_MAP_TOTAL = COMMON_MAP_DIMENSION * COMMON_MAP_DIMENSION; // total number of glyphs in one map

	// ASCII MAP //
	static constexpr int ASCII_MAP_GLYPH_SIZE = 8; // size of each glyph on the map in pixels
	static constexpr int ASCII_MAP_PIXEL_DIMENSION = COMMON_MAP_DIMENSION * ASCII_MAP_GLYPH_SIZE; // size of one row/column in pixels

	// UNICODE MAPS //
	static constexpr int UNICODE_MAP_GLYPH_SIZE = 16; // size of each glyph on the map in pixels
	static constexpr int UNICODE_MAP_PIXEL_DIMENSION = COMMON_MAP_DIMENSION * UNICODE_MAP_GLYPH_SIZE; // size of one row/column in pixels

private:
	class Materials
	{
	public:
		mce::MaterialPtr ui_text;

		Materials();
	};

	class GlyphQuad
	{
	public:
		int c;
		float x;
		float y;
		bool isAscii;

	public:
		GlyphQuad(int c, float x, float y, bool isAscii);

	public:
		void append(Tesselator& t);
	};

	class TextObject
	{
	private:
		class Page
		{
		public:
			mce::Mesh mesh;
			TextureData* textureData;

		public:
			Page(mce::Mesh& mesh, TextureData* textureData);
		};

	private:
		std::vector<Page> pages;

	public:
		TextObject();
		~TextObject();

	public:
		void addPage(mce::Mesh& mesh, TextureData* textureData);
		void render(const mce::MaterialPtr& material);
	};

private:
	typedef HashMap<FontCacheKey, TextObject> TextObjectCacheMap;

private:
	void _init(Options* pOpts);
	void _computeAsciiSizes();
	void _readUnicodeSizes(const std::string& filePath);

	TextureData* _getAsciiTextureData();
	TextureData* _getUnicodeTextureData(int id);
	TextureData* _getTextureData(int id);

	float _buildChar(int c, float x, float y);
	TextObject _createTextObject(const std::string& str, const Color& color);

	void _buildCharSimple(uint8_t c, float x, float y);

public:
	Font(Options* pOpts, const std::string& fileName, Textures* pTexs);

	void drawCached(const std::string&, int x, int y, const Color& color, bool isShadow);
	void drawSimple(const std::string&, int x, int y, const Color& color, bool bShadow);

	void draw(const std::string&, int x, int y, const Color& color);
	void draw(const std::string&, int x, int y, const Color& color, bool bShadow);
	void drawShadow(const std::string&, int x, int y, const Color& color);
	void drawScalable(const std::string&, int x, int y, const Color& color, float scale = 2.0f, bool shadow = false);
	void drawScalableShadow(const std::string&, int x, int y, const Color& color, float scale = 2.0f);
	void drawString(const std::string&, int x, int y, const Color& color, bool hasShadow, bool isConsole = false);
	void drawOutlinedString(const std::string&, int x, int y, const Color& color, const Color& outlineColor, float scale = 4.0f, int thickness = 2);
	void drawWordWrap(const std::string&, int x, int y, const Color& color, int width, int lineHeight = 8, bool shadow = false, bool isConsole = false);
	void drawWordWrap(const std::vector<std::string>&, int x, int y, const Color& color, int lineHeight = 8, bool shadow = false, bool isConsole = false);

	void drawSimple(const std::string&, int x, int y, const Color& color);
	void drawSimpleShadow(const std::string&, int x, int y, const Color& color);
	void drawSimpleScalable(const std::string&, int x, int y, const Color& color, float scale = 2.0f, bool shadow = false);
	void drawSimpleScalableShadow(const std::string&, int x, int y, const Color& color, float scale = 2.0f);

	bool getCachingEnabled() const
	{
		return m_cachingEnabled;
	}
	void setCachingEnabled(bool enabled)
	{
		m_cachingEnabled = enabled;
	}

	void onGraphicsReset();

	bool containsUnicodeCharacters(const std::string& str);

	int width(const std::string& str) const;
	int widthSimple(const std::string& str) const;
	std::vector<std::string> split(const std::string& str, int width);
	int height(const std::string& str, int maxWidth);

private:
	static bool _IsAsciiCharacter(int c);
	static int _GetGlyphMapId(int c);

private:
	uint8_t m_asciiCharWidth[NUM_ASCII_CHARS];
	uint8_t m_unicodeCharWidth[NUM_GLYPHS];
	std::vector<GlyphQuad> m_glyphMapQuads[NUM_GLYPHS / COMMON_MAP_TOTAL];
	std::set<int> m_usedGlyphMapQuads;

	TextObjectCacheMap m_textObjectCache;
	std::vector<FontCacheKey> m_recentTextObjectCaches; // TODO: circular buffer
	bool m_cachingEnabled;

	std::string m_asciiFileName;
	Options* m_options;
	Textures* m_textures;
	Materials m_materials;
};

