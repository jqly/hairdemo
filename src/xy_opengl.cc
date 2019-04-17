#include "./xy_opengl.h"

#include <thread>
#include <string>
#include <sstream>
#include <cctype>
#include <bitset>
#include <unordered_map>

#include "glad/glad.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "./xy_calc.h"
#include "./xy_ext.h"
#include "./tiny_obj_loader.h"


RenderTargetFactory& RenderTargetFactory::Size(int width, int height)
{
	width_ = width;
	height_ = height;
	return *this;
}

RenderTargetFactory& RenderTargetFactory::ColorAsTexture(GLint mag_filter, GLint min_filter, GLsizei mipmap_levels, GLenum color_format)
{
	mag_filter_ = mag_filter;
	min_filter_ = min_filter;
	mipmap_levels_ = mipmap_levels;
	color_format_ = color_format;
	color_as_texture_ = true;
	color_attachment_ = true;
	return *this;
}

RenderTargetFactory& RenderTargetFactory::DepthAsRenderbuffer(GLenum depth_format)
{
	depth_format_ = depth_format;
	depth_attachment_ = true;
	return *this;
}

RenderTargetFactory& RenderTargetFactory::ColorDepthAsRenderbuffer(GLenum color_format, GLenum depth_format)
{
	return ColorDepthAsRenderbufferMSAA(color_format, depth_format, 1);
}

RenderTargetFactory& RenderTargetFactory::ColorDepthAsRenderbufferMSAA(GLenum color_format, GLenum depth_format, int msaa)
{
	color_format_ = color_format;
	depth_format_ = depth_format;
	depth_attachment_ = true;
	msaa_ = std::max(1, msaa);
	depth_attachment_ = true;
	color_attachment_ = true;
	return *this;
}

RenderTarget RenderTargetFactory::Create()
{
	RenderTarget target{};

	target.width = width_;
	target.height = height_;
	target.msaa = msaa_;
	target.color_as_texture = color_as_texture_;
	target.depth_attachment = depth_attachment_;
	target.mipmap_levels = mipmap_levels_;

	glGenFramebuffers(1, &target.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, target.fbo);

	if (color_attachment_) {
		if (color_as_texture_) {
			glGenTextures(1, &target.color);
			glBindTexture(GL_TEXTURE_2D, target.color);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter_);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter_);
			glTexStorage2D(GL_TEXTURE_2D, mipmap_levels_, color_format_, width_, height_);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target.color, 0);
		}
		else {
			glGenRenderbuffers(1, &target.color);
			glBindRenderbuffer(GL_RENDERBUFFER, target.color);
			if (msaa_ > 1)
				glRenderbufferStorageMultisample(GL_RENDERBUFFER, msaa_, color_format_, width_, height_);
			else
				glRenderbufferStorage(GL_RENDERBUFFER, color_format_, width_, height_);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, target.color);
		}
	}

	if (depth_attachment_) {
		glGenRenderbuffers(1, &target.depth);
		glBindRenderbuffer(GL_RENDERBUFFER, target.depth);
		if (msaa_ > 1)
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, msaa_, depth_format_, width_, height_);
		else
			glRenderbufferStorage(GL_RENDERBUFFER, depth_format_, width_, height_);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, target.depth);
	}

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
		XY_Die("Framebuffer not complete");

	return target;
}


void DelRenderTarget(RenderTarget &target)
{
	glDeleteFramebuffers(1, &target.fbo);
	if (target.depth_attachment)
		glDeleteRenderbuffers(1, &target.depth);
	if (target.color_as_texture)
		glDeleteTextures(1, &target.color);
	else
		glDeleteRenderbuffers(1, &target.color);
	// POD is memset-able.
	memset(&target, 0, sizeof(target));
}

void PrettyPrintShaderLog(const std::string &log, const std::string &shader)
{
	std::vector<int> line_begin_locations;

	for (int i = 0; i < shader.size(); ++i) {
		auto ch = shader[i];

		if (i == 0)
			line_begin_locations.push_back(0);
		else if (ch == '\n'&&i + 1 < shader.size())
			line_begin_locations.push_back(i + 1);
		else
			;
	}

	int state = 0, log_line_begin = 0;
	std::string which_line;

	for (int idx = 0; idx < log.size(); ++idx) {

		char next_ch = log[idx];

		switch (state) {


		case 0:
			if (std::isdigit(next_ch)) {
				log_line_begin = idx;
				state = 1;
			}
			break;

		case 1:
			if (next_ch == '(')
				state = 2;
			else if (!std::isdigit(next_ch))
				state = 0;
			break;

		case 2:
			if (std::isdigit(next_ch)) {
				which_line.clear();
				which_line += next_ch;
				state = 3;
			}

			else
				state = 0;
			break;

		case 3:

			if (next_ch == ')' && idx + 2 < log.size() && log[idx + 1] == ' ' && log[idx + 2] == ':')
				state = 4;
			else if (std::isdigit(next_ch))
				which_line += next_ch;
			else
				state = 0;
			break;

		case 4:
		{
			int lino = std::stoi(which_line);
			int off = line_begin_locations[std::max(0, lino - 2)];
			int total_lines = line_begin_locations.size();
			int count = 0;
			if (total_lines <= lino + 1)
				count = shader.size() - off;
			else
				count = line_begin_locations[lino + 1] - off;
			xy::Print("----------\n");
			while (count > 0 && shader[off + count - 1] == '\n')
				count--;
			xy::Print("{}\n", shader.substr(off, count));

			int skip = 0;
			while (skip + idx < log.size() && log[idx + skip] != '\n')
				++skip;
			xy::Print("{}\n", log.substr(log_line_begin, idx - log_line_begin + skip));
			xy::Print("----------\n");
			state = 0;
		}
		break;

		default:
			break;
		}
	}

	return;
}

