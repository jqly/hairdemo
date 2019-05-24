#include "./xy_glfw.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "./xy_ext.h"


void Glfw45WindowDebugCallback(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar * message,
	const void * userParam)
{
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	xy::Print(
		"---------------\n"
		"Debug message ({}): {}",
		id, message
	);

	switch (source) {
	case GL_DEBUG_SOURCE_API:
		xy::Print("Source: API\n"); break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		xy::Print("Source: Window System\n"); break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		xy::Print("Source: Shader Compiler\n"); break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		xy::Print("Source: Third Party\n"); break;
	case GL_DEBUG_SOURCE_APPLICATION:
		xy::Print("Source: Application\n"); break;
	case GL_DEBUG_SOURCE_OTHER:
		xy::Print("Source: Other\n"); break;
	}

	switch (type) {
	case GL_DEBUG_TYPE_ERROR:
		xy::Print("Type: Error\n"); break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		xy::Print("Type: Deprecated Behaviour\n"); break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		xy::Print("Type: Undefined Behaviour\n"); break;
	case GL_DEBUG_TYPE_PORTABILITY:
		xy::Print("Type: Portability\n"); break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		xy::Print("Type: Performance\n"); break;
	case GL_DEBUG_TYPE_MARKER:
		xy::Print("Type: Marker\n"); break;
	case GL_DEBUG_TYPE_PUSH_GROUP:
		xy::Print("Type: Push Group\n"); break;
	case GL_DEBUG_TYPE_POP_GROUP:
		xy::Print("Type: Pop Group\n"); break;
	case GL_DEBUG_TYPE_OTHER:
		xy::Print("Type: Other\n"); break;
	}

	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH:
		xy::Print("Severity: high\n"); break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		xy::Print("Severity: medium\n"); break;
	case GL_DEBUG_SEVERITY_LOW:
		xy::Print("Severity: low\n"); break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		xy::Print("Severity: notification\n"); break;
	}
	return;
}

GLFWwindow * MakeGlfw45Window(int width, int height, std::string title, bool enable_debug, bool enable_vsync)
{

	if (glfwInit() != GLFW_TRUE)
		XY_Die("glfw init failed");

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, enable_debug);

	auto wptr = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
	if (wptr == nullptr) {
		XY_Die("glfw cannot create window");
	}

	glfwMakeContextCurrent(wptr);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	glfwSwapInterval(0);
	if (enable_vsync)
		glfwSwapInterval(1);

	GLint flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(Glfw45WindowDebugCallback, nullptr);
		glDebugMessageControl(GL_DEBUG_SOURCE_API,
			GL_DEBUG_TYPE_ERROR,
			GL_DEBUG_SEVERITY_MEDIUM,
			0, nullptr, GL_TRUE);
	}

	return wptr;

}

XYKeyboardInput::XYKeyboardInput(GLFWwindow * window)
	:wptr_{ window }
{}

void XYKeyboardInput::PollEvents()
{


	Esc_ = (glfwGetKey(wptr_, GLFW_KEY_ESCAPE) == GLFW_PRESS);
	A_ = (glfwGetKey(wptr_, GLFW_KEY_A) == GLFW_PRESS);
	W_ = (glfwGetKey(wptr_, GLFW_KEY_W) == GLFW_PRESS);
	S_ = (glfwGetKey(wptr_, GLFW_KEY_S) == GLFW_PRESS);
	D_ = (glfwGetKey(wptr_, GLFW_KEY_D) == GLFW_PRESS);
	H_ = (glfwGetKey(wptr_, GLFW_KEY_H) == GLFW_PRESS);
	J_ = (glfwGetKey(wptr_, GLFW_KEY_J) == GLFW_PRESS);
	K_ = (glfwGetKey(wptr_, GLFW_KEY_K) == GLFW_PRESS);
	L_ = (glfwGetKey(wptr_, GLFW_KEY_L) == GLFW_PRESS);
	N_ = (glfwGetKey(wptr_, GLFW_KEY_N) == GLFW_PRESS);
	M_ = (glfwGetKey(wptr_, GLFW_KEY_M) == GLFW_PRESS);

}

namespace xy_mouse_input
{

static c3d::Vec2 scroll_offset = { 0.f };

}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	xy_mouse_input::scroll_offset += c3d::Vec2{static_cast<float>(xoffset), static_cast<float>(yoffset)};
}

XYMouseInput::XYMouseInput(GLFWwindow * window)
	:wptr_{ window }, past_scroll_offset_{ xy_mouse_input::scroll_offset }, d_scroll_offset_{ 0.f }
{
	glfwSetScrollCallback(window, scroll_callback);
}

void XYMouseInput::PollEvents()
{

	d_scroll_offset_ = xy_mouse_input::scroll_offset - past_scroll_offset_;
	past_scroll_offset_ = xy_mouse_input::scroll_offset;

	int glfw_mouse_state = glfwGetMouseButton(wptr_, GLFW_MOUSE_BUTTON_LEFT);

	switch (mouse_state_) {

	case MouseState::Press:
		if (glfw_mouse_state == GLFW_RELEASE)
			mouse_state_ = MouseState::Release;
		break;

	case MouseState::Release:
		if (glfw_mouse_state == GLFW_PRESS)
			mouse_state_ = MouseState::Press;
		break;

	default:
		break;

	}

	double xpos, ypos;
	glfwGetCursorPos(wptr_, &xpos, &ypos);
	mouse_position_ = { static_cast<float>(xpos),static_cast<float>(ypos) };
}

c3d::Vec2 XYMouseInput::ScrollOffset() const
{
	return d_scroll_offset_;
}

XYInput::XYInput(GLFWwindow * window)
	:wptr_{ window }, mouse_input_{ window }, keyboard_input_{ window }
{
}

void XYInput::PollEvents()
{
	glfwPollEvents();

	mouse_input_.PollEvents();
	keyboard_input_.PollEvents();

	glfwGetWindowSize(wptr_, &width_, &height_);
}