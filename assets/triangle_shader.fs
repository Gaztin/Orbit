#if defined(ORB_GLSL)

uniform sampler2D diffuse_texture;

ORB_VARYING vec4 v_position;
ORB_VARYING vec4 v_color;
ORB_VARYING vec2 v_texcoord;

void main()
{
	vec4 tex_color = texture(diffuse_texture, v_texcoord);
	vec4 out_color = tex_color * v_color;

	ORB_SET_OUT_COLOR(out_color);
}

#elif defined(ORB_HLSL)

Texture2D diffuse_texture;

SamplerState texture_sampler;

struct PixelInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 texcoord : TEXCOORD;
};

float4 main(PixelInput input) : SV_TARGET
{
	float4 tex_color = diffuse_texture.Sample(texture_sampler, input.texcoord);
	float4 out_color = tex_color * input.color;

	return out_color;
}

#endif
