#include <GM/Render/GBuffer.h>

using namespace GM;
using namespace Render;

GBuffer::GBuffer(GMEntityPtr owner, const std::string &name) 
: Totem::Component<>(getType(), name), owner(owner)
{
	screen_width = owner->add<unsigned int>("ScreenWidth", 800);
	screen_height = owner->add<unsigned int>("ScreenHeight", 600);
}

void GBuffer::begin()
{
}

void GBuffer::end()
{
}
