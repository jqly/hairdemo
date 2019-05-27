#include "xy_opengl.h"


class HairRenderer {

public:

	HairRenderer()
		:asset{ nullptr } {}

	virtual void InitGpuResource(const HairAsset *asset) = 0;
	virtual void DelGpuResource() = 0;

	virtual void SetModel(const HairAsset *asset)
	{
		this->asset = asset;
		gasset = MakeHairGAsset(*asset);
	}

	virtual void RenderMainPass(RenderTarget &target, const Camera &camera) = 0;
	virtual void RenderShadowMap(const Camera &camera) {}
	virtual void RenderPrePass(RenderTarget &target, const Camera &camera) {}

	virtual ~HairRenderer() {}

protected:

	const HairAsset *asset;
	HairGAsset gasset;

};

// class PPLLHairRenderer2 {
// public:

// 	PPLLHairRenderer2(c3d::iVec3 ppll_size, c3d::iVec2 shadow_map_size)
// 		: ppll_size_{ppll_size},
// 		shadow_map_size_{shadow_map_size},
// 		depth4fbo_, depth4_{}, alphamap_{}, head_{}, shadow_depth4{},
// 		s_predepth_{0}, s_store_{0}, s_resolve_{0}, s_shadow_map_{0},
// 		atomic_cnt_{0}, hair_nodes_{0}, quat_vao_{0}, quad_buf_{0}
// 	{}
	
// 	void Initialize()
// 	{

// 		InitializeShaders(shader_dir);

// 		depth4fbo_ = RenderTargetFactory()
// 			.Size(ppll_size_.x, ppll_size_.y)
// 			.DepthAsRenderbuffer(GL_DEPTH_COMPONENT24)
// 			.Create();

// 		depth4_ = RenderTargetFactory()
// 			.Size(ppll_size_.x*4, ppll_size_.y)
// 			.ColorAsTexture(GL_NEAREST, GL_NEAREST, 1, GL_R32UI)
// 			.Create();

// 		alphamap_ = RenderTargetFactory()
// 			.Size(ppll_size_.x, ppll_size_.y)
// 			.ColorAsTexture(GL_NEAREST, GL_NEAREST, 1, GL_R32UI)
// 			.Create();

// 		shadow_depth4fbo_ = RenderTargetFactory()
// 			.Size(shadow_map_size.x, shadow_map_size.y)
// 			.DepthAsRenderbuffer(GL_DEPTH_COMPONENT24)
// 			.Create();

// 		shadow_depth4_ = RenderTargetFactory()
// 			.Size(shadow_map_size.x*4, shadow_map_size.y)
// 			.ColorAsTexture(GL_NEAREST, GL_NEAREST, 1, GL_R32UI)
// 			.Create();

// 		head_ = RenderTargetFactory()
// 			.Size(render_layer_width_, render_layer_height_)
// 			.ColorAsTexture(GL_NEAREST, GL_NEAREST, 1, GL_R32UI)
// 			.Create();

// 		glGenBuffers(1, &cnt_);
// 		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, cnt_);
// 		GLuint cnt_zero_state = 0;
// 		glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), &cnt_zero_state, GL_DYNAMIC_DRAW);

// 		ppll_heads_ = RenderTargetFactory()
// 			.Size(render_layer_width_, render_layer_height_)
// 			.ColorAsTexture(GL_NEAREST, GL_NEAREST, 1, GL_R32UI)
// 			.Create();

// 		glGenBuffers(1, &ppll_hair_nodes_);
// 		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ppll_hair_nodes_);

// 		glBufferData(GL_SHADER_STORAGE_BUFFER, ppll_max_hair_nodes_ * sizeof(HairNode), nullptr, GL_DYNAMIC_DRAW);
// 		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

