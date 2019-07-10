#if defined(ORB_GLSL)

ORB_CONSTANTS_BEGIN(Constants)
	ORB_CONSTANT(mat4, mvp);
ORB_CONSTANTS_END

ORB_ATTRIBUTE vec4 a_position;
ORB_ATTRIBUTE vec4 a_color;

ORB_VARYING vec4 v_position;
ORB_VARYING vec4 v_color;

void main()
{
	v_position = mvp * a_position;
	v_color = a_color;

	gl_Position = v_position;
}

#elif defined(ORB_HLSL)

cbuffer Constants
{
	matrix mvp;
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
	output.position = mul(input.position, mvp);
	output.color = input.color;

	return output;
}

#endif
