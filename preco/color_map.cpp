#include "color_map.h"

void color_map::reset()
{
	map_.clear();
}

void color_map::insert(const color from_color)
{
	// 追加済み
	if (map_.contains(from_color)) return;

	// 追加
	unsigned char to_key = '1' + map_.size();
	map_.insert(std::make_pair(from_color, to_key));
}

unsigned char color_map::to_key(const color from_color) const
{
	return map_.at(from_color);
}
