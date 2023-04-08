#pragma once

#include "Block.h"
#include <memory>
#include <array>

class World {
private:
	Dodo::Math::FreeCamera* m_Camera;
public:
	World(Dodo::Math::FreeCamera* camera);
	std::array<std::shared_ptr<Block>, 1024> m_Blocks;

	void Draw();
};