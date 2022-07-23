#pragma once

#include <algorithm>
#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>
#include <type_traits>
#include <vector>

#ifdef _linux
#include <sys/syscall.h>
#include <unistd.h>
using thread_id_t = pid_t;
#elif _APPLE_
#include <pthread.h>
using thread_id_t = unit64_t;
#else // mscv
#include <sstream>
    #if defined(_MSC_VER) // ssize_t type in windows
    #include <BaseTsd.h>
    typedef SSIZE_T ssize_t;
    #endif
using thread_id_t = unsigned int;
#endif 

/* Copy from limlog */
namespace test
{
	enum class log_level {
		trace = 0,
		debug,
		info,
		warn,
		error,
		fatal,
	};

	using output_func = ssize_t(*)(const char*, size_t);

	struct stdout_writer {
		static ssize_t write(const char* data, size_t n) {
			return fwrite(data, sizeof(char), n, stdout);
		}
	};

	class sync_logger {
	public:


	private:
		output_func output_;

	};

	template <typename logger> 
	class log 
	{
	public:
		log() : level_(log_level::info), output_(stdout_writer::write) {}
		log(const log&) = delete;
		log& operator=(const log&) = delete;

		void produce(const char* data, size_t n) { logger()->produce(data, n); }

		void flush(size_t n) { logger()->flush(n); }

		void set_level(log_level level) { level_ = level; }

		log_level getLogLevel() const { return level_; }

		void set_output(output_func func) { output_ = func; logger()->set_output(func); }

	private:
		logger* logger() {
		    /* 
			 * Explain the reason using 'static' reference to
			 * https://stackoverflow.com/questions/44124610/is-
			 * there-any-benefit-of-using-static-for-thread-lo
			 * cal-variable
			 */
			static thread_local logger* temp = nullptr;
			if (!temp) {
				std::lock_guard<std::mutex> lock(mtx_);
				temp = new logger;
				temp->set_output(output_);
				loggers_.push_back(temp);
			}
			return temp;
		}

	private:
		log_level   level_;
		output_func output_;
		std::mutex  mtx_;
		std::vector<std::shared_ptr<logger>> loggers_;
	};

	static log<sync_logger>* singleton() {
		static log<sync_logger> logger;
		return &logger;
	}
}
