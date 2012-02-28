#pragma once

#include "../GMEntity.h"
#include <Totem/Component.h>
#include <Totem/Property.h>

#include <memory>

namespace GM { namespace Render
{
	class DeferredRender;
	typedef std::shared_ptr<DeferredRender> DeferredRenderPtr;

	class DeferredRender : public Totem::Component<>
	{
	public:
		DeferredRender(GMEntityPtr owner, const std::string &name);
		static std::string getType() { return "DeferredRender"; }

		void render();

	private:
		GMEntityPtr owner;

		Totem::Property<unsigned int> screen_width;
		Totem::Property<unsigned int> screen_height;
	};
}}
