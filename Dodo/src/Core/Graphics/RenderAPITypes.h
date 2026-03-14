namespace Dodo {
    enum class DepthComparisonMethod : uint {
        NEVER,
        LESS,
        EQUAL,
        LESS_EQUAL,
        GREATER,
        NOT_EQUAL,
        GREATER_EQUAL,
        ALWAYS,
        DEFAULT = LESS
    };
} // namespace Dodo