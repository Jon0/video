#include "pipe.h"

namespace os {

std::string exec(const std::string &cmd) {
	std::string result = "";
	try {
		// TODO use file descriptors
		FILE* pipe = popen(cmd.c_str(), "r");
		if (!pipe) return "ERROR";

		// copy output to a std::string
		char buffer[256];
	

		while(!feof(pipe)) {
			if(fgets(buffer, 256, pipe) != NULL)
				result += buffer;
		}

		pclose(pipe);
	}
	catch (...) {
		return "ERROR";
	}
	return result;
}

}
