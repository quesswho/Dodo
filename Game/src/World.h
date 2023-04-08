#pragma once

#include "Chunk.h"
#include "WorldGeneration.h"
#include <memory>
#include <array>

class World {
private:
	Ref<WorldGeneration> m_WorldGen;
public:
	World();
	std::vector<Ref<Chunk>> m_Chunks;
};