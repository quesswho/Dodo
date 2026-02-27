#pragma once

#include <cstddef>
#include <string>

namespace Dodo {

class IDataFile {
public:
    virtual ~IDataFile() = default;

    virtual void BeginRead(const std::string& path) = 0;
    virtual void EndRead() = 0;
    virtual void BeginWrite() = 0;
    virtual void EndWrite(const std::string& path) = 0;

    virtual std::size_t GetCurrentOffset() const = 0;
};

}
