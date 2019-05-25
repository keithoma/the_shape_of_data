#include <sgfx/color.hpp>
#include <sgfx/image.hpp>
#include <sgfx/window.hpp>

#include <chrono>
#include <thread>

int main(int argc, char* argv[])
{
    using namespace sgfx;
    using namespace std::chrono_literals;

    window main_window{1024, 768};

    auto bg = load_rle("sample_bg.ppm.rle");
    auto fg = load_rle("sample_fg.ppm.rle");

    draw(main_window, bg, {0, 0});
    draw(main_window, fg, {200, 200}, color::gray);

    main_window.show();
    std::this_thread::sleep_for(5s);

    auto encoded = rle_encode(main_window);
    save_rle(encoded, "screenshot.rle");

    return 0;
}
