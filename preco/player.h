#pragma once

#include <list>
#include <opencv2/core/types.hpp>

#include "cell.h"

class player
{
public:
	static constexpr int p1 = 0;
	static constexpr int p2 = 1;
	static constexpr int axis = 0;
	static constexpr int child = 1;

	static constexpr int rows = 12;
	static constexpr int cols = 6;
	static constexpr int nxt_max = 2;
	static constexpr int nxt_child_max = 2;

	static constexpr int b = 0;
	static constexpr int g = 1;
	static constexpr int r = 2;

	static constexpr int h = 0;
	static constexpr int s = 1;
	static constexpr int v = 2;

private:
	int player_idx_;

	cv::Rect field_frame_rect_;
	int cell_width_;
	int cell_height_;
	cell field_cells_[rows][cols];

	cell nxt_cells_[nxt_max][nxt_child_max];

	cell end_cell_;

	cv::Rect wait_character_selection_rect_;
	cv::Rect wait_reset_rect_;

	void init_field_cells();
	void init_wait_character_selection_rect();
	void init_wait_reset_rect();
	void init_wait_end_rect();

public:
	explicit player(int player_idx);

	[[nodiscard]] bool wait_character_selection(const cv::Mat& org_mat) const;
	[[nodiscard]] bool wait_game_start(const cv::Mat& org_mat) const;
	[[nodiscard]] bool game(int cur_no, const cv::Mat& org_mat, const std::list<cv::Mat>& mat_histories);
	[[nodiscard]] bool wait_nxt_stable(const cv::Mat& org_mat, const std::list<cv::Mat>& mat_histories);
	void debug_render(const cv::Mat& debug_mat) const;

};
