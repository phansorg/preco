#pragma once
#include <memory>
#include <string>

#include "game_thread.h"

enum class CaptureMode { 
	kJpeg = 0,
};

class CaptureThread
{
	const int jpeg_file_zero_count_ = 7;

	bool thread_loop_;

	std::shared_ptr<game_thread> game_thread_ptr_;
	CaptureMode mode_;
	std::string path_;
	int start_no_;
	int last_no_;

	int cur_no_;

public:
	explicit CaptureThread(const std::shared_ptr<game_thread>& game_thread_ptr);

	void run();
	void request_end();

private:
	void process();
	void read_jpeg();
};

