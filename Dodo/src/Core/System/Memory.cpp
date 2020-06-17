#include "Memory.h"

namespace Dodo {
	size_t MemoryMetrics::s_AllocatedBytes;
	size_t MemoryMetrics::s_CurrentBytes;
	size_t MemoryMetrics::s_DeallocatedBytes;

	void* MemoryAllocator::Alloc(size_t size)
	{
		MemoryMetrics::s_AllocatedBytes += size;
		MemoryMetrics::s_CurrentBytes += size;
		return malloc(size);
	}

	void MemoryAllocator::Dealloc(void* block, size_t size)
	{
		MemoryMetrics::s_DeallocatedBytes += size;
		MemoryMetrics::s_CurrentBytes -= size;
		free(block);
	}
}
void* operator new (size_t size)
{
	return Dodo::MemoryAllocator::Alloc(size);
}

void operator delete(void* block, size_t size)
{
	Dodo::MemoryAllocator::Dealloc(block, size);
}