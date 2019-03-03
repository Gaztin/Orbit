#if defined(ORB_GLSL)

layout (std140) uniform Constants
{
	float diffuse;
};

ORB_ATTRIBUTE vec4 a_position;
ORB_ATTRIBUTE vec4 a_color;

ORB_VARYING vec4 v_position;
ORB_VARYING vec4 v_color;

void main()
{
	v_position = a_position;
	v_color = a_color * diffuse;

	gl_Position = a_position;
}

#elif defined(ORB_HLSL)

cbuffer Constants
{
	float diffuse;
};

struct VertexInput
{
	float4 position : POSITION;
	float4 color : COLOR;
};

struct VertexOutput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

VertexOutput main(VertexInput input)
{
	VertexOutput output;
	output.position = input.position;
	output.color = input.color * diffuse;

	return output;
}

#endif
