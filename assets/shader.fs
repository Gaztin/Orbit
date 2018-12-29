#if defined(ORB_GLSL)

varying vec4 v_position;
varying vec4 v_color;

void main()
{
	gl_FragColor = v_color;
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
