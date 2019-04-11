#include <string>
#include <unordered_map>

#include "xy_ext.h"
#include "xy_opengl.h"


static std::string shader_dir = "D:\\jqlyg\\haireffects\\shader\\";

struct XYREParams {
	float msm_depth_offset;
	float msm_moments_offset;
	xy::vec3 sun_light_dir;
};

class XYRE {

public:

	XYRE(int render_target_width, int render_target_height)
	{
		render_target_width_ = render_target_width;
		render_target_height_ = render_target_height;
	}

	void MakeGpuResource()
	{
		auto shader = ResolveShader(
			std::unordered_map<std::string, std::string>{
				{"version", "#version 450 core"}
		},
			"D:\\jqlyg\\hairdemo\\shader\\hair_renderer.glsl",
			"D:\\jqlyg\\hairdemo\\shader\\");
		compo_ = MakeRenderLayer(render_target_width_, render_target_height_, 0, GL_RGBA8, GL_DEPTH_COMPONENT24);
	}

	void DelGpuResource()
	{
		DelRenderLayer(compo_);
	}

	RenderLayer Render(const std::vector<HairGAsset> &&hair_gassets, const std::vector<ObjGAsset> &&obj_gassets, const Camera &camera, const XYREParams &params)
	{
		AABB bounds{};
		for (const auto asset : hair_gassets)
			bounds.Extend(asset.bounds);
		for (const auto asset : obj_gassets)
			bounds.Extend(asset.bounds);
		LightCamera light_camera(bounds, -params.sun_light_dir, { 0,1,0 });

		glBindFramebuffer(GL_FRAMEBUFFER, compo_.fbo);
		glClearColor(1, 1, 1, 1);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		return compo_;
	}

private:
	int render_target_width_, render_target_height_;
	RenderLayer compo_;
};
