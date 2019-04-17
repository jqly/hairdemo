#include "hair_renderer.h"

static const std::string shader_dir = "D:\\jqlyg\\hairdemo\\shader\\";


PPLLPHairRenderer::PPLLPHairRenderer(int render_layer_width, int render_layer_height, int k)
	:render_layer_width_{ render_layer_width },
	render_layer_height_{ render_layer_height },
	k_{ k }
{
	ppll_max_hair_nodes_ = render_layer_width_ * render_layer_height_*k_*2;
}

void PPLLPHairRenderer::InitGpuResource(const HairAsset * asset)
{
	SetModel(asset);

	s_depth_k_ = ResolveShader(
		std::unordered_map<std::string, std::string>{
			{"version", "#version 450 core"}
	},
		shader_dir + "hair_depth_k.glsl",
		shader_dir);

	rt_depth_k_ = RenderTargetFactory()
		.Size(render_layer_width_ * 2, render_layer_height_)
		.ColorAsTexture(GL_NEAREST, GL_NEAREST, 1, GL_R32UI)
		.Create();

	rt_reduced_depth_ = RenderTargetFactory()
		.Size(render_layer_width_, render_layer_height_)
		.DepthAsRenderbuffer(GL_DEPTH_COMPONENT24)
		.Create();

	rt_hair_alpha_ = RenderTargetFactory()
		.Size(render_layer_width_, render_layer_height_)
		.ColorAsTexture(GL_NEAREST, GL_NEAREST, 1, GL_R32UI)
		.Create();

	s_hair_ppll_store_ = ResolveShader(
		std::unordered_map<std::string, std::string>{
			{"version", "#version 450 core"}
	},
		shader_dir + "hair_ppll_store.glsl",
		shader_dir);

	s_hair_ppll_resolve_ = ResolveShader(
		std::unordered_map<std::string, std::string>{
			{"version", "#version 450 core"}
	},
		shader_dir + "hair_ppll_resolve.glsl",
		shader_dir);

	s_hair_detail_ = ResolveShader(
		std::unordered_map<std::string, std::string>{
			{"version", "#version 450 core"}
	},
		shader_dir + "hair_detail.glsl",
		shader_dir);

	glGenBuffers(1, &ppll_cnt_);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, ppll_cnt_);
	GLuint ppll_cnt_zero_state = 0;
	glBufferStorage(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), &ppll_cnt_zero_state, GL_DYNAMIC_STORAGE_BIT);

	ppll_heads_ = RenderTargetFactory()
		.Size(render_layer_width_, render_layer_height_)
		.ColorAsTexture(GL_NEAREST, GL_NEAREST, 1, GL_R32UI)
		.Create();

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

void PPLLPHairRenderer::DelGpuResource()
{
	glDeleteProgram(s_hair_ppll_store_);
	glDeleteProgram(s_hair_ppll_resolve_);
	glDeleteProgram(s_hair_detail_);
	DelRenderTarget(rt_depth_k_);
	DelRenderTarget(rt_reduced_depth_);
	DelRenderTarget(rt_hair_alpha_);
	DelRenderTarget(ppll_heads_);
	glDeleteBuffers(1, &ppll_hair_nodes_);
	glDeleteVertexArrays(1, &screen_quad_vao_);
	glDeleteBuffers(1, &screen_quad_buf_);
}

