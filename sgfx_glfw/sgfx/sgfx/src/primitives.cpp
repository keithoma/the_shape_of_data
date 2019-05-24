#include <sgfx/primitives.hpp>

#include <algorithm>

void sgfx::plot(canvas_view target, point p, color::rgb_color col)
{
	target.pixels()[p.y*target.width()+p.x]=col;
}

void sgfx::clear(canvas_view target, color::rgb_color col)
{
	std::fill_n(target.pixels(),target.width()*target.height(),col);
}

void sgfx::hline(canvas_view target, point p, std::uint16_t length, color::rgb_color col)
{
	if(p.y<0 || p.y>=target.height())
		return;
	
	int start=std::max(p.x,0);
	int stop=std::min(static_cast<int>(target.width()),p.x+length);
	
	if(stop>start)
		std::fill_n(&target.pixels()[p.y*target.width()+start],stop-start,col);
}

void sgfx::vline(canvas_view target, point p, std::uint16_t length, color::rgb_color col)
{
	if(p.x<0 || p.x>=target.width())
		return;
		
	int start=std::max(p.y,0);
	int stop=std::min(static_cast<int>(target.height()),p.y+length);
	
	while(start!=stop)
		plot(target,{p.x,start++},col);
}

void sgfx::fill(canvas_view target, rectangle rect, color::rgb_color col)
{
	int start_y=std::max(rect.top_left.y,0);
	int stop_y=std::min(static_cast<int>(target.height()),rect.top_left.y+rect.size.h);
	
	while(start_y!=stop_y)
		hline(target,{rect.top_left.x,start_y++},rect.size.w,col);
}

void sgfx::line(canvas_view target, point p0, point p1, color::rgb_color col)
{
	if(p0.y==p1.y)
	{
		hline(target,p0,p1.x-p0.x,col);
		return;
	}
	
	if(p0.x==p1.x)
	{
		vline(target,p0,p1.y-p0.y,col);
		return;
	}
	
	const auto xline=[&](point p0, point p1)
	{
		auto delta_x=p1.x-p0.x;
		auto delta_y=p1.y-p0.y;
		auto error=2*delta_y-delta_x;
		
		auto y=p0.y;
		auto sign=delta_y>0?1:-1;
		delta_y*=sign;
		for(int x=p0.x;x<p1.x;++x)
		{
			plot(target,{x,y},col);
			if(error>0)
			{
				y+=sign;
				error-=2*delta_x;
			}
			error+=2*delta_y;
		}
	};
	
	const auto yline=[&](point p0, point p1)
	{
		auto delta_x=p1.x-p0.x;
		auto delta_y=p1.y-p0.y;
		auto error=2*delta_x-delta_y;
		
		auto x=p0.x;
		auto sign=delta_x>0?1:-1;
		delta_x*=sign;
		for(int y=p0.y;y<p1.y;++y)
		{
			plot(target,{x,y},col);
			if(error>0)
			{
				x+=sign;
				error-=2*delta_y;
			}
			error+=2*delta_x;
		}
	};
	
	if(std::abs(p1.y-p0.y)<std::abs(p1.x-p0.x))
	{
		if(p0.x>p1.x)
			xline(p1,p0);
		else
			xline(p0,p1);
	}
	else
	{
		if(p0.y>p1.y)
			yline(p1,p0);
		else
			yline(p0,p1);
	}
}
