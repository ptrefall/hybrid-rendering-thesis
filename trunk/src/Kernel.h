#pragma once

#include <memory>
#include <string>

class Kernel;
typedef std::shared_ptr<Kernel> KernelPtr;

class Kernel
{
public:
	static KernelPtr getSingleton();
	~Kernel();

	void config(const std::string &resource_dir);
	void init(int argc, char** argv);

	void render();
	void reshape(int w, int h);

	void input(unsigned char key, int x, int y);
	void input(int key, int x, int y);

public:
	const std::string &getResourceDir() const { return resource_dir; }
	int getWidth() const { return width; }
	int getHeight() const { return height; }
	int getDepth() const { return depth; }
	int getRefreshRate() const { return refresh_rate; }
	int getFullscreen() const { return fullscreen; }
	int getGameMode() const { return game_mode; }
	std::string getGameModeString() const;
	int getOpenGLVersionMajor() const;
	int getOpenGLVersionMinor() const;
	std::string getOpenGLVersionString() const;

private:
	friend class std::_Ref_count_obj<Kernel>;
	static KernelPtr singleton;
	Kernel();

	std::string resource_dir;

	int width;
	int height;
	int depth;
	int refresh_rate;
	int fullscreen;
	int game_mode;
};
