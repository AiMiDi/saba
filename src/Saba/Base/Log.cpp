//
// Copyright(c) 2016-2017 benikabocha.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#include "Log.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#if _WIN32

#include <Windows.h>

#endif // _WIN32

namespace saba
{
	DefaultSink::DefaultSink()
	{
		m_defaultLogger = spdlog::stdout_color_mt("default");
#if _WIN32
		SetConsoleOutputCP(CP_UTF8);
#endif // _WIN32
	}

	void DefaultSink::log(const spdlog::details::log_msg & msg)
	{
		m_defaultLogger->log(msg.level, msg.payload);
	}

	void DefaultSink::flush()
	{
		m_defaultLogger->flush();
	}

	void DefaultSink::set_pattern(const std::string& pattern)
	{
		m_defaultLogger->set_pattern(pattern);
	}

	void DefaultSink::set_formatter(std::unique_ptr<spdlog::formatter> sink_formatter)
	{
		m_defaultLogger->set_formatter(std::move(sink_formatter));
	}
}
