#pragma once

#include <glm/glm.hpp>

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
		void bind(const glm::vec2 &data);
		void bind(const glm::vec3 &data);
		void bind(const glm::mat3 &data);
		void bind(const glm::mat4 &data);

		Uniform(const std::string &name);
		void bind(int data, unsigned int program);
		void bind(float data, unsigned int program);
		void bind(const glm::vec2 &data, unsigned int program);
		void bind(const glm::vec3 &data, unsigned int program);
		void bind(const glm::mat3 &data, unsigned int program);
		void bind(const glm::mat4 &data, unsigned int program);

	private:
		int location;
		unsigned int program;

		std::string name;
	};
}