GLuint CreateShader(GLenum type, std::string code)
{
	GLuint shader_handle = glCreateShader(type);
	auto code_ = code.c_str();
	glShaderSource(shader_handle, 1, &code_, nullptr);
	glCompileShader(shader_handle);

	int success = 0, log_len = 0;
	glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &success);

	if (!success) {
		std::string shader_type_name{ "Unknown shader" };
		switch (type) {
		case GL_VERTEX_SHADER:
			shader_type_name = "Vertex shader";
			break;
		case GL_FRAGMENT_SHADER:
			shader_type_name = "Fragment shader";
			break;
		case GL_GEOMETRY_SHADER:
			shader_type_name = "Geometry shader";
			break;
		case GL_COMPUTE_SHADER:
			shader_type_name = "Compute shader";
			break;
		default:
			XY_Die("Unknown shader type");
		}

		glGetShaderiv(shader_handle, GL_INFO_LOG_LENGTH, &log_len);
		if (log_len <= 0)
			xy::Print("{}: no log found", shader_type_name);
		else {
			auto log = std::unique_ptr<char>(new char[log_len]);
			glGetShaderInfoLog(shader_handle, log_len, &log_len, log.get());
			xy::Print("======={} compile log=======\n", shader_type_name);
			PrettyPrintShaderLog(std::string(log.get(), log_len), code);
			//xy::Print("{}", log.get());
			xy::Print("-------\n");
		}
		XY_Die(shader_type_name + " compile error");
	}
	return shader_handle;
}

void CheckShader(GLuint shader)
{
	int success = 0, log_len = 0;
	glGetProgramiv(shader, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &log_len);
		if (log_len <= 0)
			xy::Print("No log found");
		else {
			auto log = std::unique_ptr<char>(new char[log_len]);
			glGetProgramInfoLog(shader, log_len, nullptr, log.get());
			xy::Print("Program compile log: {}", log.get());
		}
		xy::Print("Program compile error");
	}
}


GLuint MakeShader(std::string comp_shader)
{
	GLuint compobj = CreateShader(GL_COMPUTE_SHADER, comp_shader);

	GLuint handle = glCreateProgram();
	glAttachShader(handle, compobj);
	glLinkProgram(handle);

	CheckShader(handle);

	glDeleteShader(compobj);

	return handle;
}

GLuint MakeShader(std::string vert_shader, std::string frag_shader)
{
	auto vertobj = CreateShader(GL_VERTEX_SHADER, vert_shader);
	auto fragobj = CreateShader(GL_FRAGMENT_SHADER, frag_shader);

	GLuint handle = glCreateProgram();
	glAttachShader(handle, vertobj);
	glAttachShader(handle, fragobj);
	glLinkProgram(handle);

	CheckShader(handle);

	glDeleteShader(vertobj);
	glDeleteShader(fragobj);

	return handle;
}

GLuint MakeShader(std::string vert_shader, std::string geom_shader, std::string frag_shader)
{
	auto vertobj = CreateShader(GL_VERTEX_SHADER, vert_shader);
	auto geomobj = CreateShader(GL_GEOMETRY_SHADER, geom_shader);
	auto fragobj = CreateShader(GL_FRAGMENT_SHADER, frag_shader);

	GLuint handle = glCreateProgram();
	glAttachShader(handle, vertobj);
	glAttachShader(handle, geomobj);
	glAttachShader(handle, fragobj);
	glLinkProgram(handle);

	CheckShader(handle);

	glDeleteShader(vertobj);
	glDeleteShader(geomobj);
	glDeleteShader(fragobj);

	return handle;
}

