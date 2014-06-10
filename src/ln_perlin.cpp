#include "ln_common.h"

AI_SHADER_NODE_EXPORT_METHODS(PerlinMtd);

enum PerlinParams
{
   p_input = 0,
   p_custom_input,
   p_base_noise,
   p_amplitude,
   p_frequency,
   p_octaves,
   p_persistence,
   p_lacunarity,
   p_seed,
   p_quality,
   p_normalize_output
};

node_parameters
{
   AiParameterEnum("input", I_P, InputNames);
   AiParameterPnt("custom_input", 0.0f, 0.0f, 0.0f);
   AiParameterEnum("base_noise", NT_simplex, NoiseTypeNames);
   AiParameterFlt("amplitude", 1.0f);
   AiParameterFlt("frequency", 1.0f);
   AiParameterInt("octaves", 6);
   AiParameterFlt("persistence", 0.5f);
   AiParameterFlt("lacunarity", 2.0f);
   AiParameterInt("seed", 0);
   AiParameterEnum("quality", NQ_std, NoiseQualityNames);
   AiParameterBool("normalize_output", true);
   
   AiMetaDataSetBool(mds, "quality", "linkable", false);
   AiMetaDataSetBool(mds, "input", "linkable", false);
   AiMetaDataSetBool(mds, "base_noise", "linkable", false);
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
   bool is_input_linked = (AiNodeGetLocalData(node) == (void*)1);
   Input input = (Input) AiShaderEvalParamInt(p_input);
   AtPoint P = (is_input_linked ? AiShaderEvalParamPnt(p_custom_input) : GetInput(input, sg, node));
   NoiseType nt = (NoiseType) AiShaderEvalParamInt(p_base_noise);
   int octaves = AiShaderEvalParamInt(p_octaves);
   float amplitude = AiShaderEvalParamFlt(p_amplitude);
   float persistence = AiShaderEvalParamFlt(p_persistence);
   float frequency = AiShaderEvalParamFlt(p_frequency);
   float lacunarity = AiShaderEvalParamFlt(p_lacunarity);
   
   switch (nt)
   {
   case NT_value:
      {
         fBm<ValueNoise, DefaultModifier> fbm(octaves, amplitude, persistence, frequency, lacunarity);
         fbm.noise_params.seed = AiShaderEvalParamInt(p_seed);
         fbm.noise_params.quality = (NoiseQuality) AiShaderEvalParamInt(p_quality);
         sg->out.FLT = fbm.eval(P);
      }
      break;
   case NT_perlin:
      {
         fBm<PerlinNoise, DefaultModifier> fbm(octaves, amplitude, persistence, frequency, lacunarity);
         fbm.noise_params.seed = AiShaderEvalParamInt(p_seed);
         fbm.noise_params.quality = (NoiseQuality) AiShaderEvalParamInt(p_quality);
         sg->out.FLT = fbm.eval(P);
      }
      break;
   case NT_simplex:
   default:
      {
         fBm<SimplexNoise, DefaultModifier> fbm(octaves, amplitude, persistence, frequency, lacunarity);
         sg->out.FLT = fbm.eval(P);
      }
   }
   
   if (AiShaderEvalParamBool(p_normalize_output))
   {
      sg->out.FLT = std::max(0.0f, 0.5f * (1.0f + sg->out.FLT));
   }
}
