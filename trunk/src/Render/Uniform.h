#pragma once

#include <Eigen\Eigen>

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
		void bind(const Eigen::Vector2f &data);
		void bind(const Eigen::Vector3f &data);
		void bind(const Eigen::Matrix3f &data);
		void bind(const Eigen::Matrix4f &data);

		Uniform(const std::string &name);
		void bind(int data, unsigned int program);
		void bind(float data, unsigned int program);
		void bind(const Eigen::Vector2f &data, unsigned int program);
		void bind(const Eigen::Vector3f &data, unsigned int program);
		void bind(const Eigen::Matrix3f &data, unsigned int program);
		void bind(const Eigen::Matrix4f &data, unsigned int program);

	private:
		int location;
		unsigned int program;

		std::string name;
	};
}