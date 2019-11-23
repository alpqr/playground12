cbuffer ConstantBuffer : register(b0)
{
    float4x4 mvp;
};

struct Out
{
    float4 position : SV_POSITION;
};

Out main(float4 position : POSITION)
{
    Out result;
    result.position = mul(mvp, position);
    return result;
}
