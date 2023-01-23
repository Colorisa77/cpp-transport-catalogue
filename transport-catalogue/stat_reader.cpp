#include <iostream>

#include "stat_reader.h"


namespace stat_reader {
	void StatReader::DoShowRequest() {
		if (requests_.empty()) {
			return;
		}
		for (std::string task : requests_) {
			std::cout << task << std::endl;
		}
	}
}