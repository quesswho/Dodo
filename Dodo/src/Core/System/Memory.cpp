#include "Memory.h"

#include <cmath>
#include <sstream>

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

    const float MemoryFormatter::p = 1.0f / T;
    const float MemoryFormatter::n = 1.0f / G;
    const float MemoryFormatter::u = 1.0f / M;
    const float MemoryFormatter::m = 1.0f / K;
    const float MemoryFormatter::One = 1.0f;
    const float MemoryFormatter::K = 1024.0f;
    const float MemoryFormatter::M = 1024.0f * 1024.0f;
    const float MemoryFormatter::G = 1024.0f * 1024.0f * 1024.0f;
    const float MemoryFormatter::T = 1024.0f * 1024.0f * 1024.0f * 1024.0f;

    std::string MemoryFormatter::Byte(size_t size)
    {
        if (size >= T) return precision_to_string((std::round(size / T * 100.0) / 100)).append(" TB");
        if (size >= G) return precision_to_string((std::round(size / G * 100.0) / 100)).append(" GB");
        if (size >= M) return precision_to_string((std::round(size / M * 100.0) / 100)).append(" MB");
        if (size >= K) return precision_to_string((std::round(size / K * 100.0) / 100)).append(" KB");
        return precision_to_string(size).append(" B");
    }

    std::string MemoryFormatter::KiloByte(size_t size)
    {
        if (size >= G) return precision_to_string((std::round(size / G * 100.0) / 100)).append(" TB");
        if (size >= M) return precision_to_string((std::round(size / M * 100.0) / 100)).append(" GB");
        if (size >= K) return precision_to_string((std::round(size / K * 100.0) / 100)).append(" MB");
        if (size >= One) return precision_to_string((std::round(size * 100.0) / 100)).append(" KB");
        return std::string("0 B");
    }

    std::string MemoryFormatter::KiloByte(double size)
    {
        if (size >= G) return precision_to_string((std::round(size / G * 100.0) / 100)).append(" TB");
        if (size >= M) return precision_to_string((std::round(size / M * 100.0) / 100)).append(" GB");
        if (size >= K) return precision_to_string((std::round(size / K * 100.0) / 100)).append(" MB");
        if (size >= One) return precision_to_string((std::round(size * 100.0) / 100)).append(" KB");
        if (size >= m) return precision_to_string((std::round(size * K * 100.0) / 100)).append(" B");
        return std::string("0 B");
    }

    std::string MemoryFormatter::MegaByte(size_t size)
    {
        if (size >= M) return precision_to_string((std::round(size / M * 100.0) / 100)).append(" TB");
        if (size >= K) return precision_to_string((std::round(size / K * 100.0) / 100)).append(" GB");
        if (size >= One) return precision_to_string((std::round(size * 100.0) / 100)).append(" MB");
        return std::string("0 B");
    }

    std::string MemoryFormatter::MegaByte(double size)
    {
        if (size >= M) return precision_to_string((std::round(size / M * 100.0) / 100)).append(" TB");
        if (size >= K) return precision_to_string((std::round(size / K * 100.0) / 100)).append(" GB");
        if (size >= One) return precision_to_string((std::round(size * 100.0) / 100)).append(" MB");
        if (size >= m) return precision_to_string((std::round(size * K * 100.0) / 100)).append(" KB");
        if (size >= u) return precision_to_string((std::round(size * M * 100.0) / 100)).append(" B");
        return std::string("0 B");
    }

    std::string MemoryFormatter::GigaByte(size_t size)
    {
        if (size >= K) return precision_to_string((std::round(size / K * 100.0) / 100)).append(" TB");
        if (size >= One) return precision_to_string((std::round(size * 100.0) / 100)).append(" GB");
        return std::string("0 B");
    }

    std::string MemoryFormatter::GigaByte(double size)
    {
        if (size >= K) return precision_to_string((std::round(size / K * 100.0) / 100)).append(" TB");
        if (size >= One) return precision_to_string((std::round(size * 100.0) / 100)).append(" GB");
        if (size >= m) return precision_to_string((std::round(size * K * 100.0) / 100)).append(" MB");
        if (size >= u) return precision_to_string((std::round(size * M * 100.0) / 100)).append(" KB");
        if (size >= n) return precision_to_string((std::round(size * G * 100.0) / 100)).append(" B");
        return std::string("0 B");
    }

    std::string MemoryFormatter::TeraByte(size_t size)
    {
        if (size >= One) return precision_to_string((std::round(size * 100.0) / 100)).append(" TB");
        return std::string("0 B");
    }

    std::string MemoryFormatter::TeraByte(double size)
    {
        if (size >= One) return precision_to_string((std::round(size / One * 100.0) / 100)).append(" TB");
        if (size >= m) return precision_to_string((std::round(size / K * 100.0) / 100)).append(" GB");
        if (size >= u) return precision_to_string((std::round(size * M * 100.0) / 100)).append(" MB");
        if (size >= n) return precision_to_string((std::round(size * G * 100.0) / 100)).append(" KB");
        if (size >= p) return precision_to_string((std::round(size * T * 100.0) / 100)).append(" B");
        return std::string("0 B");
    }

    template <typename Ty>
    std::string MemoryFormatter::precision_to_string(const Ty val, const int n)
    {
        std::ostringstream result;
        result.precision(n);
        result << std::fixed << val;
        return result.str();
    }
} // namespace Dodo
void* operator new(size_t size)
{
    return Dodo::MemoryAllocator::Alloc(size);
}

void operator delete(void* block, size_t size) noexcept
{
    Dodo::MemoryAllocator::Dealloc(block, size);
}
