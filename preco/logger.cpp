#include "logger.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "spdlog/sinks/daily_file_sink.h"
#include <spdlog/async.h>

#include "settings.h"

constexpr auto q_size = 8192;
constexpr auto thread_count = 1;

constexpr auto rotation_hour = 0;
constexpr auto rotation_minute = 0;
constexpr auto truncate = false;
constexpr auto max_files = 7;

void init_logger() {
    auto& json = Settings::get_instance()->json_;
    auto file_path = json["log_path"].get<std::string>();

    spdlog::init_thread_pool(q_size, thread_count);

    // stdoutログの設定
    const auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt >();
    stdout_sink->set_level(spdlog::level::trace);

    // 日付ローテーションファイルログの設定
    const auto daily_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(
        file_path,
        rotation_hour,
        rotation_minute,
        truncate,
        max_files);
    daily_sink->set_level(spdlog::level::trace);

    // loggerに登録
    std::vector<spdlog::sink_ptr> sinks{
        stdout_sink,
        daily_sink
    };
    const auto logger = std::make_shared<spdlog::async_logger>(
        kLoggerMain,
        sinks.begin(),
        sinks.end(),
        spdlog::thread_pool(),
        spdlog::async_overflow_policy::block);
    logger->set_level(spdlog::level::trace);
    register_logger(logger);
}
