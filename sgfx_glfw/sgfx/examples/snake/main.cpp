#include <sgfx/color.hpp>
#include <sgfx/primitives.hpp>
#include <sgfx/window.hpp>

#include <array>
#include <deque>
#include <iostream>
#include <random>

int random_uniform_int(int max)
{
	static std::mt19937 random_gen{std::random_device{}()};
	std::uniform_int_distribution<int> dist(0,max-1);
	return dist(random_gen);
}

int main(int argc, char* argv[])
{
	using namespace sgfx;
	constexpr auto scale=8;
	constexpr auto board_size=64;
	point origin{0,0};
	
	window main_window{scale*board_size,scale*board_size, "Snake!"};
	std::array<std::array<bool,board_size>,board_size> board{};
	std::deque<point> snake;
	point head{0,0};
	snake.push_back(head);
	board[head.y][head.x]=true;
	point price{board_size/4,board_size/4};
	vec direction{1,0};
	int framecount=0, max_framecount=60;
	
	while(main_window.handle_events() && !main_window.should_close())
	{
		if(main_window.is_pressed(key::down)) direction = {0,1};
		if(main_window.is_pressed(key::right)) direction = {1,0};
		if(main_window.is_pressed(key::left)) direction = {-1,0};
		if(main_window.is_pressed(key::up)) direction = {0,-1};
		if(++framecount==max_framecount)
		{
			head+=direction;
			head.x%=board[0].size();
			head.y%=board.size();
			if(board[head.y][head.x]){ std::cerr<<"You died!\n"; break; };
			board[head.y][head.x]=true;
			framecount=0;
			snake.push_front(head);
			if(head==price)
			{
				while(head==price)
					price={random_uniform_int(board_size),random_uniform_int(board_size)};
				max_framecount=std::max(1,max_framecount-1);
			}
			else
			{
				board[snake.back().y][snake.back().x]=false;
				snake.pop_back();
			}
		}

		clear(main_window, color::black);
		for(int y=0;y<(int)board.size();++y)
			for(int x=0;x<(int)board[y].size();++x)
				if(board[y][x]) fill(main_window, {origin+scale*vec{x,y},{scale,scale}}, color::white);
		fill(main_window,{origin+scale*(price-origin),{scale,scale}},color::red);
		main_window.show();
	}
	return 0;
}
