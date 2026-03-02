#pragma once

#include <Core/Common.h>

#include "Light.h"

#include <Core/ECS/World.h>

#include <Core/Graphics/Skybox.h>
#include <Core/Math/Camera/FreeCamera.h>

namespace Dodo {

    // Generic Scene class. The Engine uses Scene<World>, while the editor uses Scene<EditorWorld>
    template<typename WorldType = World>
    class GenericScene {
      public:
        std::string m_Name;

        // Contains all entities and their components.
        WorldType* m_World;

        Skybox* m_SkyBox;
        LightSystem m_LightSystem;

      public:
        GenericScene(Math::FreeCamera* camera, std::string name = "Unnamed");
        ~GenericScene();

        WorldType& GetWorld();

        inline void UpdateCamera(Math::FreeCamera* camera) { m_Camera = camera; }

      private:
        Math::FreeCamera* m_Camera;
    };

    using Scene = GenericScene<World>; // Engine scene
    // EditorScene will be defined in editor code as GenericScene<EditorWorld>
    
    // Template implementation
    template<typename WorldType>
    GenericScene<WorldType>::GenericScene(Math::FreeCamera* camera, std::string name)
        : m_Name(name), m_Camera(camera), m_SkyBox(nullptr),
          m_World(new WorldType()) 
    {}

    template<typename WorldType>
    GenericScene<WorldType>::~GenericScene() { 
        delete m_World; 
        delete m_SkyBox;
    }

    template<typename WorldType>
    WorldType& GenericScene<WorldType>::GetWorld() { 
        return *m_World; 
    }

} // namespace Dodo