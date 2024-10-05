struct UniformBufferControl
{
    float4x4 projection;
    float4x4 model;
    float4x4 view;
};

cbuffer ubo 
{
    UniformBufferControl ubo;
}

struct VSInput
{
    [[vk::location(0)]] float2 Position : POSTION0;
    [[vk::location(1)]] float3 Color : COLOR0;
};

struct VSOutput
{
    [[vk::location(0)]] float4 Position : SV_Position;
    [[vk::location(1)]] float3 Color : COLOR0;
};

VSOutput VS_main(VSInput input, uint VertexIndex : SV_VertexID)
{
    VSOutput output = (VSOutput) 0;
    output.Position = mul(ubo.projection, mul(ubo.view, mul(ubo.model, float4(input.Position, 0.0, 1.0))));
    output.Color = input.Color;
    return output;
}

float4 FS_main(VSOutput input) : SV_Target
{
    return float4(input.Color, 1.0);
}
