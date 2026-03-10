#pragma once
#include "ShaderSource.h"

namespace Dodo {
    struct ShaderParser {

        static ShaderSource Parse(std::string source)
        {
            ShaderStage stage = ShaderStage::Unknown;

            std::string stringFragmentSource = "";
            std::string stringVertexSource = "";

            ShaderSource shaderSource;
            std::istringstream text(source);
            std::string line;
            while (std::getline(text, line)) {
                // Trim beginning of line
                const auto strBegin = line.find_first_not_of(" \t");
                if (strBegin != std::string::npos) line = line.substr(strBegin);

                // Check for stage
                if (line == "#shader fragment") {
                    stage = ShaderStage::Fragment;
                    continue;
                }

                if (line == "#shader vertex") {
                    stage = ShaderStage::Vertex;
                    continue;
                }

                switch (stage) {
                case ShaderStage::Fragment:
                    stringFragmentSource.append(line).append("\n");
                    break;
                case ShaderStage::Vertex:
                    stringVertexSource.append(line).append("\n");
                    break;
                }
            }

            if (stringFragmentSource == "" || stringVertexSource == "") {
                DD_ERR("Source needs to be in a specific format! Add \"#shader fragment\" or \"#shader vertex\" "
                       "as the first line of the different shaders to differentiate between between the differnt "
                       "shader types!");
                return shaderSource;
            }

            shaderSource.stages.push_back({ShaderStage::Vertex, stringVertexSource});
            shaderSource.stages.push_back({ShaderStage::Fragment, stringFragmentSource});
            return shaderSource;
        }
    };
} // namespace Dodo