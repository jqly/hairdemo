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
		int k)
		:render_layer_width_{ render_layer_width },
		render_layer_height_{ render_layer_height },
		k_{ k }
	{
		ppll_max_hair_nodes_ = render_layer_width_ * render_layer_height_*k_;
	}

	void InitGpuResource(const HairAsset *asset) override
	{
		SetModel(asset);

		s_depth_reduce_k_ = ResolveShader(
			std::unordered_map<std::string, std::string>{
				{"version", "#version 450 core"}
		},
			"D:\\jqlyg\\hairdemo\\shader\\hair_depth_k.glsl",
			"D:\\jqlyg\\hairdemo\\shader\\");

		rt_depth_reduce_k_ = RenderTargetFactory()
			.Size(render_layer_width_ * 3, render_layer_height_)
			.ColorAsTexture(GL_NEAREST, GL_NEAREST, 1, GL_R32UI)
			.Create();

		rt_transparency_ = RenderTargetFactory()
			.Size(render_layer_width_, render_layer_height_)
			.ColorAsTexture(GL_NEAREST, GL_NEAREST, 1, GL_R8)
			.DepthAsRenderbuffer(GL_DEPTH_COMPONENT24)
			.Create();

		rt_hair_ = RenderTargetFactory()
			.Size(render_layer_width_, render_layer_height_)
			.ColorAsTexture(GL_NEAREST, GL_NEAREST, 1, GL_RGBA8)
			.DepthAsRenderbuffer(GL_DEPTH_COMPONENT24)
			.Create();

		s_hair_ppll_store_ = ResolveShader(
			std::unordered_map<std::string, std::string>{
				{"version", "#version 450 core"}
		},
			"D:\\jqlyg\\hairdemo\\shader\\hair_ppll_store.glsl",
			"D:\\jqlyg\\hairdemo\\shader\\");

		s_hair_ppll_resolve_ = ResolveShader(
			std::unordered_map<std::string, std::string>{
				{"version", "#version 450 core"}
		},
			"D:\\jqlyg\\hairdemo\\shader\\hair_ppll_resolve.glsl",
			"D:\\jqlyg\\hairdemo\\shader\\");

		glGenBuffers(1, &ppll_cnt_);
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, ppll_cnt_);
		GLuint ppll_cnt_zero_state = 0;
		glBufferStorage(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), &ppll_cnt_zero_state, GL_DYNAMIC_STORAGE_BIT);

		ppll_heads_ = RenderTargetFactory()
			.Size(render_layer_width_, render_layer_height_)
			.ColorAsTexture(GL_NEAREST, GL_NEAREST, 1, GL_R32UI)
			.Create();

		int ppll_max_hair_nodes_ = render_layer_width_ * render_layer_height_*k_;
		glGenBuffers(1, &ppll_hair_nodes_);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ppll_hair_nodes_);
		glBufferStorage(GL_SHADER_STORAGE_BUFFER, ppll_max_hair_nodes_ * sizeof(HairNode), nullptr, GL_NONE);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		std::vector<xy::vec3> quad_verts{ {-1,-1,0},{1,-1,0},{1,1,0},{-1,1,0} };
		glGenVertexArrays(1, &screen_quad_vao_);
		glGenBuffers(1, &screen_quad_buf_);
		glBindVertexArray(screen_quad_vao_);
		glBindBuffer(GL_ARRAY_BUFFER, screen_quad_buf_);
		glBufferData(GL_ARRAY_BUFFER, quad_verts.size() * sizeof(xy::vec3), quad_verts.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(xy::vec3), (void*)0);
		glBindVertexArray(0);
	}

	void DelGpuResource() override
	{
		glDeleteProgram(s_hair_ppll_store_);
		glDeleteProgram(s_hair_ppll_resolve_);
		DelRenderTarget(rt_depth_reduce_k_);
		DelRenderTarget(rt_transparency_);
		DelRenderTarget(ppll_heads_);
		glDeleteBuffers(1, &ppll_hair_nodes_);
		glDeleteVertexArrays(1, &screen_quad_vao_);
		glDeleteBuffers(1, &screen_quad_buf_);
	}

	void RenderMainPass(RenderTarget &target, const Camera &camera) override
	{

		glBindFramebuffer(GL_READ_FRAMEBUFFER, target.fbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, rt_transparency_.fbo);
		glBlitFramebuffer(
			0, 0,
			target.width, target.height,
			0, 0,
			rt_transparency_.width, rt_transparency_.height,
			GL_DEPTH_BUFFER_BIT, GL_NEAREST);

		glBindFramebuffer(GL_FRAMEBUFFER, rt_depth_reduce_k_.fbo);
		glViewport(0, 0, rt_depth_reduce_k_.width, rt_depth_reduce_k_.height);
		glClearColor(1.f, 1.f, 1.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);

		glBindFramebuffer(GL_FRAMEBUFFER, rt_transparency_.fbo);
		glViewport(0, 0, rt_transparency_.width, rt_transparency_.height);
		glClearColor(1.f, 1.f, 1.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_DST_COLOR, GL_ZERO);

		glUseProgram(s_depth_reduce_k_);
		glBindImageTexture(0, rt_depth_reduce_k_.color, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
		ShaderAssign(glGetUniformLocation(s_depth_reduce_k_, "g_Model"), gasset.model);
		ShaderAssign(glGetUniformLocation(s_depth_reduce_k_, "g_ViewProj"), camera.Proj()*camera.View());

		ShaderAssign(glGetUniformLocation(s_depth_reduce_k_, "g_Eye"), camera.Pos());
		ShaderAssign(glGetUniformLocation(s_depth_reduce_k_, "g_ViewProj"), camera.Proj()*camera.View());
		ShaderAssign(glGetUniformLocation(s_depth_reduce_k_, "g_WinSize"), xy::vec2(rt_transparency_.width, rt_transparency_.height));
		ShaderAssign(glGetUniformLocation(s_depth_reduce_k_, "g_HairRadius"), 3.f);
		ShaderAssign(glGetUniformLocation(s_depth_reduce_k_, "g_HairTransparency"), 0.f);

		glDepthFunc(GL_LESS);
		glDepthMask(GL_TRUE);
		gasset.DrawElements({ 0,1 });

		glBindFramebuffer(GL_FRAMEBUFFER, ppll_heads_.fbo);
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);

		glBindFramebuffer(GL_FRAMEBUFFER, rt_transparency_.fbo);
		glUseProgram(s_hair_ppll_store_);
		glViewport(0, 0, rt_transparency_.width, rt_transparency_.height);

		GLuint ppll_cnt_zero_state = 0;
		glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, ppll_cnt_);
		glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &ppll_cnt_zero_state);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ppll_hair_nodes_);
		glBindImageTexture(0, ppll_heads_.color, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);

		ShaderAssign(glGetUniformLocation(s_hair_ppll_store_, "g_Model"), gasset.model);
		ShaderAssign(glGetUniformLocation(s_hair_ppll_store_, "g_ViewProj"), camera.Proj()*camera.View());
		ShaderAssign(glGetUniformLocation(s_hair_ppll_store_, "g_Eye"), camera.Pos());
		ShaderAssign(glGetUniformLocation(s_hair_ppll_store_, "g_SpotLightPos"), xy::vec3(10.f,10.f,10.f));
		ShaderAssign(glGetUniformLocation(s_hair_ppll_store_, "g_ViewProj"), camera.Proj()*camera.View());
		ShaderAssign(glGetUniformLocation(s_hair_ppll_store_, "g_WinSize"), xy::vec2(rt_transparency_.width, rt_transparency_.height));
		ShaderAssign(glGetUniformLocation(s_hair_ppll_store_, "g_HairRadius"), 1.f);
		ShaderAssign(glGetUniformLocation(s_hair_ppll_store_, "g_HairTransparency"), .2f);
		ShaderAssign(glGetUniformLocation(s_hair_ppll_store_, "g_MaxHairNodes"), ppll_max_hair_nodes_);

		glDepthFunc(GL_LESS);
		glDepthMask(GL_FALSE);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		gasset.DrawElements({ 0,1 });
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);


		glBindFramebuffer(GL_FRAMEBUFFER, target.fbo);
		glViewport(0, 0, target.width, target.height);

		glUseProgram(s_hair_ppll_resolve_);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ppll_hair_nodes_);
		glBindImageTexture(0, rt_transparency_.color, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R8);
		glBindImageTexture(1, ppll_heads_.color, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);

		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
		glBindVertexArray(screen_quad_vao_);
		glEnableVertexAttribArray(0);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		glDisableVertexAttribArray(0);
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
	}