// 		std::vector<c3d::Vec3> quad_verts{ {-1,-1,1},{1,-1,1},{1,1,1},{-1,1,1} };
// 		glGenVertexArrays(1, &screen_quad_vao_);
// 		glGenBuffers(1, &screen_quad_buf_);
// 		glBindVertexArray(screen_quad_vao_);
// 		glBindBuffer(GL_ARRAY_BUFFER, screen_quad_buf_);
// 		glBufferData(GL_ARRAY_BUFFER, quad_verts.size() * sizeof(c3d::Vec3), quad_verts.data(), GL_STATIC_DRAW);
// 		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(c3d::Vec3), (void*)0);
// 		glBindVertexArray(0);







// 		// Resource init.


// 		// Shader init.
		
// 		// Set Model.
// 	}
// 	void Destory();
// 	void Render();

// private:
// 	c3d::iVec3 ppll_size_;
// 	c3d::iVec2 shadow_map_size_;
// 	GLuint s_predepth_, s_store_, s_resolve_, s_shadow_map_, s_detail_;
// 	RenderTarget depth4fbo_, depth4_, alphamap_, head_, shadow_depth4_, shadow_depth4fbo_;
// 	GLuint cnt_, nodes_, quat_vao_, quad_buf_;

// 	struct HairNode {
// 		GLuint depth;
// 		GLuint color;
// 		GLuint next;
// 	};

// 	void InitializeShaders(std::string shader_dir)
// 	{
// 		std::unordered_map<std::string, std::string> map{
// 			{"version", "#version 450 core"}
// 		};

// 		s_predepth_ = ResolveShader(
// 			map,
// 			shader_dir + "ppll_predepth.glsl",
// 			shader_dir);

// 		s_store_ = ResolveShader(
// 			map,
// 			shader_dir + "ppll_store.glsl",
// 			shader_dir);

// 		s_resolve_ = ResolveShader(
// 			map,
// 			shader_dir + "ppll_resolve.glsl",
// 			shader_dir);

// 		s_shadow_map_ = ResolveShader(
// 			map,
// 			shader_dir + "ppll_predepth.glsl",
// 			shader_dir);

// 		s_detail_ = ResolveShader(
// 			map,
// 			shader_dir + "hair_detail.glsl",
// 			shader_dir);
// 	}
// };

class PPLLPHairRenderer : public HairRenderer {

public:

	PPLLPHairRenderer(
		int render_layer_width,
		int render_layer_height,
		int k);

	void InitGpuResource(const HairAsset *asset) override;
	void DelGpuResource() override;
	void RenderMainPass(RenderTarget &target, const Camera &camera) override;

private:

	int render_layer_width_, render_layer_height_, shadow_depth_k_map_width_, shadow_depth_k_map_height_, k_;
	GLuint s_hair_ppll_store_, s_hair_ppll_resolve_, s_depth_k_, s_write_depth_k_, s_hair_detail_, s_shadow_depth_k_;
	RenderTarget rt_reduced_depth_, rt_hair_alpha_, rt_depth_k_, rt_shadow_depth_k_, rt_shadow_depth_k_fbo_;

	int ppll_max_hair_nodes_;
	GLuint ppll_cnt_, ppll_hair_nodes_;
	RenderTarget ppll_heads_;
	GLuint screen_quad_vao_, screen_quad_buf_;

	// Smooth shadow depth
	GLuint filter_, filter_tmp_tex_;

	struct HairNode {
		GLuint depth;
		GLuint color;
		GLuint next;
		GLuint padding;
	};
};

class SimpleHairRenderer :public HairRenderer {

public:

	SimpleHairRenderer(int render_layer_width, int render_layer_height)
		:render_layer_width_{ render_layer_width },
		render_layer_height_{ render_layer_height }{}

	void InitGpuResource(const HairAsset *asset) override;
	void DelGpuResource() override;
	void RenderMainPass(RenderTarget &target, const Camera &camera) override;
	void RenderPrePass(RenderTarget &target, const Camera &camera) override;

private:

	int render_layer_width_, render_layer_height_;
	GLuint s_main_, s_depth_reduce_;
	RenderTarget rt_reduced_depth_, rt_hair_alpha_;

};
