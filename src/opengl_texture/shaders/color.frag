#version 430 core

out vec4 frag_color;

in vec3 vertex_color;
in vec2 vertex_texture_pos;

uniform sampler2D uniform_texture0;
uniform sampler2D uniform_texture1;

void main()
{
    frag_color = mix(texture(uniform_texture0, vertex_texture_pos), texture(uniform_texture1, vertex_texture_pos), 0.2);
}
