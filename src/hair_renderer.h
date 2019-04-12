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

	virtual void RenderMainPass(RenderLayer &target, const Camera &camera) = 0;
	virtual void RenderShadowMap(const Camera &camera) {}
	virtual void ReduceDepth(RenderLayer &target, const Camera &camera) {}

	virtual ~HairRenderer() {}

protected:

	const HairAsset *asset;
	HairGAsset gasset;

};

class TressFXHairRenderer : public HairRenderer {

};

class SimpleHairRenderer :public HairRenderer {

public:

	SimpleHairRenderer(int render_layer_width, int render_layer_height)
		:render_layer_width_{ render_layer_width },
		render_layer_height_{ render_layer_height }{}

	void InitGpuResource(const HairAsset *asset) override
	{
		SetModel(asset);

		main_shader_ = ResolveShader(
			std::unordered_map<std::string, std::string>{
				{"version", "#version 450 core"}
		},
			"D:\\jqlyg\\hairdemo\\shader\\hair_renderer.glsl",
			"D:\\jqlyg\\hairdemo\\shader\\");

		depth_reduce_shader_ = ResolveShader(
			std::unordered_map<std::string, std::string>{
				{"version", "#version 450 core"}
		},
			"D:\\jqlyg\\hairdemo\\shader\\hair_renderer_reduce_depth.glsl",
			"D:\\jqlyg\\hairdemo\\shader\\");

		depth_reduce_ = MakeRenderLayer(1024, 1024, 0, GL_R32F, GL_DEPTH_COMPONENT24);

		depth_k_ = RenderLayer{};
		depth_k_.width = render_layer_width_*3;
		depth_k_.height = render_layer_height_;
		glGenTextures(1, &depth_k_.color);
		glBindTexture(GL_TEXTURE_2D, depth_k_.color);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, depth_k_.width, depth_k_.height);
		glGenFramebuffers(1, &depth_k_.fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, depth_k_.fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, depth_k_.color, 0);
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
			XY_Die("Framebuffer not complete");

	}

	void DelGpuResource() override
	{
		glDeleteProgram(main_shader_);
		DelRenderLayer(depth_reduce_);
		DelRenderLayer(depth_k_);
	}

	void RenderMainPass(RenderLayer &target, const Camera &camera) override
	{
		glBindFramebuffer(GL_FRAMEBUFFER, target.fbo);
		glEnable(GL_DEPTH_TEST);
		glViewport(0, 0, target.width, target.height);
		glUseProgram(main_shader_);
		ShaderAssign(glGetUniformLocation(main_shader_, "g_Model"), gasset.model);
		ShaderAssign(glGetUniformLocation(main_shader_, "g_ViewProj"), camera.Proj()*camera.View());
		gasset.DrawElements({ 0,1 });
	}

	void ReduceDepth(RenderLayer &target, const Camera &camera) override
	{

		glBindFramebuffer(GL_FRAMEBUFFER, depth_k_.fbo);
		glViewport(0, 0, depth_k_.width, depth_k_.height);
		glClearColor(1.f, 1.f, 1.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);
		
		glBindFramebuffer(GL_FRAMEBUFFER, target.fbo);
		glEnable(GL_DEPTH_TEST);
		glViewport(0, 0, target.width, target.height);
		glUseProgram(depth_reduce_shader_);

		glBindImageTexture(0, depth_k_.color, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);

		glColorMask(false, false, false, false);

		ShaderAssign(glGetUniformLocation(depth_reduce_shader_, "g_Model"), gasset.model);
		ShaderAssign(glGetUniformLocation(depth_reduce_shader_, "g_ViewProj"), camera.Proj()*camera.View());
		gasset.DrawElements({ 0 });
		
		glColorMask(true, true, true, true);
	}

private:

	int render_layer_width_, render_layer_height_;
	GLuint main_shader_, depth_reduce_shader_;
	RenderLayer depth_reduce_, depth_k_;

};