private:

	int render_layer_width_, render_layer_height_, k_;
	GLuint s_hair_ppll_store_, s_hair_ppll_resolve_, s_depth_reduce_k_;
	RenderTarget rt_depth_reduce_, rt_transparency_, rt_depth_reduce_k_, rt_hair_;

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

		rt_depth_reduce_ = RenderTargetFactory()
			.Size(render_layer_width_, render_layer_height_)
			.ColorAsTexture(GL_NEAREST, GL_NEAREST, 1, GL_R32F)
			.DepthAsRenderbuffer(GL_DEPTH_COMPONENT24)
			.Create();

		rt_transparency_ = RenderTargetFactory()
			.Size(render_layer_width_ * 3, render_layer_height_)
			.ColorAsTexture(GL_NEAREST, GL_NEAREST, 1, GL_R32UI)
			.Create();
	}

	void DelGpuResource() override
	{
		glDeleteProgram(s_main_);
		DelRenderTarget(rt_depth_reduce_);
		DelRenderTarget(rt_transparency_);
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

		glBindFramebuffer(GL_FRAMEBUFFER, rt_transparency_.fbo);
		glViewport(0, 0, rt_transparency_.width, rt_transparency_.height);
		glClearColor(1.f, 1.f, 1.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);

		glBindFramebuffer(GL_FRAMEBUFFER, target.fbo);
		glEnable(GL_DEPTH_TEST);
		glViewport(0, 0, target.width, target.height);
		glUseProgram(s_depth_reduce_);

		glBindImageTexture(0, rt_transparency_.color, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);

		glColorMask(false, false, false, false);

		ShaderAssign(glGetUniformLocation(s_depth_reduce_, "g_Model"), gasset.model);
		ShaderAssign(glGetUniformLocation(s_depth_reduce_, "g_ViewProj"), camera.Proj()*camera.View());
		gasset.DrawElements({ 0 });

		glColorMask(true, true, true, true);
	}

private:

	int render_layer_width_, render_layer_height_;
	GLuint s_main_, s_depth_reduce_;
	RenderTarget rt_depth_reduce_, rt_transparency_;

};
