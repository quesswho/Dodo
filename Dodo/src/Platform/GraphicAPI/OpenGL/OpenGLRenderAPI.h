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
			void DrawIndices(uint count) const;

			void DepthTest(bool depthtest) const;
			void Blending(bool blending) const;
			void StencilTest(bool stenciltest) const;

			const char* GetAPIName() const;
			int CurrentVRamUsage() const;

			std::string m_GPUInfo;
			int m_VramKbs;
		private:
		};
	}
}