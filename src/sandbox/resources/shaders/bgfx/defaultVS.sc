$input a_position

#include <bgfx_shader.sh>

void main()
{
	gl_Position = mul(u_modelViewProj, a_position);
}