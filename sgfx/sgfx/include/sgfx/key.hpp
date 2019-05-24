#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace sgfx {

// TODO: should be `enum class key { ... };`

class key_id {
  public:
	explicit constexpr key_id(char c) : id_{(c >= 'a' && c <= 'z') ? (c - 'a' + 'A') : c} {}

	explicit constexpr key_id(int id) : id_{id} {}

  private:
	int id_;
	friend class window;
};

namespace key {

// do we need this?
inline constexpr const key_id space{GLFW_KEY_SPACE};


inline constexpr const key_id escape{GLFW_KEY_ESCAPE};

// do we need this?
inline constexpr const key_id left{GLFW_KEY_LEFT};
inline constexpr const key_id right{GLFW_KEY_RIGHT};

inline constexpr const key_id up{GLFW_KEY_UP};
inline constexpr const key_id down{GLFW_KEY_DOWN};

// added
inline constexpr const key_id backspace{GLFW_KEY_BACKSPACE};
inline constexpr const key_id wkey{GLFW_KEY_W};
inline constexpr const key_id skey{GLFW_KEY_S};

}  // namespace key

}  // namespace sgfx
