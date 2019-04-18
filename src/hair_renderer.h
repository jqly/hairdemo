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
	GLuint s_hair_ppll_store_, s_hair_ppll_resolve_, s_depth_k_, s_hair_detail_, s_shadow_depth_k_;
	RenderTarget rt_reduced_depth_, rt_hair_alpha_, rt_depth_k_, rt_shadow_depth_k_, rt_shadow_depth_k_fake_;

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

	void InitGpuResource(const HairAsset *asset) override;
	void DelGpuResource() override;
	void RenderMainPass(RenderTarget &target, const Camera &camera) override;
	void RenderPrePass(RenderTarget &target, const Camera &camera) override;

private:

	int render_layer_width_, render_layer_height_;
	GLuint s_main_, s_depth_reduce_;
	RenderTarget rt_reduced_depth_, rt_hair_alpha_;

};
