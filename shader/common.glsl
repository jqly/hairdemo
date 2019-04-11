void PositionToWorldAndClipSpace(
    mat4 model,
    mat4 viewproj,
    vec3 position,
    out vec3 world_space_position, 
    out vec4 clip_space_position)
{
    vec4 ws_pos_4 = model*vec4(position,1.);
    world_space_position = ws_pos_4.xyz;
    clip_space_position = viewproj*ws_pos_4;
}

void PositionToClipSpace(
    mat4 model,
    mat4 viewproj,
    vec3 position,
    out vec4 clip_space_position)
{
    vec4 ws_pos_4 = model*vec4(position,1.);
    clip_space_position = viewproj*ws_pos_4;
}

void NormalToWorld(
    mat4 model, 
    vec3 normal, 
    out vec3 world_space_world)
{
    world_space_world = mat3(transpose(inverse(model)))*normal;
}
