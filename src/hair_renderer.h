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
	virtual void ReduceDepth(RenderTarget &target, const Camera &camera) {}

	virtual ~HairRenderer() {}

protected:

	const HairAsset *asset;
	HairGAsset gasset;

};

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

	int render_layer_width_, render_layer_height_, k_;
	GLuint s_hair_ppll_store_, s_hair_ppll_resolve_, s_depth_k_;
	RenderTarget rt_reduced_depth_, rt_hair_alpha_, rt_depth_k_, rt_hair_;

	int ppll_max_hair_nodes_;
	GLuint ppll_cnt_, ppll_hair_nodes_;
	RenderTarget ppll_heads_;
	GLuint screen_quad_vao_, screen_quad_buf_;

	struct HairNode {
		GLuint depth, color, next;
	};
};

class SimpleHairRenderer :public HairRenderer {

public:

	SimpleHairRenderer(int render_layer_width, int render_layer_height)
		:render_layer_width_{ render_layer_width },
		render_layer_height_{ render_layer_height }{}

	void InitGpuResource(const HairAsset *asset) override
	{
		SetModel(asset);

		s_main_ = ResolveShader(
			std::unordered_map<std::string, std::string>{
				{"version", "#version 450 core"}
		},
			"D:\\jqlyg\\hairdemo\\shader\\hair_renderer.glsl",
			"D:\\jqlyg\\hairdemo\\shader\\");

		s_depth_reduce_ = ResolveShader(
			std::unordered_map<std::string, std::string>{
				{"version", "#version 450 core"}
		},
			"D:\\jqlyg\\hairdemo\\shader\\hair_renderer_reduce_depth.glsl",
			"D:\\jqlyg\\hairdemo\\shader\\");

		rt_reduced_depth_ = RenderTargetFactory()
			.Size(render_layer_width_, render_layer_height_)
			.ColorAsTexture(GL_NEAREST, GL_NEAREST, 1, GL_R32F)
			.DepthAsRenderbuffer(GL_DEPTH_COMPONENT24)
			.Create();

		rt_hair_alpha_ = RenderTargetFactory()
			.Size(render_layer_width_ * 3, render_layer_height_)
			.ColorAsTexture(GL_NEAREST, GL_NEAREST, 1, GL_R32UI)
			.Create();
	}

	void DelGpuResource() override
	{
		glDeleteProgram(s_main_);
		DelRenderTarget(rt_reduced_depth_);
		DelRenderTarget(rt_hair_alpha_);
	}

	void RenderMainPass(RenderTarget &target, const Camera &camera) override
	{
		glBindFramebuffer(GL_FRAMEBUFFER, target.fbo);
		glEnable(GL_DEPTH_TEST);
		glViewport(0, 0, target.width, target.height);
		glUseProgram(s_main_);
		ShaderAssign(glGetUniformLocation(s_main_, "g_Model"), gasset.model);
		ShaderAssign(glGetUniformLocation(s_main_, "g_ViewProj"), camera.Proj()*camera.View());
		gasset.DrawElements({ 0,1 });
	}

	void ReduceDepth(RenderTarget &target, const Camera &camera) override
	{

		glBindFramebuffer(GL_FRAMEBUFFER, rt_hair_alpha_.fbo);
		glViewport(0, 0, rt_hair_alpha_.width, rt_hair_alpha_.height);
		glClearColor(1.f, 1.f, 1.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);

		glBindFramebuffer(GL_FRAMEBUFFER, target.fbo);
		glEnable(GL_DEPTH_TEST);
		glViewport(0, 0, target.width, target.height);
		glUseProgram(s_depth_reduce_);

		glBindImageTexture(0, rt_hair_alpha_.color, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);

		glColorMask(false, false, false, false);

		ShaderAssign(glGetUniformLocation(s_depth_reduce_, "g_Model"), gasset.model);
		ShaderAssign(glGetUniformLocation(s_depth_reduce_, "g_ViewProj"), camera.Proj()*camera.View());
		gasset.DrawElements({ 0 });

		glColorMask(true, true, true, true);
	}

private:

	int render_layer_width_, render_layer_height_;
	GLuint s_main_, s_depth_reduce_;
	RenderTarget rt_reduced_depth_, rt_hair_alpha_;

};
