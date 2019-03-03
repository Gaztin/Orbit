#if defined(ORB_GLSL)

ORB_VARYING vec4 v_position;
ORB_VARYING vec4 v_color;

void main()
{
	ORB_SET_OUT_COLOR(v_color);
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
