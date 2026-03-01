#include "Memory.h"
#include "pch.h"

#include <sstream>

namespace Dodo {
    size_t MemoryMetrics::s_AllocatedBytes;
    size_t MemoryMetrics::s_CurrentBytes;
    size_t MemoryMetrics::s_DeallocatedBytes;

    void *MemoryAllocator::Alloc(size_t size)
    {
        MemoryMetrics::s_AllocatedBytes += size;
        MemoryMetrics::s_CurrentBytes += size;
        return malloc(size);
    }

    void MemoryAllocator::Dealloc(void *block, size_t size)
    {
        MemoryMetrics::s_DeallocatedBytes += size;
        MemoryMetrics::s_CurrentBytes -= size;
        free(block);
    }

    const float StringUtils::p = 1.0f / T;
    const float StringUtils::n = 1.0f / G;
    const float StringUtils::u = 1.0f / M;
    const float StringUtils::m = 1.0f / K;
    const float StringUtils::One = 1.0f;
    const float StringUtils::K = 1024.0f;
    const float StringUtils::M = 1024.0f * 1024.0f;
    const float StringUtils::G = 1024.0f * 1024.0f * 1024.0f;
    const float StringUtils::T = 1024.0f * 1024.0f * 1024.0f * 1024.0f;

    std::string StringUtils::Byte(size_t size)
    {
        if (size >= T) return precision_to_string((round(size / T * 100.0) / 100)).append(" TB");
        if (size >= G) return precision_to_string((round(size / G * 100.0) / 100)).append(" GB");
        if (size >= M) return precision_to_string((round(size / M * 100.0) / 100)).append(" MB");
        if (size >= K) return precision_to_string((round(size / K * 100.0) / 100)).append(" KB");
        return precision_to_string(size).append(" B");
    }

    std::string StringUtils::KiloByte(size_t size)
    {
        if (size >= G) return precision_to_string((round(size / G * 100.0) / 100)).append(" TB");
        if (size >= M) return precision_to_string((round(size / M * 100.0) / 100)).append(" GB");
        if (size >= K) return precision_to_string((round(size / K * 100.0) / 100)).append(" MB");
        if (size >= One) return precision_to_string((round(size * 100.0) / 100)).append(" KB");
        return std::string("0 B");
    }

    std::string StringUtils::KiloByte(double size)
    {
        if (size >= G) return precision_to_string((round(size / G * 100.0) / 100)).append(" TB");
        if (size >= M) return precision_to_string((round(size / M * 100.0) / 100)).append(" GB");
        if (size >= K) return precision_to_string((round(size / K * 100.0) / 100)).append(" MB");
        if (size >= One) return precision_to_string((round(size * 100.0) / 100)).append(" KB");
        if (size >= m) return precision_to_string((round(size * K * 100.0) / 100)).append(" B");
        return std::string("0 B");
    }

    std::string StringUtils::MegaByte(size_t size)
    {
        if (size >= M) return precision_to_string((round(size / M * 100.0) / 100)).append(" TB");
        if (size >= K) return precision_to_string((round(size / K * 100.0) / 100)).append(" GB");
        if (size >= One) return precision_to_string((round(size * 100.0) / 100)).append(" MB");
        return std::string("0 B");
    }

    std::string StringUtils::MegaByte(double size)
    {
        if (size >= M) return precision_to_string((round(size / M * 100.0) / 100)).append(" TB");
        if (size >= K) return precision_to_string((round(size / K * 100.0) / 100)).append(" GB");
        if (size >= One) return precision_to_string((round(size * 100.0) / 100)).append(" MB");
        if (size >= m) return precision_to_string((round(size * K * 100.0) / 100)).append(" KB");
        if (size >= u) return precision_to_string((round(size * M * 100.0) / 100)).append(" B");
        return std::string("0 B");
    }

    std::string StringUtils::GigaByte(size_t size)
    {
        if (size >= K) return precision_to_string((round(size / K * 100.0) / 100)).append(" TB");
        if (size >= One) return precision_to_string((round(size * 100.0) / 100)).append(" GB");
        return std::string("0 B");
    }

    std::string StringUtils::GigaByte(double size)
    {
        if (size >= K) return precision_to_string((round(size / K * 100.0) / 100)).append(" TB");
        if (size >= One) return precision_to_string((round(size * 100.0) / 100)).append(" GB");
        if (size >= m) return precision_to_string((round(size * K * 100.0) / 100)).append(" MB");
        if (size >= u) return precision_to_string((round(size * M * 100.0) / 100)).append(" KB");
        if (size >= n) return precision_to_string((round(size * G * 100.0) / 100)).append(" B");
        return std::string("0 B");
    }

    std::string StringUtils::TeraByte(size_t size)
    {
        if (size >= One) return precision_to_string((round(size * 100.0) / 100)).append(" TB");
        return std::string("0 B");
    }

    std::string StringUtils::TeraByte(double size)
    {
        if (size >= One) return precision_to_string((round(size / One * 100.0) / 100)).append(" TB");
        if (size >= m) return precision_to_string((round(size / K * 100.0) / 100)).append(" GB");
        if (size >= u) return precision_to_string((round(size * M * 100.0) / 100)).append(" MB");
        if (size >= n) return precision_to_string((round(size * G * 100.0) / 100)).append(" KB");
        if (size >= p) return precision_to_string((round(size * T * 100.0) / 100)).append(" B");
        return std::string("0 B");
    }

    template <typename Ty> std::string StringUtils::precision_to_string(const Ty val, const int n)
    {
        std::ostringstream result;
        result.precision(n);
        result << std::fixed << val;
        return result.str();
    }
} // namespace Dodo
void *operator new(size_t size)
{
    return Dodo::MemoryAllocator::Alloc(size);
}

void operator delete(void *block, size_t size)
{
    Dodo::MemoryAllocator::Dealloc(block, size);
}
