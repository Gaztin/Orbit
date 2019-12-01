#if defined(ORB_GLSL)

ORB_ATTRIBUTE vec4 a_position;
ORB_ATTRIBUTE vec4 a_color;
ORB_ATTRIBUTE vec2 a_texcoord;

ORB_VARYING vec4 v_position;
ORB_VARYING vec4 v_color;
ORB_VARYING vec2 v_texcoord;

void main()
{
	v_position = a_position;
	v_color = a_color;
	v_texcoord = a_texcoord;

	gl_Position = v_position;
}

#elif defined(ORB_HLSL)

struct VertexInput
{
	float4 position : POSITION;
	float4 color : COLOR;
	float2 texcoord : TEXCOORD;
};

struct VertexOutput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 texcoord : TEXCOORD;
};

VertexOutput main(VertexInput input)
{
	VertexOutput output;
	output.position = input.position;
	output.color = input.color;
	output.texcoord = input.texcoord;

	return output;
}

#endif
