#pragma once
#include "Core/Common.h"

namespace Dodo {

	enum class TextureFilter : uint
	{
		FILTER_MIN_MAG_LINEAR_MIP_NEAREST,
		FILTER_MIN_MAG_MIP_LINEAR,
		FILTER_MIN_NEAREST_MAG_LINEAR_MIP_NEAREST,
		FILTER_MIN_NEAREST_MAG_MIP_LINEAR,

		FILTER_MIN_MAG_MIP_NEAREST,
		FILTER_MIN_MAG_NEAREST_MIP_LINEAR,
		FILTER_MIN_LINEAR_MAG_MIP_NEAREST,
		FILTER_MIN_LINEAR_MAG_MIP_LINEAR
	};

	enum class TextureWrapMode
	{
		WRAP_REPEAT,
		WRAP_CLAMP_TO_BORDER,
		WRAP_CLAMP_TO_EDGE,
		WRAP_MIRRORED_REPEAT
	};

	struct TextureProperties {
		TextureProperties()
			: m_Filter(TextureFilter::FILTER_MIN_NEAREST_MAG_MIP_LINEAR), m_WrapU(TextureWrapMode::WRAP_REPEAT), m_WrapV(TextureWrapMode::WRAP_REPEAT)
		{}
		TextureProperties(TextureFilter filter, TextureWrapMode wrapU, TextureWrapMode wrapV)
			: m_Filter(filter), m_WrapU(wrapU), m_WrapV(wrapV)
		{}
		TextureFilter m_Filter;
		TextureWrapMode m_WrapU;
		TextureWrapMode m_WrapV;
	};
}