void PPLLPHairRenderer::RenderMainPass(RenderTarget & target, const Camera & camera)
{

	glBindFramebuffer(GL_READ_FRAMEBUFFER, target.fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, rt_reduced_depth_.fbo);
	glBlitFramebuffer(
		0, 0, target.width, target.height,
		0, 0, rt_reduced_depth_.width, rt_reduced_depth_.height,
		GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, rt_depth_k_.fbo);
	glViewport(0, 0, rt_depth_k_.width, rt_depth_k_.height);
	glClearColor(1.f, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindFramebuffer(GL_FRAMEBUFFER, rt_hair_alpha_.fbo);
	glViewport(0, 0, rt_hair_alpha_.width, rt_hair_alpha_.height);
	glClearColor(0.f, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	////
	// Reduce depth.
	////

	glBindFramebuffer(GL_FRAMEBUFFER, rt_reduced_depth_.fbo);
	glViewport(0, 0, rt_reduced_depth_.width, rt_reduced_depth_.height);
	glUseProgram(s_depth_k_);
	glBindImageTexture(0, rt_depth_k_.color, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
	glBindImageTexture(1, rt_hair_alpha_.color, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
	ShaderAssign(glGetUniformLocation(s_depth_k_, "g_Model"), gasset.model);
	ShaderAssign(glGetUniformLocation(s_depth_k_, "g_ViewProj"), camera.Proj()*camera.View());
	ShaderAssign(glGetUniformLocation(s_depth_k_, "g_Eye"), camera.Pos());
	ShaderAssign(glGetUniformLocation(s_depth_k_, "g_WinSize"), xy::vec2(render_layer_width_, render_layer_height_));
	ShaderAssign(glGetUniformLocation(s_depth_k_, "g_HairRadius"), 3.f);
	ShaderAssign(glGetUniformLocation(s_depth_k_, "g_HairTransparency"), .2f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	gasset.DrawIndexed(0, { 0,1 });
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthFunc(GL_LESS);

	////
	// PPLL store fragments into linked list.
	////

	glBindFramebuffer(GL_FRAMEBUFFER, ppll_heads_.fbo);
	glClearColor(0.f, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, rt_reduced_depth_.fbo);
	glUseProgram(s_hair_ppll_store_);
	glViewport(0, 0, rt_reduced_depth_.width, rt_reduced_depth_.height);

	GLuint ppll_cnt_zero_state = 0;
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, ppll_cnt_);
	glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &ppll_cnt_zero_state);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ppll_hair_nodes_);
	glBindImageTexture(0, ppll_heads_.color, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);

	ShaderAssign(glGetUniformLocation(s_hair_ppll_store_, "g_Model"), gasset.model);
	ShaderAssign(glGetUniformLocation(s_hair_ppll_store_, "g_ViewProj"), camera.Proj()*camera.View());
	ShaderAssign(glGetUniformLocation(s_hair_ppll_store_, "g_Eye"), camera.Pos());
	ShaderAssign(glGetUniformLocation(s_hair_ppll_store_, "g_SpotLightPos"), xy::vec3(10.f, 10.f, 10.f));
	ShaderAssign(glGetUniformLocation(s_hair_ppll_store_, "g_ViewProj"), camera.Proj()*camera.View());
	ShaderAssign(glGetUniformLocation(s_hair_ppll_store_, "g_WinSize"), xy::vec2(rt_hair_alpha_.width, rt_hair_alpha_.height));
	ShaderAssign(glGetUniformLocation(s_hair_ppll_store_, "g_HairRadius"), 3.f);
	ShaderAssign(glGetUniformLocation(s_hair_ppll_store_, "g_HairTransparency"), .2f);
	ShaderAssign(glGetUniformLocation(s_hair_ppll_store_, "g_MaxHairNodes"), ppll_max_hair_nodes_);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_FALSE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	gasset.DrawIndexed(0, { 0,1 });
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);

	////
	// Resolve fragments in the linked list. Blend them with target.
	////

	glBindFramebuffer(GL_FRAMEBUFFER, target.fbo);
	glViewport(0, 0, target.width, target.height);

	glUseProgram(s_hair_ppll_resolve_);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ppll_hair_nodes_);
	glBindImageTexture(0, rt_hair_alpha_.color, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
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


	////
	// Finally, add thin hairs on top of the previous.
	// (TODO: When cemara near?)
	////

	glBindFramebuffer(GL_FRAMEBUFFER, target.fbo);
	glViewport(0, 0, target.width, target.height);

	glUseProgram(s_hair_detail_);
	glBindImageTexture(0, rt_depth_k_.color, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32UI);
	ShaderAssign(glGetUniformLocation(s_hair_detail_, "g_Model"), gasset.model);
	ShaderAssign(glGetUniformLocation(s_hair_detail_, "g_ViewProj"), camera.Proj()*camera.View());
	ShaderAssign(glGetUniformLocation(s_hair_detail_, "g_Eye"), camera.Pos());
	ShaderAssign(glGetUniformLocation(s_hair_detail_, "g_WinSize"), xy::vec2(render_layer_width_, render_layer_height_));
	ShaderAssign(glGetUniformLocation(s_hair_detail_, "g_HairRadius"), 0.5f);
	ShaderAssign(glGetUniformLocation(s_hair_detail_, "g_HairTransparency"), 0.f);
	ShaderAssign(glGetUniformLocation(s_hair_detail_, "g_Eye"), camera.Pos());
	ShaderAssign(glGetUniformLocation(s_hair_detail_, "g_SpotLightPos"), xy::vec3(10.f, 10.f, 10.f));

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
	glDepthFunc(GL_LESS);
	gasset.DrawIndexed(1, { 0,1 });
	glDepthMask(GL_TRUE);
}

void SimpleHairRenderer::InitGpuResource(const HairAsset * asset)
{
	SetModel(asset);

	s_main_ = ResolveShader(
		std::unordered_map<std::string, std::string>{
			{"version", "#version 450 core"}
	},
		shader_dir + "hair_renderer.glsl",
		shader_dir);

	s_depth_reduce_ = ResolveShader(
		std::unordered_map<std::string, std::string>{
			{"version", "#version 450 core"}
	},
		shader_dir + "hair_renderer_reduce_depth.glsl",
		shader_dir);

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

void SimpleHairRenderer::DelGpuResource()
{
	glDeleteProgram(s_main_);
	DelRenderTarget(rt_reduced_depth_);
	DelRenderTarget(rt_hair_alpha_);
}

void SimpleHairRenderer::RenderMainPass(RenderTarget & target, const Camera & camera)
{
	glBindFramebuffer(GL_FRAMEBUFFER, target.fbo);
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, target.width, target.height);
	glUseProgram(s_main_);
	ShaderAssign(glGetUniformLocation(s_main_, "g_Model"), gasset.model);
	ShaderAssign(glGetUniformLocation(s_main_, "g_ViewProj"), camera.Proj()*camera.View());
	gasset.DrawIndexed(0, { 0,1 });
}

void SimpleHairRenderer::RenderPrePass(RenderTarget & target, const Camera & camera)
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
	gasset.DrawIndexed(0, { 0 });

	glColorMask(true, true, true, true);
}
