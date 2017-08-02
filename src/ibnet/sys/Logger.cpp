#include "Logger.h"

namespace ibnet {
namespace sys {

std::shared_ptr<spdlog::logger> Logger::m_logger;

void Logger::Setup(void)
{
    std::vector<spdlog::sink_ptr> sinks;
    auto colorSink = std::make_shared<spdlog::sinks::ansicolor_sink>(std::make_shared<spdlog::sinks::stdout_sink_mt>());
    colorSink->set_color(spdlog::level::trace, colorSink->white);
    colorSink->set_color(spdlog::level::debug, colorSink->green);
    colorSink->set_color(spdlog::level::info, colorSink->bold + colorSink->blue);
    colorSink->set_color(spdlog::level::warn, colorSink->bold + colorSink->yellow);
    colorSink->set_color(spdlog::level::err, colorSink->bold + colorSink->red);
    colorSink->set_color(spdlog::level::critical, colorSink->bold + colorSink->on_red);
    colorSink->set_color(spdlog::level::off, colorSink->reset);

    sinks.push_back(colorSink);
    sinks.push_back(std::make_shared<spdlog::sinks::simple_file_sink_mt>("ibnet.log", true));

    m_logger = std::make_shared<spdlog::logger>("ibnet", begin(sinks), end(sinks));
    m_logger->set_pattern("[%L][%D][%T.%e][thread-%t]%v");

    m_logger->set_level(spdlog::level::trace);

    m_logger->flush_on(spdlog::level::info);
}

void Logger::Shutdown(void)
{
    m_logger.reset();
}

}
}