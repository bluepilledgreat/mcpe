#pragma once

#include "TileItem.hpp"

class ClothItem : public TileItem
{
public:
	ClothItem(int id);

public:
	std::string getDescriptionId(ItemStack& item) const override;
	int getIcon(const ItemStack* itemStack = nullptr, int layer = 0) const override;
	TileData getLevelDataForAuxValue(int x) const override;
};
