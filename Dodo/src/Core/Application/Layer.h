#pragma once

namespace Dodo {

	class Layer
	{
	public:
		Layer() {}
		virtual ~Layer() {}

		virtual void Update(float elapsed) = 0;
		virtual void Render() = 0;
	};

}