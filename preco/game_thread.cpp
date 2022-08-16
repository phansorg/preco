#include "game_thread.h"
#include "base_thread.h"

#include "settings.h"
#include "logger.h"

game_thread::game_thread()
{
	thread_loop_ = true;

	auto& json = settings::get_instance()->json;
	cur_no_ = json["capture_start_no"].get<int>();
	debug_write_ = json["game_debug_write"].get<bool>();
	debug_path_ = json["game_debug_path"].get<std::string>();
	mode_ = static_cast<game_mode>(json["game_start_mode"].get<int>());

	capture_end_ = false;

	players_.push_back(std::make_unique<player>(player::p1));
	players_.push_back(std::make_unique<player>(player::p2));
}

void game_thread::run()
{
	const auto logger = spdlog::get(logger_main);
	SPDLOG_LOGGER_DEBUG(logger, "start");

	while (thread_loop_)
	{
		process();
		SPDLOG_LOGGER_TRACE(logger, "sleep");
		std::this_thread::sleep_for(std::chrono::milliseconds(thread_sleep_ms));
	}

	SPDLOG_LOGGER_DEBUG(logger, "end");
}

void game_thread::request_end()
{
	thread_loop_ = false;
}

void game_thread::set_capture_end()
{
	capture_end_ = true;
}

bool game_thread::is_mat_queue_max() const
{
	return mat_queue_.size() >= mat_queue_max_size_;
}

void game_thread::add_mat_queue(const cv::Mat& mat)
{
	std::lock_guard lock(mat_queue_mutex_);
	mat_queue_.push(mat);
}

void game_thread::process()
{
	while(!mat_queue_.empty())
	{
		cv::Mat org_mat = pop();

		switch(mode_)
		{
		case game_mode::wait_character_selection:
			wait_character_selection(org_mat);
			break;
		case game_mode::wait_start:
			wait_start(org_mat);
			break;
		case game_mode::wait_init:
			debug_wait_init(org_mat);
			break;
		}

		cur_no_++;
	}

	if (capture_end_&& mat_queue_.empty())
	{
		request_end();
	}
}

cv::Mat game_thread::pop()
{
	std::lock_guard lock(mat_queue_mutex_);
	cv::Mat org_mat = mat_queue_.front();
	mat_queue_.pop();
	return org_mat;
}

void game_thread::wait_character_selection(const cv::Mat& org_mat)
{
	const auto logger = spdlog::get(logger_main);

	std::vector<cv::Mat> channels;
	cv::Point* pos = nullptr;

	// 1P�Ֆʂ̏㔼���̈悪�S�ĐԂł����OK
	const auto& p1 = players_[player::p1];
	auto roi_mat = org_mat(p1->wait_character_rect);
	split(roi_mat, channels);
	if (!checkRange(channels[b], true, pos, 0, 100)) return;
	if (!checkRange(channels[g], true, pos, 0, 100)) return;
	if (!checkRange(channels[r], true, pos, 180, 255)) return;

	// 2P�Ֆʂ̉������̈悪�S�ė΂ł����OK
	const auto& p2 = players_[player::p2];
	roi_mat = org_mat(p2->wait_character_rect);
	split(roi_mat, channels);
	if (!checkRange(channels[b], true, pos, 0, 100)) return;
	if (!checkRange(channels[g], true, pos, 180, 255)) return;
	if (!checkRange(channels[r], true, pos, 0, 100)) return;

	// ���Z�b�g�҂��ɑJ��
	mode_ = game_mode::wait_start;
	logger->info("No:{} game_mode:wait_start", cur_no_);
}

void game_thread::wait_start(const cv::Mat& org_mat)
{
	const auto logger = spdlog::get(logger_main);

	// �Ֆʂ̒����̈悪�S�č��ł����OK
	for (const auto& player : players_)
	{
		cv::Point* pos = nullptr;
		if (const auto roi_mat = org_mat(player->wait_reset_rect);
			!checkRange(roi_mat, true, pos, 0, 30)) return;
	}

	// �������҂��ɑJ��
	mode_ = game_mode::wait_init;
	logger->info("No:{} game_mode:wait_init", cur_no_);
}

void game_thread::debug_wait_init(const cv::Mat& org_mat) const
{
	if (!settings::debug)
	{
		return;
	}

	auto debug_mat = org_mat.clone();

	for (const auto& player : players_)
	{
		// field�̐���`��
		auto rect = player->field_rect;
		auto x1 = rect.x;
		auto y1 = rect.y;
		auto x2 = rect.x + rect.width;
		auto y2 = rect.y + rect.height;
		rectangle(debug_mat, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 0, 255), 1);

		// next�̐���`��
		for (const auto& next_rect : player->next_rect_vector)
		{
			rect = next_rect;
			x1 = rect.x;
			y1 = rect.y;
			x2 = rect.x + rect.width;
			y2 = rect.y + rect.height;
			rectangle(debug_mat, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 0, 255), 1);
		}
	}

	// �o�̓t�@�C����
	std::ostringstream file_name;
	file_name << cur_no_ << ".png";

	// �f�B���N�g���p�X�Əo�̓t�@�C����������
	std::filesystem::path file_path = debug_path_;
	file_path.append(file_name.str());

	// �t�@�C���o��
	imwrite(file_path.string(), debug_mat);
}