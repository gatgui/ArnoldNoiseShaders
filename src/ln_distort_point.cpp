#include "ln_common.h"

AI_SHADER_NODE_EXPORT_METHODS(DistortPointMtd);

enum DistortPointParams
{
   p_input = 0,
   p_custom_input,
   p_frequency,
   p_power,
   p_roughness,
   p_base_noise,
   p_value_seed,
   p_perlin_seed,
   p_flow_power,
   p_flow_time
};

node_parameters
{
   AiParameterEnum("input", I_P, InputNames);
   AiParameterPnt("custom_input", 0.0f, 0.0f, 0.0f);
   
   AiParameterFlt("frequency", 1.0f);
   AiParameterFlt("power", 1.0f);
   AiParameterInt("roughness", 3);
   
   AiParameterEnum("base_noise", NT_simplex, NoiseTypeNames);
   AiParameterInt("value_seed", 0);
   AiParameterInt("perlin_seed", 0);
   AiParameterFlt("flow_power", 0.25f);
   AiParameterFlt("flow_time", 0.0f);
   
   AiMetaDataSetBool(mds, "input", "linkable", false);
   AiMetaDataSetBool(mds, "base_noise", "linkable", false);
   AiMetaDataSetInt(mds, "value_seed", "softmin", 0);
   AiMetaDataSetInt(mds, "value_seed", "softmax", 10);
   AiMetaDataSetInt(mds, "perlin_seed", "softmin", 0);
   AiMetaDataSetInt(mds, "perlin_seed", "softmax", 10);
   AiMetaDataSetFlt(mds, "flow_power", "softmin", 0.0f);
   AiMetaDataSetFlt(mds, "flow_power", "softmax", 1.0f);
   AiMetaDataSetFlt(mds, "flow_time", "softmin", 0.0f);
   AiMetaDataSetFlt(mds, "flow_time", "softmax", 10.0f);
   
   // Houdini specifics
   AiMetaDataSetStr(mds, "value_seed", "houdini.hide_when", "{ base_noise != value }");
   AiMetaDataSetStr(mds, "perlin_seed", "houdini.hide_when", "{ base_noise != perlin }");
   AiMetaDataSetStr(mds, "flow_power", "houdini.hide_when", "{ base_noise != flow }");
   AiMetaDataSetStr(mds, "flow_time", "houdini.hide_when", "{ base_noise != flow }");
}

node_initialize
{
}

node_update
{
   AiNodeSetLocalData(node, reinterpret_cast<void*>(AiNodeIsLinked(node, "custom_input") ? 1 : 0));
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
   Input input = (Input) AiShaderEvalParamInt(p_input);
   AtPoint P = (is_input_linked ? AiShaderEvalParamPnt(p_custom_input) : GetInput(input, sg, node));
   
   NoiseType nt = (NoiseType) AiShaderEvalParamInt(p_base_noise);
   float frequency = AiShaderEvalParamFlt(p_frequency);
   float power = AiShaderEvalParamFlt(p_power);
   int roughness = AiShaderEvalParamInt(p_roughness);
   
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
   
   switch (nt)
   {
   case NT_value:
      {
         int seed = AiShaderEvalParamInt(p_value_seed);
         fBm<ValueNoise, DefaultModifier> fbm(roughness, 1.0f, 0.5f, frequency, 2.0f);
         fbm.noise_params.quality = NQ_std;
         fbm.noise_params.seed = seed + 0;
         sg->out.PNT.x = P.x + power * fbm.eval(P0, false);
         fbm.noise_params.seed = seed + 1;
         sg->out.PNT.y = P.y + power * fbm.eval(P1, false);
         fbm.noise_params.seed = seed + 2;
         sg->out.PNT.z = P.z + power * fbm.eval(P2, false);
      }
      break;
   case NT_perlin:
      {
         int seed = AiShaderEvalParamInt(p_perlin_seed);
         fBm<PerlinNoise, DefaultModifier> fbm(roughness, 1.0f, 0.5f, frequency, 2.0f);
         fbm.noise_params.quality = NQ_std;
         fbm.noise_params.seed = seed + 0;
         sg->out.PNT.x = P.x + power * fbm.eval(P0, false);
         fbm.noise_params.seed = seed + 1;
         sg->out.PNT.y = P.y + power * fbm.eval(P1, false);
         fbm.noise_params.seed = seed + 2;
         sg->out.PNT.z = P.z + power * fbm.eval(P2, false);
      }
      break;
   case NT_flow:
      {
         fBm<FlowNoise, DefaultModifier> fbm(roughness, 1.0f, 0.5f, frequency, 2.0f);
         fbm.noise_params.power = AiShaderEvalParamFlt(p_flow_power);
         fbm.noise_params.t = AiShaderEvalParamFlt(p_flow_time);
         sg->out.PNT.x = P.x + power * fbm.eval(P0, false);
         sg->out.PNT.y = P.y + power * fbm.eval(P1, false);
         sg->out.PNT.z = P.z + power * fbm.eval(P2, false);
      }
      break;
   case NT_simplex:
   default:
      {
         fBm<SimplexNoise, DefaultModifier> fbm(roughness, 1.0f, 0.5f, frequency, 2.0f);
         sg->out.PNT.x = P.x + power * fbm.eval(P0, false);
         sg->out.PNT.y = P.y + power * fbm.eval(P1, false);
         sg->out.PNT.z = P.z + power * fbm.eval(P2, false);
      }
      break;
   }
}
