/********************************************************************
	Minecraft: Pocket Edition - Decompilation Project
	Copyright (C) 2023 iProgramInCpp
	
	The following code is licensed under the BSD 1 clause license.
	SPDX-License-Identifier: BSD-1-Clause
 ********************************************************************/

#include "GameMods.hpp"
#ifdef ENH_ALLOW_SAND_GRAVITY
#include "FallingTileRenderer.hpp"
#include "client/renderer/entity/EntityRenderDispatcher.hpp"
#include "client/renderer/renderer/RenderMaterialGroup.hpp"
#include "renderer/MatrixStack.hpp"
#include "world/entity/FallingTile.hpp"

FallingTileRenderer::Materials::Materials()
{
	MATERIAL_PTR(switchable, heavy_tile);
}

FallingTileRenderer::FallingTileRenderer()
{
	m_shadowRadius = 0.5f;
}

void FallingTileRenderer::render(const Entity& entity, const Vec3& pos, float rot, float a)
{
	const FallingTile& tile = (const FallingTile&)entity;
	
	MatrixStack::Ref matrix = MatrixStack::World.push();
	matrix->translate(pos);

	bindTexture(C_TERRAIN_NAME);

	m_pDispatcher->m_tileRenderer->renderTile(FullTile(tile.getTile(), 0), m_heavyMaterials.heavy_tile);
}

#endif
