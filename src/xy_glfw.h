#ifndef XY_GLFW
#define XY_GLFW

#include <string>
#include <functional>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "./xy_calc.h"


GLFWwindow * MakeGlfw45Window(int width, int height, std::string title, bool enable_debug, bool enable_vsync);

class XYKeyboardInput {
public:

	XYKeyboardInput(GLFWwindow *window);

	void PollEvents();

	bool Esc() const { return Esc_; }
	bool A() const { return A_; }
	bool W() const { return W_; }
	bool S() const { return S_; }
	bool D() const { return D_; }

	bool H() const { return H_; }
	bool J() const { return J_; }
	bool K() const { return K_; }
	bool L() const { return L_; }

	bool N() const { return N_; }
	bool M() const { return M_; }

private:
	bool Esc_{ false };
	bool A_{ false };
	bool W_{ false };
	bool S_{ false };
	bool D_{ false };

	bool H_{ false };
	bool J_{ false };
	bool K_{ false };
	bool L_{ false };

	bool N_{ false };
	bool M_{ false };

	GLFWwindow *wptr_;
};

class XYMouseInput {
public:

	XYMouseInput(GLFWwindow *window);

	void PollEvents();

	bool ButtonLeftPress() const { return mouse_state_ == MouseState::Press; }
	bool ButtonLeftRelease() const { return mouse_state_ == MouseState::Release; }
	xy::vec2 CursorPosition() const { return mouse_position_; }
	xy::vec2 ScrollOffset() const;

private:
	enum class MouseState { Press, Release };

	MouseState mouse_state_{ MouseState::Release };
	xy::vec2 mouse_position_{ 0.f,0.f };
	xy::vec2 past_scroll_offset_, d_scroll_offset_;

	GLFWwindow *wptr_;
};

class XYInput {
public:

	XYInput(GLFWwindow *window);

	void PollEvents();

	int WindowWidth() const { return width_; }
	int WindowHeight() const { return height_; }

	const XYMouseInput &MouseInput() const { return mouse_input_; }
	const XYKeyboardInput &KeyboardInput() const { return keyboard_input_; }

private:
	GLFWwindow *wptr_;
	int width_, height_;
	XYMouseInput mouse_input_;
	XYKeyboardInput keyboard_input_;
};


#endif // !XY_GLFW

