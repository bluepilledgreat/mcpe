#pragma once

#include <map>

#include "Item.hpp"
#include "world/entity/EntityType.hpp"

class SpawnEggItem : public Item
{
public:
	struct Type
	{
		Type(EntityType::ID spawnedType, const Color& primaryColor, const Color& secondaryColor);

		EntityType::ID m_spawnedType;
		Color m_primaryColor;
		Color m_secondaryColor;
	};

public:
	SpawnEggItem(int itemID);

public:
	std::string getHovertextName(ItemStack& item) const override;
	Color getColor(const ItemStack* itemStack = nullptr, int layer = 0) const override;
	bool useOn(ItemStack& itemStack, Player& player, const TilePos& pos, Facing::Name face) const override;

private:
	static void _AddEgg(EntityType::ID spawnedType, const Color& primaryColor, const Color& secondaryColor);
	static bool _SpawnCreature(Level& level, EntityType::ID entityType, const Vec3& pos);

public:
	static void initTypes();

	static const Type* GetTypeByEntityTypeID(EntityType::ID id);
	static const std::map<EntityType::ID, Type>& GetTypes() { return eggTypes; }

private:
	static std::map<EntityType::ID, Type> eggTypes;
};
