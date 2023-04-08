#pragma once
#include <Dodo.h>

#include "Chunk.h"

using namespace Dodo::Math;
class WorldGeneration {

private:
	uint m_Seed;
public:

	WorldGeneration(uint seed)
		: m_Seed(seed)
	{}

	Ref<Chunk> GenerateChunk(TVec2<int> chunpos);
};