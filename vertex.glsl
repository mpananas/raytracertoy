#version 330
layout (location = 0) in vec2 in_pos;
layout (location = 1) in vec2 in_tex;

out vec2 tex_coord;
uniform float ratio;

void main()
{
	gl_Position = vec4(in_pos.x, ratio * in_pos.y, 0.0f, 1.0f);
	tex_coord = in_tex;
}
