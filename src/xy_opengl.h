#ifndef XY_OPENGL
#define XY_OPENGL

#include <string>
#include <functional>
#include <unordered_map>

#include "glad/glad.h"
#include "./xy_glfw.h"
#include "calc3.h"
#include "./xy_ext.h"
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

inline void ShaderAssign(GLint location, const c3d::Mat4 &value)
{
	glUniformMatrix4fv(location, 1, GL_FALSE, begin(value));
}

inline void ShaderAssign(GLint location, const c3d::Vec3 &value)
{
	glUniform3f(location, value.x, value.y, value.z);
}

inline void ShaderAssign(GLint location, const c3d::Vec2 &value) {
	glUniform2f(location, value.x, value.y);
}

inline void ShaderAssign(GLint location, const c3d::Vec4 &value) {
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
	bool color_as_texture_ = false, depth_attachment_ = false, color_attachment_ = false;

};

void DelRenderTarget(RenderTarget & target);


////
// AABB code.
////

struct AABB {
	c3d::Vec3 inf, sup;

	AABB();
	AABB(const std::vector<c3d::Vec3> &ps);

	void Extend(const AABB &aabb);
	void Extend(const c3d::Vec3 &p);
	void Extend(const std::vector<c3d::Vec3> &ps);

	bool IsInside(const c3d::Vec3 &p) const;

	c3d::Vec3 Center() const;
	c3d::Vec3 Lengths() const;
};


////
// HairAsset-loading code.
////


struct HairAsset {
	std::string path;
	std::vector<int> vcounts;
	std::vector<c3d::Vec3> positions;
	float radius[3];
	float transparency[3];
};

HairAsset MakeHairAsset(std::string path);

struct HairGAsset {
	const HairAsset *asset;
	GLuint vao;
	GLuint bufs[2];
	GLuint index_bufs[3];
	int num_indices[3];

	void DrawIndexed(int K, const std::vector<int> &&attribs) const;

	c3d::Mat4 model;
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
	c3d::Vec3 Ka;
	c3d::Vec3 Kd;
	c3d::Vec3 Ks;
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

	c3d::Mat4 model;
	AABB bounds;
};

ObjGAsset MakeObjGAsset(const ObjAsset &asset);
void DelObjGAsset(ObjGAsset &gasset);


class Camera {
public:
	virtual c3d::Mat4 View() const = 0;
	virtual c3d::Mat4 Proj() const = 0;
	virtual c3d::Vec3 Pos() const = 0;
	virtual void HandleInput(const XYInput &input) = 0;
};

class LightCamera :public Camera {
public:
	LightCamera(const AABB &bounds, c3d::Vec3 forward, c3d::Vec3 up);

	c3d::Mat4 View() const override;
	c3d::Mat4 Proj() const override;
	c3d::Vec3 Pos() const override;

	void HandleInput(const XYInput &input) override {}

private:
	c3d::Vec3 pos_;
	c3d::Mat4 view_matrix_, proj_matrix_;
};

class ArcballCamera : public Camera {
public:
	ArcballCamera(AABB bounds, c3d::Vec3 forward, c3d::Vec3 up, float FoVy, float aspect);

	c3d::Mat4 View() const override;
	c3d::Mat4 Proj() const override;
	c3d::Vec3 Pos() const override;

	void HandleInput(const XYInput &input) override;

private:
	AABB bounds_;
	c3d::Vec3 forward_, up_;
	float FoVy_, aspect_, bounds_radius_, dist_, nearp_, farp_;
	bool tracking{ false };
	c3d::Quat track_rot_{ 1,0,0,0 }, track_rot_prev_{ 1,0,0,0 };
	c3d::Vec3 first_hit_;
	xy::PhysicsParticle1D zoom_part_;
	xy::Catcher cat_;
	c3d::Mat4 init_view_matrix_;
	c3d::Vec3 init_pos_;
};

class WanderCamera : public Camera {

public:
	WanderCamera(
		c3d::Vec3 position, c3d::Vec3 target, c3d::Vec3 up,
		float FoVy, float aspect, float nearp, float farp);

	c3d::Vec3 Pos() const override;
	c3d::Mat4 View() const override;
	c3d::Mat4 Proj() const override;

	void HandleInput(const XYInput &input) override;

private:
	c3d::Vec3 pos_;
	c3d::Vec3 forward_, up_;
	float FoVy_, aspect_, nearp_, farp_;
	xy::PhysicsParticle1D par_m_lr_, par_m_fb_, par_r_lr_, par_r_ud_, par_r_roll_;
	xy::Catcher cat_;
};


#endif // !XY_OPENGL

