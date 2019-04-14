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

