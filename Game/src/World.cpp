#include "World.h"

World::World()
	: m_WorldGen(0)
{
	m_Chunks.push_back(m_WorldGen->GenerateChunk(TVec2<int>(0.0f, 0.0f)));
	m_Chunks.push_back(m_WorldGen->GenerateChunk(TVec2<int>(-1.0f, 0.0f)));
	m_Chunks.push_back(m_WorldGen->GenerateChunk(TVec2<int>(0.0f, -1.0f)));
	m_Chunks.push_back(m_WorldGen->GenerateChunk(TVec2<int>(-1.0f, -1.0f)));
	//m_Chunks.push_back(m_WorldGen->GenerateChunk(TVec2<int>(-2.0f, -1.0f)));
	//m_Chunks.push_back(m_WorldGen->GenerateChunk(TVec2<int>(-1.0f, -2.0f)));
}