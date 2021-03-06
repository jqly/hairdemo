#include <string>
#include <cctype>

#include "app_settings.h"
#include "xy_opengl.h"
#include "render.h"
#include "hair_renderer.h"
#include "mesh_renderer.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"


struct XYREParams{
	c3d::Vec3 *point_light_position;
};

void ImguiInit(GLFWwindow *window);
void ImguiOverlay(XYREParams &params);
void ImguiExit();

int main(int argc, char* argv[])
{

	const int W = 1024, H = 1024;
	auto wptr = MakeGlfw45Window(W, H, "c01dbeaf", false, false);
	const std::string asset_dir = "D:\\jqlyg\\hairdemo\\asset\\";

	//auto hair_asset = MakeHairAsset(std::string(argv[1]));
	//auto hair_asset = MakeHairAsset(asset_dir + "train\\test.ind");
	//auto hair_asset = MakeHairAsset(asset_dir + "grass/grass.ind");
	auto hair_asset = MakeHairAsset(asset_dir + "lionking/lionking.ind");

	hair_asset.radius[0] = 10.f;
	hair_asset.radius[1] = 5.f;
	hair_asset.radius[2] = 1.5f;

	hair_asset.transparency[0] = .75f;
	hair_asset.transparency[1] = .2f;
	hair_asset.transparency[2] = 0.f;

	auto obj_asset = MakeObjAsset(asset_dir + "lionking/lionking/lionsmooth.obj", asset_dir + "lionking/lionking/");
	//auto obj_gasset = MakeObjGAsset(obj_asset);

	AABB scene_bounds{hair_asset.positions};
	for (int i = 0; i < obj_asset.attrib.vertices.size() / 3; ++i) {
		c3d::Vec3 position{ obj_asset.attrib.vertices[3*i+0], obj_asset.attrib.vertices[3*i+1], obj_asset.attrib.vertices[3*i+2] };
		scene_bounds.Extend(position);
	}

	//for (const auto asset : { obj_gasset })
	//	scene_bounds.Extend(asset.bounds);

	ArcballCamera camera(
		scene_bounds, { 0,0,-1 }, { 0,1,0 },
		c3d::Deg2Rad(60.f),
		static_cast<float>(W) / H);

	//WanderCamera camera{
	//	{0,0,5},{0,0,0},{0,1,0},c3d::Deg2Rad(60.f),
	//	static_cast<float>(W) / H,.1f,10.f
	//};

	SimpleMeshRenderer mesh_renderer{c3d::iVec2{W,H}};
	mesh_renderer.InitGpuResource(&obj_asset);

	PPLLPHairRenderer hair_renderer{W,H,8};
	hair_renderer.InitGpuResource(&hair_asset);
	XYInput input{ wptr };
	auto composition = RenderTargetFactory()
		.Size(W, H)
		.ColorAsTexture(GL_LINEAR, GL_NEAREST, 1, GL_RGBA8)
		.DepthAsRenderbuffer(GL_DEPTH_COMPONENT24)
		.Create();

	XYREParams xyre_params;
	app_settings::point_light_position = { 10.f, 10.f, 10.f };
	xyre_params.point_light_position = &app_settings::point_light_position;

	ImguiInit(wptr);
	int frame_count = 0;
	long long duration = 0;
	while (!glfwWindowShouldClose(wptr)) {
		input.PollEvents();

		if (!ImGui::GetIO().WantCaptureMouse)
			camera.HandleInput(input);

		glBindFramebuffer(GL_FRAMEBUFFER, composition.fbo);
		glClearColor(0,0,0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//mesh_renderer.RenderPrePass(composition, camera);
		//mesh_renderer.RenderMainPass(composition, camera);

		hair_renderer.RenderPrePass(composition, camera);
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

		ImguiOverlay(xyre_params);

		glfwSwapBuffers(wptr);

	}

	hair_renderer.DelGpuResource();

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
	XYREParams &params
)
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui::Begin("Tweak");
	ImGui::Text("(%.2fms,%.0ffps)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	ImGui::SliderFloat3("Point light dir", c3d::begin(*params.point_light_position), -10.f, 10.f);

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
