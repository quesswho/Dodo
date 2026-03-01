#pragma once

#include <string>

namespace Dodo {

    class MemoryMetrics {
      public:
        static size_t s_AllocatedBytes;
        static size_t s_DeallocatedBytes;
        static size_t s_CurrentBytes;
    };

    class MemoryAllocator {
      public:
        static inline void *Alloc(size_t size);
        static inline void Dealloc(void *block, size_t size);
    };

    class StringUtils {
      private:
        static const float p;
        static const float n;
        static const float u;
        static const float m;
        static const float One;
        static const float K;
        static const float M;
        static const float G;
        static const float T;

      public:
        static std::string Byte(size_t size);

        static std::string KiloByte(size_t size);
        static std::string KiloByte(double size);

        static std::string MegaByte(size_t size);
        static std::string MegaByte(double size);

        static std::string GigaByte(size_t size);
        static std::string GigaByte(double size);

        static std::string TeraByte(size_t size);
        static std::string TeraByte(double size);

        template <typename Ty> static std::string precision_to_string(const Ty val, const int n = 2);
    };
} // namespace Dodo
#if !defined(DODO_DISABLE_MEM_DEBUG)

#pragma push_macro("new")
#undef new

void *operator new(size_t size);

#define ddnew new
#pragma pop_macro("new")

#pragma push_macro("delete")
void operator delete(void *block, size_t size);
#define dddelete delete
#pragma pop_macro("delete")
#else
#define dddelete delete
#define ddnew    new
#endif