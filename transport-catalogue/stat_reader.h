#pragma once
#include <string>
#include <deque>

namespace stat_reader {
	class StatReader {
	public:
		StatReader(std::deque<std::string> task_show)
			: requests_(std::move(task_show)) {
		}

		void DoShowRequest();

	private:
		std::deque<std::string> requests_;
	};
}