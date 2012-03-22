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

		Uniform(const std::string &name);
		void bind(int data, unsigned int program);
		void bind(float data, unsigned int program);

	private:
		int location;
		unsigned int program;

		std::string name;
	};
}