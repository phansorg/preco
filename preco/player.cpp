#include "player.h"

#include <opencv2/imgproc.hpp>

#include "settings.h"

player::player(const int player_idx)
{
	player_idx_ = player_idx;

	auto& json = settings::get_instance()->json;
	field_x_ = json["player_field_x"][player_idx_].get<int>();
	field_y_ = json["player_field_y"].get<int>();
	field_w_ = json["player_field_w"].get<int>();
	field_h_ = json["player_field_h"].get<int>();
	next1_x_ = json["player_next1_x"][player_idx_].get<int>();
	next1_y_ = json["player_next1_y"].get<int>();
	next2_x_ = json["player_next2_x"][player_idx_].get<int>();
	next2_y_ = json["player_next2_y"].get<int>();

	next1_w_ = field_w_ / cols;
	next1_h_ = field_h_ / rows;
	next2_w_ = next1_w_ * 4 / 5;
	next2_h_ = next1_h_ * 4 / 5;

	init_field_rect();
	init_next_rect_vector();
	init_wait_character_selection_rect();
	init_wait_reset_rect();
}

// ============================================================
// rect
// ============================================================

void player::init_field_rect()
{
	field_rect_ = cv::Rect(
		field_x_,
		field_y_,
		field_w_,
		field_h_
	);
}

void player::init_next_rect_vector()
{
	auto x = next1_x_ + next1_w_ / 4;
	auto y = next1_y_ + next1_h_ / 4;
	auto w = next1_w_ / 2;
	auto h = next1_h_ / 2;
	next_rect_vector_.emplace_back(x, y, w, h);

	y += next1_h_;
	next_rect_vector_.emplace_back(x, y, w, h);

	x = next2_x_ + next2_w_ / 4;
	y = next2_y_ + next2_h_ / 4;
	w = next2_w_ / 2;
	h = next2_h_ / 2;
	next_rect_vector_.emplace_back(x, y, w, h);

	y += next2_h_;
	next_rect_vector_.emplace_back(x, y, w, h);
}

void player::init_wait_character_selection_rect()
{
	// 1P盤面の上半分領域が全て赤であればOK
	// 2P盤面の下半分領域が全て緑であればOK
	const auto y = player_idx_ == p1 ? field_y_ : field_y_ + field_h_ / 2;
	wait_character_selection_rect_ = cv::Rect(
		field_x_,
		y,
		field_w_,
		field_h_ / 2
	);
}

void player::init_wait_reset_rect()
{
	// 盤面の中央領域が全て黒であればOK
	wait_reset_rect_ = cv::Rect(
		field_x_ + field_w_ / 2,
		field_y_ + field_h_ / 2,
		field_w_ / 10,
		field_h_ / 10
	);
}

// ============================================================
// game
// ============================================================

bool player::wait_character_selection(const cv::Mat& org_mat) const
{
	std::vector<cv::Mat> channels;
	cv::Point* pos = nullptr;

	const auto roi_mat = org_mat(wait_reset_rect_);
	split(roi_mat, channels);
	if (player_idx_ == p1)
	{
		// 1P盤面の上半分領域が全て赤であればOK
		if (!checkRange(channels[b], true, pos, 0, 100)) return false;
		if (!checkRange(channels[g], true, pos, 0, 100)) return false;
		if (!checkRange(channels[r], true, pos, 180, 255)) return false;
	}
	else
	{
		// 2P盤面の下半分領域が全て緑であればOK
		if (!checkRange(channels[b], true, pos, 0, 100)) return false;
		if (!checkRange(channels[g], true, pos, 180, 255)) return false;
		if (!checkRange(channels[r], true, pos, 0, 100)) return false;
	}

	return true;
}

bool player::wait_game_start(const cv::Mat& org_mat) const
{
	// 盤面の中央領域が全て黒であればOK
	cv::Point* pos = nullptr;
	if (const auto roi_mat = org_mat(wait_reset_rect_);
		!checkRange(roi_mat, true, pos, 0, 30)) return false;

	return true;
}

// ============================================================
// debug
// ============================================================

void player::debug_wait_init(const cv::Mat& debug_mat) const
{
	// fieldの線を描画
	auto rect = field_rect_;
	auto x1 = rect.x;
	auto y1 = rect.y;
	auto x2 = rect.x + rect.width;
	auto y2 = rect.y + rect.height;
	rectangle(debug_mat, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 0, 255), 1);

	// nextの線を描画
	for (const auto& next_rect : next_rect_vector_)
	{
		rect = next_rect;
		x1 = rect.x;
		y1 = rect.y;
		x2 = rect.x + rect.width;
		y2 = rect.y + rect.height;
		rectangle(debug_mat, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 0, 255), 1);
	}
}
