#include "ln_common.h"

AI_SHADER_NODE_EXPORT_METHODS(TurbulenceMtd);

enum TurbulenceParams
{
   p_input = 0,
   p_custom_input,
   p_frequency,
   p_power,
   p_roughness,
   p_seed
};

node_parameters
{
   AiParameterEnum("input", I_P, InputNames);
   AiParameterPnt("custom_input", 0.0f, 0.0f, 0.0f);
   AiParameterFlt("frequency", 1.0f);
   AiParameterFlt("power", 1.0f);
   AiParameterInt("roughness", 3);
   AiParameterInt("seed", 0);
   
   AiMetaDataSetBool(mds, "input", "linkable", false);
}

node_initialize
{
}

node_update
{
   AiNodeSetLocalData(node, (void*) (AiNodeIsLinked(node, "custom_input") ? 1 : 0));
}

node_finish
{
}

shader_evaluate
{
   static float x0 = (12414.0f / 65536.0f);
   static float y0 = (65124.0f / 65536.0f);
   static float z0 = (31337.0f / 65536.0f);
   static float x1 = (26519.0f / 65536.0f);
   static float y1 = (18128.0f / 65536.0f);
   static float z1 = (60493.0f / 65536.0f);
   static float x2 = (53820.0f / 65536.0f);
   static float y2 = (11213.0f / 65536.0f);
   static float z2 = (44845.0f / 65536.0f);
   
   bool is_input_linked = (AiNodeGetLocalData(node) == (void*)1);
   
   float frequency = AiShaderEvalParamFlt(p_frequency);
   float power = AiShaderEvalParamFlt(p_power);
   int roughness = AiShaderEvalParamInt(p_roughness);
   int seed = AiShaderEvalParamInt(p_seed);
   Input input = (Input) AiShaderEvalParamInt(p_input);
   
   AtPoint P = (is_input_linked ? AiShaderEvalParamPnt(p_custom_input) : GetInput(input, sg, node));
   
   AtPoint P0, P1, P2;
   
   P0.x = P.x + x0;
   P0.y = P.y + y0;
   P0.z = P.z + z0;
   
   P1.x = P.x + x1;
   P1.y = P.y + y1;
   P1.z = P.z + z1;
   
   P2.x = P.x + x2;
   P2.y = P.y + y2;
   P2.z = P.z + z2;
   
   sg->out.PNT.x = P.x + Fractal(P0, roughness, 1.0f, 0.5f, frequency, 2.0f, seed) * power;
   sg->out.PNT.y = P.y + Fractal(P1, roughness, 1.0f, 0.5f, frequency, 2.0f, seed+1) * power;
   sg->out.PNT.z = P.z + Fractal(P2, roughness, 1.0f, 0.5f, frequency, 2.0f, seed+2) * power;
}
