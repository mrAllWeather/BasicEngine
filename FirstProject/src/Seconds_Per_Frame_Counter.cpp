#include "../include/Seconds_Per_Frame_Counter.h"

SPF_Counter::SPF_Counter(bool p_report)
{
	this->lastTime = glfwGetTime();
	this->nbFrames = 0;
	this->report = p_report;
}

void SPF_Counter::tick()
{
	// Measure speed (http://www.opengl-tutorial.org/miscellaneous/an-fps-counter/)
	nbFrames++;
	if (delta() >= 1.0) { // If last prinf() was more than 1 sec ago printf and reset timer
		if(report) printf("%f ms/frame\n", 1000.0 / double(nbFrames));
		nbFrames = 0;
		lastTime += 1.0;
	}
}

double SPF_Counter::delta()
{
	currentTime = glfwGetTime();
	return currentTime - lastTime;
}