//-------------------------------------------------------------------------------------------------
// File : SimplePS.hlsl
// Desc : Simpile Pixel Shader.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////////////////////////
// VSOutput structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct VSOutput
{
    float4  Position : SV_POSITION;
    float4  Color    : VTX_COLOR;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// PSOutput structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct PSOutput
{
    float4  Color : SV_TARGET0;
};

//-------------------------------------------------------------------------------------------------
//      メインエントリーポイントです.
//-------------------------------------------------------------------------------------------------
PSOutput PSFunc( const VSOutput input )
{
    PSOutput output = (PSOutput)0;

    output.Color = input.Color;

    return output;
}