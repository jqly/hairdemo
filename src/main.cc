#include <string>
#include <cctype>

#include "xy_opengl.h"
#include "render.h"
#include "hair_renderer.h"
//#include "hair_renderer.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"


void ImguiInit(GLFWwindow *window);
void ImguiOverlay(XYREParams &params, HairGAsset &hair_gasset);
void ImguiExit();

int main()
{
	//HairRenderer hair_renderer{};

	//hair_renderer.GpuInit();

	const int W = 1024, H = 1024;
	auto wptr = MakeGlfw45Window(W, H, "c01dbeaf", true, true);
	const std::string asset_dir = "D:\\jqlyg\\hairdemo\\asset\\";

	auto hair_asset = MakeHairAsset(asset_dir + "lionking/lionking.ind");
	xy::Print("#hairs:{},#verts:{}\n", hair_asset.vcounts.size(), hair_asset.positions.size());
	auto hair_gasset = MakeHairGAsset(hair_asset);

	//auto obj_asset = MakeObjAsset(asset_dir + "lionking/lionking/lionsmooth.obj", asset_dir + "lionking/lionking/");
	//auto obj_gasset = MakeObjGAsset(obj_asset);

	AABB scene_bounds{};
	for (const auto asset : { hair_gasset })
		scene_bounds.Extend(asset.bounds);
	//for (const auto asset : { obj_gasset })
	//	scene_bounds.Extend(asset.bounds);


	xy::Print("center:{},dims:{}\n", scene_bounds.Center(), scene_bounds.Lengths());

	ArcballCamera camera(
		scene_bounds, { 0,0,-1 }, { 0,1,0 },
		xy::DegreeToRadian(60.f),
		static_cast<float>(W) / H);

	//WanderCamera camera(xy::vec3{ 100.f,0.f,0.f }, scene_bounds.Center(), { 0,1,0 },
	//	xy::DegreeToRadian(60.f),
	//	static_cast<float>(W) / H, .1f, 100.f);

	SimpleHairRenderer hair_renderer{W,H};
	hair_renderer.InitGpuResource(&hair_asset);
	XYInput input{ wptr };
	auto composition = MakeRenderLayer(W, H, 0, GL_RGBA8, GL_DEPTH_COMPONENT24);

	XYREParams xyre_params;
	xyre_params.msm_depth_offset = 0.f;
	xyre_params.msm_moments_offset = 0.f;
	xyre_params.sun_light_dir = xy::vec3{ 1.f,0.f,0.f };

	ImguiInit(wptr);
	int frame_count = 0;
	long long duration = 0;
	while (!glfwWindowShouldClose(wptr)) {
		input.PollEvents();

		if (!ImGui::GetIO().WantCaptureMouse)
			camera.HandleInput(input);

		glBindFramebuffer(GL_FRAMEBUFFER, composition.fbo);
		glClearColor(1, 1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_DEPTH_TEST);

		hair_renderer.ReduceDepth(composition, camera);
		hair_renderer.RenderMainPass(composition, camera);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(1, 1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, composition.fbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBlitFramebuffer(
			0, 0,
			composition.width, composition.height,
			0, 0,
			W, H,
			GL_COLOR_BUFFER_BIT, GL_LINEAR);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		ImguiOverlay(xyre_params, hair_gasset);

		glfwSwapBuffers(wptr);

	}

	hair_renderer.DelGpuResource();
	DelHairGAsset(hair_gasset);

	ImguiExit();

	return 0;
}

void ImguiInit(GLFWwindow *window)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui_ImplGlfw_InitForOpenGL(window, false);
	ImGui_ImplOpenGL3_Init();
	ImGui::StyleColorsDark();
}

void ImguiOverlay(
	XYREParams &params,
	HairGAsset &hair_gasset
)
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui::Begin("Tweak");
	ImGui::Text("(%.2fms,%.0ffps)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	ImGui::SliderFloat("MSM moments offset", &params.msm_moments_offset, 0.f, 1.f);
	ImGui::SliderFloat("MSM depth offset", &params.msm_depth_offset, 0.f, 1.f);
	ImGui::SliderFloat3("Sun light dir", params.sun_light_dir.data, -1.f, 1.f);

	ImGui::SliderFloat("Hair radius", &hair_gasset.radius, 0.f, 10.f);
	ImGui::SliderFloat("Hair transp", &hair_gasset.alpha, 0.f, 1.f);

	ImGui::End();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImguiExit()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}
