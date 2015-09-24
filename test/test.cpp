#include <iostream>

#include <video/window.h>

int main() {
	std::cout << "running\n";

	video::window w;

	std::string str;
	while (str != "q") {
		std::cin >> str;
		w.update();
	}
	
	return 0;
}