#pragma once

#include "../GMEntity.h"
#include <Totem/Component.h>
#include <Totem/Property.h>

#include <memory>

namespace GM { namespace Render
{
	class GBuffer;
	typedef std::shared_ptr<GBuffer> GBufferPtr;

	class GBuffer : public Totem::Component<>
	{
	public:
		GBuffer(GMEntityPtr owner, const std::string &name);
		static std::string getType() { return "GBuffer"; }

		void begin();
		void end();

	private:
		GMEntityPtr owner;

		Totem::Property<unsigned int> screen_width;
		Totem::Property<unsigned int> screen_height;
	};
}}