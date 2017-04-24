#pragma once
/* Based upon (Measure speed (http://www.opengl-tutorial.org/miscellaneous/an-fps-counter/))
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
