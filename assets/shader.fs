#if defined(ORB_GLSL)

layout (location = 0) in vec4 v_position;
layout (location = 1) in vec4 v_color;

layout (location = 0) out vec4 fragColor;

void main()
{
	fragColor = v_color;
}

#elif defined(ORB_HLSL)

struct PixelInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

float4 main(PixelInput input) : SV_TARGET
{
	return input.color;
}

#endif