GLuint ResolveShader(
	const std::unordered_map<std::string, std::string> &symbols,
	std::string shader_file_path, std::string shader_include_dir)
{
	std::string vertex_shader, geometry_shader, fragment_shader, compute_shader;

	auto glsl = std::istringstream(xy::ReadFile(shader_file_path));
	std::string line;

	std::string *stage = nullptr;

	while (std::getline(glsl, line)) {
		if (line.find("#include") != std::string::npos && stage != nullptr) {
			int i = 0, j = 0;
			while (i < line.size() && line[i++] != '\"')
				;
			j = i;
			while (++j < line.size() && line[j] != '\"')
				;

			auto symbol = line.substr(i, j - i);
			if (symbols.count(symbol))
				*stage += symbols.at(symbol);
			else
				*stage += xy::ReadFile(shader_include_dir + line.substr(i, j - i));
			*stage += "\n";
		}
		else if (line.find("#stage vertex") != std::string::npos) {
			stage = &vertex_shader;
		}
		else if (line.find("#stage geometry") != std::string::npos) {
			stage = &geometry_shader;
		}
		else if (line.find("#stage fragment") != std::string::npos) {
			stage = &fragment_shader;
		}
		else if (line.find("#stage compute") != std::string::npos) {
			stage = &compute_shader;
		}
		else if (line.find("#endstage") != std::string::npos) {
			stage = nullptr;
		}
		else {
			if (stage != nullptr)
				*stage += line + "\n";
		}

	}

	int status = 0;
	if (!vertex_shader.empty())
		status |= 1;
	if (!geometry_shader.empty())
		status |= 2;
	if (!fragment_shader.empty())
		status |= 4;
	if (!compute_shader.empty())
		status |= 8;

	GLuint shader = 0;
	if (status == 5)
		shader = MakeShader(vertex_shader, fragment_shader);
	else if (status == 7)
		shader = MakeShader(vertex_shader, geometry_shader, fragment_shader);
	else if (status == 8)
		shader = MakeShader(compute_shader);
	else
		;

	return shader;
}


AABB::AABB()
	:
	inf{ std::numeric_limits<float>::max() },
	sup{ std::numeric_limits<float>::lowest() }
{}

AABB::AABB(const std::vector<xy::vec3>& ps)
	: AABB::AABB()
{
	for (const auto &p : ps)
		Extend(p);
}

void AABB::Extend(const AABB & aabb)
{
	Extend(aabb.inf);
	Extend(aabb.sup);
}

void AABB::Extend(const xy::vec3 & p)
{
	inf = CompMin(inf, p);
	sup = CompMax(sup, p);
}

void AABB::Extend(const std::vector<xy::vec3>& ps)
{
	for (auto &p : ps)
		AABB::Extend(p);
}

bool AABB::IsInside(const xy::vec3 & p) const
{
	return p.x <= sup.x && p.y <= sup.y && p.z <= sup.z &&
		p.x >= inf.x && p.y >= inf.y && p.z >= inf.z;
}

xy::vec3 AABB::Center() const
{
	return (inf + sup) / 2.f;
}

xy::vec3 AABB::Lengths() const
{
	return sup - inf;
}



HairAsset MakeHairAsset(std::string path)
{
	std::ifstream fp(path, std::ios::binary);

	char header[9];
	fp.read(header, 8);
	header[8] = '\0';
	if (strcmp(header, "IND_HAIR") != 0)
		XY_Die("Hair file's header not match!\n");

	HairAsset asset{};
	asset.path = path;

	auto read_unsigned = [&fp]()->unsigned {
		unsigned n;
		fp.read(reinterpret_cast<char*>(&n), sizeof(n));
		return n;
	};

	auto read_float = [&fp]()->float {
		float n;
		fp.read(reinterpret_cast<char*>(&n), sizeof(n));
		return n;
	};

	unsigned num_fibers = read_unsigned();
	unsigned num_total_verts = read_unsigned();

	for (unsigned kthfib = 1; kthfib <= num_fibers; ++kthfib) {
		auto vert_count = read_unsigned();
		if (vert_count < 3) {
			while (vert_count--) {
				read_float();
				read_float();
				read_float();
			}
			continue;
		}
		asset.vcounts.push_back(vert_count);

		for (unsigned kthp = 0; kthp < vert_count; ++kthp) {
			float x = read_float();
			float y = read_float();
			float z = read_float();
			asset.positions.emplace_back(x, y, z);
		}
	}

	return asset;
}

