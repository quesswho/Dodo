#pragma once

#include "Core/Graphics/FrameBuffer.h"

namespace Dodo {

	// Essentially a wrapper of framebuffer. Implementing point lights will give this class a distinction
	class ShadowMap {
	private:
		FrameBuffer* m_FrameBuffer;
	public:
		ShadowMap();
		~ShadowMap();

		void Bind() const;

		void BindTexture(uint index = 0) const;
	};
}