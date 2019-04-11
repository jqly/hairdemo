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

	SimpleHairRenderer() {}

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
	}

	void DelGpuResource() override
	{
		glDeleteProgram(main_shader_);
		DelRenderLayer(depth_reduce_);
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

	void ReduceDepth(RenderLayer &target, const Camera &camera)
	{

		glBindFramebuffer(GL_FRAMEBUFFER, target.fbo);
		glEnable(GL_DEPTH_TEST);
		glViewport(0, 0, target.width, target.height);
		glUseProgram(depth_reduce_shader_);
		glColorMask(false, false, false, false);

		ShaderAssign(glGetUniformLocation(depth_reduce_shader_, "g_Model"), gasset.model);
		ShaderAssign(glGetUniformLocation(depth_reduce_shader_, "g_ViewProj"), camera.Proj()*camera.View());
		gasset.DrawElements({ 0 });
		
		glColorMask(true, true, true, true);
	}

private:

	GLuint main_shader_, depth_reduce_shader_;
	RenderLayer depth_reduce_;

};