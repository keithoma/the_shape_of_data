#include <sgfx/canvas.hpp>

#include <algorithm>

void sgfx::draw(canvas_view target, const canvas& img, point top_left)
{
	if(top_left.x+img.width()<=0 || top_left.x>=target.width())
		return;
	if(top_left.y+img.height()<=0 || top_left.y>=target.height())
		return;
		
	auto x_offset=std::max(-top_left.x,0);
	auto y_offset=std::max(-top_left.y,0);
	
	auto line_length=img.width()-x_offset;
	auto line_num=img.height()-y_offset;

	if(top_left.x+x_offset+line_length>=target.width())
		line_length=target.width()-(top_left.x+x_offset);
		
	if(top_left.y+y_offset+line_num>=target.height())
		line_num=target.height()-(top_left.y+y_offset);
	
	int src_x=x_offset;
	int dest_x=top_left.x+x_offset;
	for(int line=0;line<line_num;++line)
	{
		int src_y=y_offset+line;
		int dest_y=top_left.y+y_offset+line;
		std::copy_n(&img.pixels()[src_y*img.width()+src_x],line_length,&target.pixels()[dest_y*target.width()+dest_x]);
	}
}


void sgfx::draw(canvas_view target, const canvas& img, point top_left, color::rgb_color color_key)
{
	if(top_left.x+img.width()<=0 || top_left.x>=target.width())
		return;
	if(top_left.y+img.height()<=0 || top_left.y>=target.height())
		return;
		
	auto x_offset=std::max(-top_left.x,0);
	auto y_offset=std::max(-top_left.y,0);
	
	auto line_length=img.width()-x_offset;
	auto line_num=img.height()-y_offset;

	if(top_left.x+x_offset+line_length>=target.width())
		line_length=target.width()-(top_left.x+x_offset);
		
	if(top_left.y+y_offset+line_num>=target.height())
		line_num=target.height()-(top_left.y+y_offset);
	
	int src_x=x_offset;
	int dest_x=top_left.x+x_offset;
	for(int line=0;line<line_num;++line)
	{
		int src_y=y_offset+line;
		int dest_y=top_left.y+y_offset+line;
		auto begin=&img.pixels()[src_y*img.width()+src_x];
		auto end=begin+line_length;
		auto target_begin=&target.pixels()[dest_y*target.width()+dest_x];
		std::transform(begin,end,target_begin, target_begin,[&](auto c_dest, auto c_target)
		{
			return c_dest==color_key?c_target:c_dest;
		});
	}
}
