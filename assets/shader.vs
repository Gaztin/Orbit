#ifdef ORB_GLSL

layout (location = 0) in vec2 a_position;
layout (location = 1) in vec4 a_color;

layout (location = 0) out vec4 v_color;

void main()
{
	v_color = a_color;

	gl_Position = a_position;
}

#endif
#ifdef ORB_HLSL

struct VertexInput
{
	float2 position : POSITION;
	float4 color : COLOR;
};

struct VertexOutput
{
	float4 color : COLOR;
};

VertexOutput main(VertexInput input)
{
	VertexOutput output;
	output.color = input.color;

	return output;
}

#endif
