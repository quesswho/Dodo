#pragma once

namespace Dodo {


	class MemoryMetrics {
	public:
		static size_t s_AllocatedBytes;
		static size_t s_DeallocatedBytes;
		static size_t s_CurrentBytes;
	};


	class MemoryAllocator
	{
	public:
		static inline void* Alloc(size_t size);
		static inline void Dealloc(void* block, size_t size);
	};
}
#if !defined(DODO_DISABLE_MEM_DEBUG)

#pragma push_macro("new")
#undef new

void* operator new (size_t size);

#define ddnew new
#pragma pop_macro("new")

#pragma push_macro("delete")
void operator delete(void* block, size_t size);
#define dddelete delete
#pragma pop_macro("delete")
#else
#define dddelete delete
#define ddnew new
#endif