#pragma once

#include "TileItem.hpp"

class AuxTileItem : public TileItem
{
public:
	AuxTileItem(int id);

public:
	int getIcon(const ItemStack* itemStack = nullptr, int layer = 0) const override;
	TileData getLevelDataForAuxValue(int x) const override;
};
