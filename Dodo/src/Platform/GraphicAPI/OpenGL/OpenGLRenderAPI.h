#pragma once

#include "Core/Application/Window.h"

namespace Dodo {

	namespace Platform {

		class OpenGLRenderAPI {
		public:
			OpenGLRenderAPI();
			~OpenGLRenderAPI();
			int Init(const WindowProperties& winprop);

			void Begin() const;
			void End() const {}

			void ClearColor(float r, float g, float b) const;
			void Viewport(uint width, uint height) const;
			const char* GetAPIName() const;

			std::string m_GPUInfo;
			int m_VramKbs;
		private:
			bool WGLExtensionCheck(const char* ext) const;
		};
	}
}