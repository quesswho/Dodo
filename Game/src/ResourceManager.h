#pragma once

#include <Dodo.h>

#include "world/Block.h"

class ResourceManager {
private:

public:
	ResourceManager();

	Ref<Dodo::IndexBuffer> m_FaceIBuffer;

	// Texture atlas
	Ref<Dodo::Material> m_TextureAtlas;

	uint m_SizeX;
	uint m_SizeY;

	FaceData GetFrontFace(BlockType type, BlockPos pos);
	FaceData GetBackFace(BlockType type, BlockPos pos);
	FaceData GetTopFace(BlockType type, BlockPos pos);
	FaceData GetBottomFace(BlockType type, BlockPos pos);
	FaceData GetLeftFace(BlockType type, BlockPos pos);
	FaceData GetRightFace(BlockType type, BlockPos pos);
};