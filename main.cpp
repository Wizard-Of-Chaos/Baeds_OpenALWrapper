#include <iostream>
#include "AudioDriver.h"
#include "dummyent.h"

int main()
{
	AudioDriver<dummyEntity> driver(dummyVel, dummyPos, dummyValid);
	std::cout << "Initialized driver. Playing impact noise.\n";
	driver.playMenuSound("impact_1.ogg");
	std::cout << "Test done.\n";
	std::string input;
	std::cin >> input;
}
