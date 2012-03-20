#pragma once

#include <memory>
#include <string>

namespace Render
{
	class Uniform;
	typedef std::shared_ptr<Uniform> UniformPtr;

	class Uniform
	{
	public:
		Uniform(unsigned int program, const std::string &name);

		void bind(int data);
		void bind(float data);

	private:
		int location;
		unsigned int program;
	};
}