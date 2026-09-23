#include "AuxTileItem.hpp"
#include "world/tile/Tile.hpp"

AuxTileItem::AuxTileItem(int id) : TileItem(id)
{
	m_maxDamage = 0;
	m_bStackedByData = true;
}

int AuxTileItem::getIcon(const ItemStack* itemStack, int layer) const
{
	int auxValue = itemStack ? itemStack->getAuxValue() : 0;
	return Tile::tiles[m_itemID]->getTexture(Facing::NORTH, auxValue);
}

TileData AuxTileItem::getLevelDataForAuxValue(int x) const
{
	return x;
}
