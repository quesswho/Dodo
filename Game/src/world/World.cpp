#include "World.h"

World::World()
	: m_WorldGen(0)
{
	m_Chunks.push_back(m_WorldGen->GenerateChunk(TVec2<int>(0, 0)));
	m_Chunks.push_back(m_WorldGen->GenerateChunk(TVec2<int>(-1, 0)));
	m_Chunks.push_back(m_WorldGen->GenerateChunk(TVec2<int>(0, -1)));
	m_Chunks.push_back(m_WorldGen->GenerateChunk(TVec2<int>(-1, -1)));
	m_Chunks.push_back(m_WorldGen->GenerateChunk(TVec2<int>(1, 0)));
	m_Chunks.push_back(m_WorldGen->GenerateChunk(TVec2<int>(0, 1)));
	m_Chunks.push_back(m_WorldGen->GenerateChunk(TVec2<int>(1, 1)));
	m_Chunks.push_back(m_WorldGen->GenerateChunk(TVec2<int>(1, -1)));
	m_Chunks.push_back(m_WorldGen->GenerateChunk(TVec2<int>(-1, 1)));
	m_Chunks.push_back(m_WorldGen->GenerateChunk(TVec2<int>(-1, -1)));
}