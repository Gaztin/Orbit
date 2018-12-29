#if defined(ORB_GLSL)

attribute vec4 a_position;
attribute vec4 a_color;

varying vec4 v_position;
varying vec4 v_color;

void main()
{
	v_position = a_position;
	v_color = a_color;

	gl_Position = a_position;
}

#elif defined(ORB_HLSL)

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
	output.color = input.color;

	return output;
}

#endif
