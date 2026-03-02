#pragma once

#include <string>

struct NameComponent {
    std::string name;

    NameComponent() = default;
    NameComponent(const std::string& n) : name(n) {}
};