HairGAsset MakeHairGAsset(const HairAsset & asset)
{
	HairGAsset gasset;
	gasset.asset = &asset;

	std::vector<xy::vec4> tangents(asset.positions.size());

	auto num_hairs = asset.vcounts.size();
	std::size_t kthvert = 0;

	for (int kthfib = 0; kthfib < num_hairs; ++kthfib) {
		auto vcount = asset.vcounts[kthfib];
		for (std::size_t i = 0; i < vcount; ++i) {
			if (i == vcount - 1)
				tangents[kthvert].v = xy::Normalize(asset.positions[kthvert] - asset.positions[kthvert - 1]);
			else
				tangents[kthvert].v = xy::Normalize(asset.positions[kthvert + 1] - asset.positions[kthvert]);

			float scale = xy::Lerp(.9f, .1f, static_cast<float>(i) / (vcount - 1));
			tangents[kthvert].w = scale;

			++kthvert;
		}

	}

	glGenVertexArrays(1, &gasset.vao);
	glGenBuffers(2, gasset.bufs);

	glBindVertexArray(gasset.vao);

	glBindBuffer(GL_ARRAY_BUFFER, gasset.bufs[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(xy::vec3)*asset.positions.size(), asset.positions.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(xy::vec3), (void*)(0));

	glBindBuffer(GL_ARRAY_BUFFER, gasset.bufs[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(xy::vec4)*tangents.size(), tangents.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(xy::vec4), (void*)(0));

	std::vector<GLuint> idx_set0, idx_set1;
	{
		xy::RandomEngine prng{ 0xc01dbeefdeadbead };
		int kth_vert = 0;
		for (int kth_hair = 0; kth_hair < asset.vcounts.size(); ++kth_hair) {
			int vcount = asset.vcounts[kth_hair];
			auto fdice = xy::Unif(prng);
			if (fdice < .75)
				;
			else if (fdice < .90) {
				for (int vi = 0; vi < vcount; ++vi)
					idx_set0.push_back(kth_vert++);
				idx_set0.push_back(0xFFFFFFFF);
			}
			else {
				for (int vi = 0; vi < vcount; ++vi)
					idx_set1.push_back(kth_vert++);
				idx_set1.push_back(0xFFFFFFFF);
			}
		}
	}

	gasset.num_indices[0] = idx_set0.size();
	gasset.num_indices[1] = idx_set1.size();

	glGenBuffers(2, gasset.index_bufs);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gasset.index_bufs[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * idx_set0.size(), idx_set0.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gasset.index_bufs[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * idx_set1.size(), idx_set1.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);

	gasset.model = xy::mat4{ 1.f };
	gasset.bounds = AABB(asset.positions);

	return gasset;
}

void DelHairGAsset(HairGAsset & gasset)
{
	glDeleteVertexArrays(1, &gasset.vao);
	glDeleteBuffers(2, gasset.bufs);
	memset(&gasset, 0, sizeof(gasset));
}

ObjAsset MakeObjAsset(std::string obj_path, std::string mtl_dirpath)
{
	ObjAsset asset;
	asset.obj_path = obj_path;
	asset.mtl_dirpath = mtl_dirpath;
	std::string errmsg, warnmsg;
	bool is_success = tinyobj::LoadObj(&asset.attrib, &asset.shapes, &asset.mtls, &warnmsg, &errmsg, obj_path.c_str(), mtl_dirpath.c_str(), true);
	if (!is_success)
		XY_Die(errmsg);

	if (asset.attrib.vertices.empty())
		XY_Die("obj empty vertex list");
	if (asset.attrib.normals.empty())
		XY_Die("obj empty normal list");
	if (asset.attrib.texcoords.empty())
		XY_Die("obj empty texcoord list");

	return asset;
}

AABB MakeObjBounds(const ObjAsset & asset)
{
	AABB bounds{};
	for (int kth_vert = 0; kth_vert < asset.attrib.vertices.size() / 3; ++kth_vert) {
		auto x = asset.attrib.vertices[kth_vert * 3 + 0];
		auto y = asset.attrib.vertices[kth_vert * 3 + 1];
		auto z = asset.attrib.vertices[kth_vert * 3 + 2];
		bounds.Extend(xy::vec3{ x,y,z });
	}
	return bounds;
}

std::bitset<256> Vertex2Bitsets(xy::vec3 v, xy::vec3 n, xy::vec2 t)
{
	std::bitset<256> bits{};

	bits |= reinterpret_cast<unsigned &>(v.x); bits <<= 32;
	bits |= reinterpret_cast<unsigned &>(v.y); bits <<= 32;
	bits |= reinterpret_cast<unsigned &>(v.z); bits <<= 32;
	bits |= reinterpret_cast<unsigned &>(n.x); bits <<= 32;
	bits |= reinterpret_cast<unsigned &>(n.y); bits <<= 32;
	bits |= reinterpret_cast<unsigned &>(n.z); bits <<= 32;
	bits |= reinterpret_cast<unsigned &>(t.x); bits <<= 32;
	bits |= reinterpret_cast<unsigned &>(t.y);

	return bits;
}

ObjGAsset_Part MakeObjGAsset_Part(
	const std::vector<xy::vec3> &positions,
	const std::vector<xy::vec3> &normals,
	const std::vector<xy::vec2> &texcoords,
	bool compute_tangents)
{
	////
	// Reduce duplicates by indexing the vertices.
	////

	auto num_verts = positions.size();

	std::vector<xy::vec3> opt_positions;
	std::vector<xy::vec3> opt_normals;
	std::vector<xy::vec2> opt_texcoords;
	std::vector<unsigned int> indices;

	int unique_vertex_counter = 0;
	std::unordered_map<std::bitset<256>, int> vimap;

	for (int kth_vert = 0; kth_vert < num_verts; ++kth_vert) {

		auto p = positions[kth_vert];
		auto n = normals[kth_vert];
		auto t = texcoords[kth_vert];

		auto bits = Vertex2Bitsets(p, n, t);

		if (vimap.count(bits) == 0) {
			opt_positions.push_back(p);
			opt_normals.push_back(n);
			opt_texcoords.push_back(t);
			indices.push_back(unique_vertex_counter);
			vimap[bits] = unique_vertex_counter;
			++unique_vertex_counter;
		}
		else {
			indices.push_back(vimap[bits]);
		}

	}

	////
	// Create VAO&EBO
	////

	ObjGAsset_Part part{};

	glGenVertexArrays(1, &part.vao);

	if (compute_tangents)
		glGenBuffers(5, part.bufs);
	else
		glGenBuffers(3, part.bufs);

	glBindVertexArray(part.vao);

	glBindBuffer(GL_ARRAY_BUFFER, part.bufs[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(xy::vec3)*opt_positions.size(), opt_positions.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(xy::vec3), (void*)(0));

	glBindBuffer(GL_ARRAY_BUFFER, part.bufs[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(xy::vec3)*opt_normals.size(), opt_normals.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(xy::vec3), (void*)(0));

	glBindBuffer(GL_ARRAY_BUFFER, part.bufs[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(xy::vec2)*opt_texcoords.size(), opt_texcoords.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(xy::vec2), (void*)(0));

	if (compute_tangents) {

		auto num_opt_verts = opt_positions.size();
		auto num_indices = indices.size();
		std::vector<xy::vec3> tangents(num_opt_verts, { 0.f });
		std::vector<xy::vec3> bitangents(num_opt_verts, { 0.f });
		std::vector<int> reps(num_opt_verts, 0);

		int num_faces = num_indices / 3;
		for (int kth_face = 0; kth_face < num_faces; ++kth_face) {
			auto i0 = indices[kth_face * 3 + 0];
			auto i1 = indices[kth_face * 3 + 1];
			auto i2 = indices[kth_face * 3 + 2];

			auto v02 = opt_positions[i2] - opt_positions[i0];
			auto v01 = opt_positions[i1] - opt_positions[i0];

			auto t02 = opt_texcoords[i2] - opt_texcoords[i0];
			auto t01 = opt_texcoords[i1] - opt_texcoords[i0];

			auto det_inv = 1.f / (t01.x*t02.y - t02.x*t01.y);

			auto tangent = det_inv * (t02.y*v01 - t01.y*v02);
			auto bitangent = det_inv * (t01.x*v02 - t02.x*v01);

			tangents[i0] += tangent;
			bitangents[i0] += bitangent;
			reps[i0] += 1;

			tangents[i1] += tangent;
			bitangents[i1] += bitangent;
			reps[i1] += 1;

			tangents[i2] += tangent;
			bitangents[i2] += bitangent;
			reps[i2] += 1;

		}

		for (int kth_opt_vert = 0; kth_opt_vert < num_opt_verts; ++kth_opt_vert) {
			tangents[kth_opt_vert] = xy::Normalize(tangents[kth_opt_vert]);
			bitangents[kth_opt_vert] = xy::Normalize(bitangents[kth_opt_vert]);
		}

		glBindBuffer(GL_ARRAY_BUFFER, part.bufs[3]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(xy::vec3)*tangents.size(), tangents.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(xy::vec3), (void*)(0));

		glBindBuffer(GL_ARRAY_BUFFER, part.bufs[4]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(xy::vec3)*bitangents.size(), bitangents.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(xy::vec3), (void*)(0));
	}

	glGenBuffers(1, &part.ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, part.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);

	part.num_vertices = indices.size();

	return part;

	//// When draw.
	//glUseProgram(shaderProgram);
	//glBindVertexArray(VAO);
	//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0)
	//glBindVertexArray(0);
}

GLuint MakeTextureFromFile(std::string tex_path)
{
	int width, height, num_channels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char *data = stbi_load(tex_path.c_str(), &width, &height, &num_channels, 0);
	if (data == nullptr)
		XY_Die(std::string("failed to load texture(") + tex_path + ")");

	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (num_channels == 4) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}
	else if (num_channels == 3) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	}
	else if (num_channels == 1) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
	}
	else
		XY_Die("Unsupported texture format(#channels not 1, 3 or 4)");

	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(data);
	return tex;
}

ObjGAsset MakeObjGAsset(const ObjAsset & asset)
{
	ObjGAsset gasset;
	gasset.asset = &asset;

	// Make `ObjGAsset_Mtl`s.
	for (int kth_mtl = 0; kth_mtl < asset.mtls.size(); ++kth_mtl) {

		const auto &mtl = asset.mtls[kth_mtl];
		ObjGAsset_Mtl gmtl{};

		gmtl.Ka = xy::FloatArrayToVec(mtl.ambient);
		gmtl.Kd = xy::FloatArrayToVec(mtl.diffuse);
		gmtl.Ks = xy::FloatArrayToVec(mtl.specular);

		if (mtl.ambient_texname != "")
			gmtl.map_Ka = MakeTextureFromFile(asset.mtl_dirpath + mtl.ambient_texname);
		if (mtl.diffuse_texname != "")
			gmtl.map_Kd = MakeTextureFromFile(asset.mtl_dirpath + mtl.diffuse_texname);
		if (mtl.specular_texname != "")
			gmtl.map_Ks = MakeTextureFromFile(asset.mtl_dirpath + mtl.specular_texname);
		if (mtl.alpha_texname != "")
			gmtl.map_d = MakeTextureFromFile(asset.mtl_dirpath + mtl.alpha_texname);
		if (mtl.bump_texname != "")
			gmtl.map_bump = MakeTextureFromFile(asset.mtl_dirpath + mtl.bump_texname);

		const auto &kv = mtl.unknown_parameter.find("map_AO");
		if (kv != mtl.unknown_parameter.end() && kv->second != "")
			gmtl.map_AO = MakeTextureFromFile(asset.mtl_dirpath + kv->second);

		gasset.mtls.push_back(gmtl);
	}

	std::vector<int> mtl_ids(asset.mtls.size(), -1);
	for (int kth_shape = 0; kth_shape < asset.shapes.size(); ++kth_shape) {

		const auto &shape = asset.shapes[kth_shape];

		for (auto &mid : mtl_ids)
			mid = -1;
		for (auto mid : shape.mesh.material_ids)
			mtl_ids[mid] = 1;

		int num_new_parts = 0;
		for (auto &mid : mtl_ids)
			if (mid > 0)
				mid = num_new_parts++;

		struct VertexBlob {
			std::vector<xy::vec3> positions;
			std::vector<xy::vec3> normals;
			std::vector<xy::vec2> texcoords;
		};

		std::vector<VertexBlob> blobs(num_new_parts);
		auto num_old_parts = gasset.parts.size();
		gasset.part2mtl.resize(num_old_parts + num_new_parts);

		auto num_faces = shape.mesh.num_face_vertices.size();
		int index_offset = 0;

		for (int kth_face = 0; kth_face < num_faces; ++kth_face) {
			auto mtl_id = shape.mesh.material_ids[kth_face];
			gasset.part2mtl[num_old_parts + mtl_ids[mtl_id]] = mtl_id;
			auto &blob = blobs[mtl_ids[mtl_id]];
			for (int kthvert = 0; kthvert < 3; ++kthvert) {
				auto &idxset = shape.mesh.indices[index_offset + kthvert];

				float x = asset.attrib.vertices[3 * idxset.vertex_index + 0];
				float y = asset.attrib.vertices[3 * idxset.vertex_index + 1];
				float z = asset.attrib.vertices[3 * idxset.vertex_index + 2];
				blob.positions.emplace_back(x, y, z);

				float nx = asset.attrib.normals[3 * idxset.normal_index + 0];
				float ny = asset.attrib.normals[3 * idxset.normal_index + 1];
				float nz = asset.attrib.normals[3 * idxset.normal_index + 2];
				blob.normals.emplace_back(nx, ny, nz);

				float tx = asset.attrib.texcoords[2 * idxset.texcoord_index + 0];
				float ty = asset.attrib.texcoords[2 * idxset.texcoord_index + 1];
				blob.texcoords.emplace_back(tx, ty);
			}
			index_offset += 3;
		}

		// Reduce vertices.
		for (int kth_blob = 0; kth_blob < blobs.size(); ++kth_blob) {
			const auto &blob = blobs[kth_blob];
			auto mtl = gasset.mtls[gasset.part2mtl[num_old_parts + kth_blob]];
			bool compute_tangents = (mtl.map_bump != 0);

			auto part = MakeObjGAsset_Part(
				blob.positions, blob.normals, blob.texcoords, compute_tangents);

			gasset.parts.push_back(part);
			gasset.part2shape.push_back(kth_shape);
		}

	}

	gasset.model = xy::mat4(1.f);
	AABB bounds;
	for (int kth_vert = 0; kth_vert < asset.attrib.vertices.size() / 3; ++kth_vert) {
		auto x = asset.attrib.vertices[kth_vert * 3 + 0];
		auto y = asset.attrib.vertices[kth_vert * 3 + 1];
		auto z = asset.attrib.vertices[kth_vert * 3 + 2];
		bounds.Extend(xy::vec3{ x, y, z });
	}
	gasset.bounds = bounds;

	return gasset;

}

void DelObjGAsset(ObjGAsset & gasset)
{
	for (auto &part : gasset.parts) {
		glDeleteVertexArrays(1, &part.vao);
		glDeleteBuffers(3, part.bufs);
		glDeleteBuffers(1, &part.ebo);
		part.num_vertices = 0;
	}

	for (auto &mtl : gasset.mtls) {
		glDeleteTextures(1, &mtl.map_Ka);
		glDeleteTextures(1, &mtl.map_Kd);
		glDeleteTextures(1, &mtl.map_Ks);
		glDeleteTextures(1, &mtl.map_d);
		glDeleteTextures(1, &mtl.map_bump);
	}

	gasset.parts.clear();
	gasset.mtls.clear();
	gasset.part2mtl.clear();
	gasset.part2shape.clear();

}

void ObjGAsset_Part::DrawElements(GLenum mode, const std::vector<int>&& attribs) const
{
	glBindVertexArray(vao);
	for (auto attrib : attribs)
		glEnableVertexAttribArray(attrib);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glDrawElements(GL_TRIANGLES, num_vertices, GL_UNSIGNED_INT, 0);

	for (auto attrib : attribs)
		glDisableVertexAttribArray(attrib);
	glBindVertexArray(0);
}

void HairGAsset::DrawIndexed(int index, const std::vector<int>&& attribs) const
{
	glBindVertexArray(vao);
	for (auto attrib : attribs)
		glEnableVertexAttribArray(attrib);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_bufs[index]);

	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(0xFFFFFFFF);
	glDrawElements(GL_LINE_STRIP, num_indices[index], GL_UNSIGNED_INT, (GLvoid*)0);

	for (auto attrib : attribs)
		glDisableVertexAttribArray(attrib);
	glBindVertexArray(0);
}

ArcballCamera::ArcballCamera(AABB bounds, xy::vec3 forward, xy::vec3 up, float FoVy, float aspect)
	:zoom_part_{ 100.f }, cat_{ 10 }
{
	bounds_ = bounds;
	forward_ = forward;
	auto right = xy::Cross(forward, up);
	up_ = xy::Normalize(xy::Cross(right, forward));
	FoVy_ = FoVy;
	aspect_ = aspect;

	bounds_radius_ = bounds.Lengths().Norm()*.5f;
	dist_ = bounds_radius_ / sin(FoVy*.5f);

	nearp_ = dist_ - bounds_radius_;
	farp_ = dist_ + bounds_radius_;

	init_pos_ = Pos();
	init_view_matrix_ = xy::LookAt(init_pos_, bounds_.Center(), up_);
}

xy::mat4 ArcballCamera::View() const
{
	if (tracking)
		return xy::LookAt(Pos(), bounds_.Center(), track_rot_*up_);
	else
		return xy::LookAt(Pos(), bounds_.Center(), up_);
}

xy::mat4 ArcballCamera::Proj() const
{
	return xy::Perspective(FoVy_, aspect_, nearp_, farp_);
}

xy::vec3 ArcballCamera::Pos() const
{
	if (tracking)
		return bounds_.Center() - dist_ * xy::Normalize(track_rot_*forward_);
	else
		return bounds_.Center() - dist_ * xy::Normalize(forward_);
}

void ArcballCamera::HandleInput(const XYInput & input)
{
	auto scroll_offset = input.MouseInput().ScrollOffset();
	if (scroll_offset.y > xy::big_eps<float>)
		zoom_part_.Push();
	if (scroll_offset.y < -xy::big_eps<float>)
		zoom_part_.Pull();

	cat_.Sync([this]() {
		zoom_part_.Simulate();
		FoVy_ = xy::Clamp(FoVy_ + zoom_part_.Velocity(), xy::DegreeToRadian(10.f), xy::DegreeToRadian(120.f));
	});


	bool press = input.MouseInput().ButtonLeftPress();
	bool hit = false;
	xy::vec3 nhit{};

	if (press) {
		auto w = input.WindowWidth(), h = input.WindowHeight();
		auto mpos = input.MouseInput().CursorPosition();
		float y = h - 1 - mpos.y, x = mpos.x;
		xy::vec3 wpos{ x,y,.0f };
		xy::vec4 view_port{ 0,0,static_cast<float>(w),static_cast<float>(h) };

		auto pos = init_pos_;

		auto view_matrix = init_view_matrix_;
		auto proj_matrix = xy::Perspective(FoVy_, aspect_, nearp_, farp_);;
		auto p = xy::UnProject(wpos, view_matrix, proj_matrix, view_port);
		xy::Sphere sphere{ bounds_.Center(), bounds_radius_ };

		xy::Ray ray{ pos,p - pos };

		hit = xy::Hit(sphere, ray);
		nhit = ray.p() - bounds_.Center();
		nhit = xy::Normalize(nhit);
	}

	bool tracking_now = press && hit;

	if (!tracking && tracking_now) {
		first_hit_ = nhit;
		track_rot_ = xy::quat{ 1,0,0,0 };
		tracking = true;
		return;
	}

	if (tracking && !tracking_now) {
		track_rot_prev_ = track_rot_ * track_rot_prev_;
		forward_ = track_rot_ * forward_;
		up_ = track_rot_ * up_;
		if (std::abs(xy::Dot(up_, forward_)) > xy::big_eps<float>) {
			auto right = xy::Cross(forward_, up_);
			up_ = xy::Cross(right, forward_);
		}
		tracking = false;

		return;
	}

	if (tracking&&tracking_now) {

		if ((nhit - first_hit_).Norm() < xy::big_eps<float>)
			return;

		auto qrot_axis = track_rot_prev_ * xy::Cross(first_hit_, nhit);

		float dw = -std::acos(xy::Clamp(Dot(first_hit_, nhit), -1.f, 1.f));
		track_rot_ = xy::AngleAxisToQuat(dw, xy::Normalize(qrot_axis));
		return;
	}

}

WanderCamera::WanderCamera(xy::vec3 position, xy::vec3 target, xy::vec3 up, float FoVy, float aspect, float nearp, float farp)
	:par_m_lr_{ 10.f },
	par_m_fb_{ 10.f },
	par_r_lr_{ 10.f },
	par_r_ud_{ 10.f },
	par_r_roll_{ 10.f },
	cat_{ 10 }
{
	pos_ = position;
	forward_ = target - position;

	up_ = up;

	FoVy_ = FoVy;
	aspect_ = aspect;
	nearp_ = nearp;
	farp_ = farp;
}

xy::vec3 WanderCamera::Pos() const
{
	return pos_;
}

xy::mat4 WanderCamera::View() const
{
	return xy::LookAt(pos_, pos_ + forward_, up_);
}

xy::mat4 WanderCamera::Proj() const
{
	return xy::Perspective(FoVy_, aspect_, nearp_, farp_);
}

void WanderCamera::HandleInput(const XYInput & input)
{
	if (input.KeyboardInput().A()) {
		par_m_lr_.Pull();
	}
	if (input.KeyboardInput().D()) {
		par_m_lr_.Push();
	}
	if (input.KeyboardInput().W()) {
		par_m_fb_.Push();
	}
	if (input.KeyboardInput().S()) {
		par_m_fb_.Pull();
	}

	if (input.KeyboardInput().H()) {
		par_r_lr_.Push();
	}
	if (input.KeyboardInput().L()) {
		par_r_lr_.Pull();
	}
	if (input.KeyboardInput().J()) {
		par_r_ud_.Pull();
	}
	if (input.KeyboardInput().K()) {
		par_r_ud_.Push();
	}
	if (input.KeyboardInput().N()) {
		par_r_roll_.Pull();
	}
	if (input.KeyboardInput().M()) {
		par_r_roll_.Push();
	}

	cat_.Sync([this, &input]() {

		par_m_lr_.Simulate();
		par_m_fb_.Simulate();
		par_r_lr_.Simulate();
		par_r_ud_.Simulate();
		par_r_roll_.Simulate();

		auto right = xy::Normalize(xy::Cross(forward_, up_));
		up_ = xy::Normalize(xy::Cross(right, forward_));

		pos_ += xy::Normalize(right)*par_m_lr_.Velocity();
		pos_ += xy::Normalize(forward_)*par_m_fb_.Velocity();

		forward_ = xy::AngleAxisToQuat(par_r_lr_.Velocity()*.01f, up_)*forward_;
		forward_ = xy::AngleAxisToQuat(par_r_ud_.Velocity()*.01f, right)*forward_;

		up_ = xy::AngleAxisToQuat(par_r_roll_.Velocity()*.01f, forward_)*up_;
	});

}

LightCamera::LightCamera(const AABB & bounds, xy::vec3 forward, xy::vec3 up)
{
	auto inf = bounds.inf;
	auto sup = bounds.sup;

	auto diag0 = sup - inf;

	std::swap(inf.x, sup.x);
	auto diag1 = sup - inf;
	std::swap(inf.x, sup.x);

	std::swap(inf.y, sup.y);
	auto diag2 = sup - inf;
	std::swap(inf.y, sup.y);

	std::swap(inf.z, sup.z);
	auto diag3 = sup - inf;
	std::swap(inf.z, sup.z);

	forward = xy::Normalize(forward);
	auto right = xy::Normalize(xy::Cross(forward, up));
	up = xy::Normalize(xy::Cross(right, forward));

	auto u1 = std::max(std::abs(xy::Dot(up, diag0)), std::abs(xy::Dot(up, diag1)));
	auto u2 = std::max(std::abs(xy::Dot(up, diag2)), std::abs(xy::Dot(up, diag3)));
	auto u = std::max(u1, u2);

	auto r1 = std::max(std::abs(xy::Dot(right, diag0)), std::abs(xy::Dot(right, diag1)));
	auto r2 = std::max(std::abs(xy::Dot(right, diag2)), std::abs(xy::Dot(right, diag3)));
	auto r = std::max(r1, r2);

	auto f1 = std::max(std::abs(xy::Dot(forward, diag0)), std::abs(xy::Dot(forward, diag1)));
	auto f2 = std::max(std::abs(xy::Dot(forward, diag2)), std::abs(xy::Dot(forward, diag3)));
	auto f = std::max(f1, f2);

	auto pos_ = bounds.Center() - forward * .5f*f;
	auto tgt = bounds.Center();

	float nearp = 0;
	float farp = f;
	float leftp = -.5f*r;
	float rightp = .5f*r;
	float topp = .5f*u;
	float bottomp = -.5f*u;

	view_matrix_ = xy::LookAt(pos_, tgt, up);
	proj_matrix_ = xy::Orthographic(leftp, rightp, bottomp, topp, nearp, farp);
}

xy::mat4 LightCamera::View() const
{
	return view_matrix_;
}

xy::mat4 LightCamera::Proj() const
{
	return proj_matrix_;
}

xy::vec3 LightCamera::Pos() const
{
	return pos_;
}

