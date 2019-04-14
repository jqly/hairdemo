#ifndef XY_OPENGL
#define XY_OPENGL

#include <string>
#include <functional>
#include <unordered_map>

#include "glad/glad.h"
#include "./xy_glfw.h"
#include "./xy_calc.h"
#include "./xy_ext.h"
#include "./xy_rt.h"
#include "./tiny_obj_loader.h"

////
// Shader code.
////

GLuint MakeShader(std::string comp_shader);
GLuint MakeShader(std::string vert_shader, std::string frag_shader);
GLuint MakeShader(std::string vert_shader, std::string geom_shader, std::string frag_shader);

GLuint ResolveShader(
	const std::unordered_map<std::string, std::string> &symbols,
	std::string shader_file_path, std::string shader_include_dir);

// Use glGetUniformLocation(program, name) to get location.

inline void ShaderAssign(GLint location, const xy::mat4 &value)
{
	glUniformMatrix4fv(location, 1, GL_FALSE, value.data);
}

inline void ShaderAssign(GLint location, const xy::vec3 &value)
{
	glUniform3f(location, value.r, value.g, value.b);
}

inline void ShaderAssign(GLint location, const xy::vec2 &value) {
	glUniform2f(location, value.x, value.y);
}

inline void ShaderAssign(GLint location, const xy::vec4 &value) {
	glUniform4f(location, value.x, value.y, value.z, value.w);
}

inline void ShaderAssign(GLint location, const GLuint value) {
	glUniform1ui(location, value);
}

inline void ShaderAssign(GLint location, const GLint value) {
	glUniform1i(location, value);
}

inline void ShaderAssign(GLint location, const GLfloat value) {
	glUniform1f(location, value);
}

////
// Render target code.
////

struct RenderTarget {
	int width, height, msaa;
	GLuint fbo, color, depth;
	bool color_as_texture, depth_attachment;
	int mipmap_levels;
};

class RenderTargetFactory {

public:

	RenderTargetFactory& Size(int width, int height);
	RenderTargetFactory& ColorAsTexture(GLint mag_filter, GLint min_filter, GLsizei mipmap_levels, GLenum color_format);
	RenderTargetFactory& DepthAsRenderbuffer(GLenum depth_format);
	RenderTargetFactory& ColorDepthAsRenderbuffer(GLenum color_format, GLenum depth_format);
	RenderTargetFactory& ColorDepthAsRenderbufferMSAA(GLenum color_format, GLenum depth_format, int msaa);
	RenderTarget Create();

private:

	int width_ = 0, height_ = 0, msaa_ = 0;
	GLint mag_filter_ = 0, min_filter_ = 0, mipmap_levels_ = 0;
	GLenum color_format_ = 0, depth_format_ = 0;
	bool color_as_texture_ = false, depth_attachment_ = false;

};

void DelRenderTarget(RenderTarget & target);


////
// AABB code.
////

struct AABB {
	xy::vec3 inf, sup;

	AABB();
	AABB(const std::vector<xy::vec3> &ps);

	void Extend(const AABB &aabb);
	void Extend(const xy::vec3 &p);
	void Extend(const std::vector<xy::vec3> &ps);

	bool IsInside(const xy::vec3 &p) const;

	xy::vec3 Center() const;
	xy::vec3 Lengths() const;
};


////
// HairAsset-loading code.
////

struct HairAsset {
	std::string path;
	std::vector<int> vcounts;
	std::vector<xy::vec3> positions;
};

HairAsset MakeHairAsset(std::string path);

struct HairGAsset {
	const HairAsset *asset;
	GLuint vao;
	GLuint bufs[3];
	int num_indices;

	void DrawElements(const std::vector<int> &&attribs) const;

	xy::mat4 model;
	float radius;
	float alpha;
	AABB bounds;
};

HairGAsset MakeHairGAsset(const HairAsset &asset);

void DelHairGAsset(HairGAsset & gasset);

////
// Asset-loading code.
////

struct ObjAsset {
	std::string obj_path;
	std::string mtl_dirpath;
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> mtls;
};

ObjAsset MakeObjAsset(std::string obj_path, std::string mtl_dirpath);
AABB MakeObjBounds(const ObjAsset &asset);

struct ObjGAsset_Part {
	GLuint vao;
	GLuint ebo;
	// Position,Normal,TexCoord,Tangent,Bitangent
	GLuint bufs[5];
	int num_vertices;

	void DrawElements(GLenum mode, const std::vector<int> &&attribs) const;

};

struct ObjGAsset_Mtl {
	xy::vec3 Ka;
	xy::vec3 Kd;
	xy::vec3 Ks;
	GLuint map_Ka;
	GLuint map_Kd;
	GLuint map_Ks;
	GLuint map_d;
	GLuint map_bump;
	GLuint map_AO;
};

struct ObjGAsset {
	const ObjAsset *asset;
	std::vector<ObjGAsset_Part> parts;
	std::vector<ObjGAsset_Mtl> mtls;
	std::vector<int> part2mtl;
	std::vector<int> part2shape;

	xy::mat4 model;
	AABB bounds;
};

ObjGAsset MakeObjGAsset(const ObjAsset &asset);
void DelObjGAsset(ObjGAsset &gasset);


class Camera {
public:
	virtual xy::mat4 View() const = 0;
	virtual xy::mat4 Proj() const = 0;
	virtual xy::vec3 Pos() const = 0;
	virtual void HandleInput(const XYInput &input) = 0;
};

class LightCamera :public Camera {
public:
	LightCamera(const AABB &bounds, xy::vec3 forward, xy::vec3 up);

	xy::mat4 View() const override;
	xy::mat4 Proj() const override;
	xy::vec3 Pos() const override;

	void HandleInput(const XYInput &input) override {}

private:
	xy::vec3 pos_;
	xy::mat4 view_matrix_, proj_matrix_;
};

class ArcballCamera : public Camera {
public:
	ArcballCamera(AABB bounds, xy::vec3 forward, xy::vec3 up, float FoVy, float aspect);

	xy::mat4 View() const override;
	xy::mat4 Proj() const override;
	xy::vec3 Pos() const override;

	void HandleInput(const XYInput &input) override;

private:
	AABB bounds_;
	xy::vec3 forward_, up_;
	float FoVy_, aspect_, bounds_radius_, dist_, nearp_, farp_;
	bool tracking{ false };
	xy::quat track_rot_{ 1,0,0,0 }, track_rot_prev_{ 1,0,0,0 };
	xy::vec3 first_hit_;
	xy::PhysicsParticle1D zoom_part_;
	xy::Catcher cat_;
	xy::mat4 init_view_matrix_;
	xy::vec3 init_pos_;
};

class WanderCamera : public Camera {

public:
	WanderCamera(
		xy::vec3 position, xy::vec3 target, xy::vec3 up,
		float FoVy, float aspect, float nearp, float farp);

	xy::vec3 Pos() const override;
	xy::mat4 View() const override;
	xy::mat4 Proj() const override;

	void HandleInput(const XYInput &input) override;

private:
	xy::vec3 pos_;
	xy::vec3 forward_, up_;
	float FoVy_, aspect_, nearp_, farp_;
	xy::PhysicsParticle1D par_m_lr_, par_m_fb_, par_r_lr_, par_r_ud_, par_r_roll_;
	xy::Catcher cat_;
};


#endif // !XY_OPENGL

