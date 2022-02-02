// Where the fun takes place.
//

struct VS_INPUT {
   float2 pos   : POSITION;
   float2 uv    : TEXCOORD;
   float4 color : COLOR;
};

struct PS_INPUT {
    float4 pos   : SV_POSITION;
    float2 uv    : TEXCOORD;
    float4 color : COLOR;
};



static const float      PI          = 3.14159265359f;
static const float      TWO_PI      = 2.0f * PI;
static const float      PI_HALF     = PI / 2.0f;

// Visible to all shaders.

cbuffer cbuffer0 : register(b0) {
    float width;
    float height;
    float aspect;
    float uptime;
};



PS_INPUT vs(VS_INPUT input)
{
    float2 pos = input.pos;
    float2 uv = input.uv;


    float angle = fmod(uptime / 21.0f, 1.0f) * TWO_PI;

    float2x2 rotation = {
         aspect * cos(angle),    aspect * sin(angle),
                 -sin(angle),             cos(angle),
    };

    // Rotate the triangle.
    pos = mul(rotation, pos);

    // Zoom the triangle in/out.
    pos *= pow(cos(uptime - PI) + 1.0f, 3.0f) + 1.0f;

    // Zoom the checkerboard in/out.
    uv *= cos(uptime) + 1.0f;


    PS_INPUT output;
    output.pos = float4(pos, 0.0f, 1.0f);
    output.uv = uv;
    output.color = input.color;
    return output;
}



// Only visible to the pixel shader.

sampler sampler0 : register(s0);
Texture2D<float4> texture0 : register(t0);

float4 ps(PS_INPUT input) : SV_TARGET
{
    float4 texel = texture0.Sample(sampler0, input.uv);
    float4 color = input.color;

    // Fade the checkerboard in/out.
    texel.a *= (cos(uptime) + 1.0f)/2.0f;

    color.rgb = (texel.rgb * texel.a) + color.rgb * (1.0f - texel.a);
    return color;
}