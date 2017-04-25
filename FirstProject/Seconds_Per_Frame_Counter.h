#pragma once
/* Author: Ben Weatherall (a1617712)
 * Description: Class to report how long the last frame took to build. Use to optimise scene and engine
 * Based upon (Measure speed (http://www.opengl-tutorial.org/miscellaneous/an-fps-counter/))
*/
#include <cstdio>
#include <GLFW/glfw3.h>

class SPF_Counter {
public:
	SPF_Counter(bool report);
	void tick();
	double delta();
private:
	double currentTime;
	double lastTime;
	int nbFrames;
	bool report;
};
