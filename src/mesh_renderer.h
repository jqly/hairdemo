#include "xy_opengl.h"
#include "app_settings.h"


static const std::string shader_dir = "D:\\jqlyg\\hairdemo\\shader\\";

class MeshRenderer {

public:

	MeshRenderer()
		:asset{ nullptr } {}

	virtual void InitGpuResource(const ObjAsset *asset) = 0;
	virtual void DelGpuResource() = 0;

	virtual void SetModel(const ObjAsset *asset)
	{
		this->asset = asset;
		gasset = MakeObjGAsset(*asset);
	}

	virtual void RenderMainPass(RenderTarget &target, const Camera &camera) = 0;
	virtual void RenderShadowMap(const Camera &camera) {}
	virtual void RenderPrePass(RenderTarget &target, const Camera &camera) {}

	virtual ~MeshRenderer() {}

protected:

	const ObjAsset *asset;
	ObjGAsset gasset;

};

class SimpleMeshRenderer : public MeshRenderer {
public:
	SimpleMeshRenderer(c3d::iVec2 rtsize)
		: rtsize_{rtsize}
	{}

	void InitGpuResource(const ObjAsset *asset) override 
	{
		SetModel(asset);

		shader_ = ResolveShader(
			std::unordered_map<std::string, std::string>{
				{"version", "#version 450 core"}
			},
			shader_dir + "simple_mesh.glsl",
			shader_dir);
	}

	void DelGpuResource() override
	{
		glDeleteProgram(shader_);
		DelObjGAsset(gasset);
	}

	void RenderMainPass(RenderTarget &target, const Camera &camera) override
	{

		glBindFramebuffer(GL_FRAMEBUFFER, target.fbo);
		glViewport(0, 0, target.width, target.height);

		glEnable(GL_DEPTH_TEST);

		glUseProgram(shader_);
		ShaderAssign(glGetUniformLocation(shader_, "g_DiffuseMap"), 1);
		ShaderAssign(glGetUniformLocation(shader_, "g_ViewProj"),  c3d::Dot(camera.Proj(),camera.View()));
		ShaderAssign(glGetUniformLocation(shader_, "g_Eye"), camera.Pos());
		ShaderAssign(glGetUniformLocation(shader_, "g_SunLightDir"), app_settings::point_light_position);

		ShaderAssign(glGetUniformLocation(shader_, "g_Model"), gasset.model);
		for (int kth_part = 0; kth_part < gasset.parts.size(); ++kth_part) {

			auto mtl_idx = gasset.part2mtl[kth_part];
			const auto &mtl = gasset.mtls[mtl_idx];
			const auto &part = gasset.parts[kth_part];

			//if (asset.asset->shapes[asset.part2shape[kth_part]].name[0] == 'S')
			//	continue;
			//xy::Print("shape_{},mtl_{}\n", asset.asset->shapes[asset.part2shape[kth_part]].name, asset.asset->mtls[asset.part2mtl[kth_part]].name);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, mtl.map_Kd);
			part.DrawElements(GL_TRIANGLES, { 0,1,2 });
		}

	}

	void RenderPrePass(RenderTarget &target, const Camera &camera) override {}

private:
	GLuint shader_;
	c3d::iVec2 rtsize_;
};
