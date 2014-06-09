#include "ln_common.h"

AI_SHADER_NODE_EXPORT_METHODS(RidgedMtd);

enum RidgedParams
{
   p_input = 0,
   p_custom_input,
   p_amplitude,
   p_frequency,
   p_octaves,
   p_lacunarity,
   p_exponent,
   p_offset,
   p_gain,
   p_seed,
   p_quality
};

node_parameters
{
   AiParameterEnum("input", I_P, InputNames);
   AiParameterPnt("custom_input", 0.0f, 0.0f, 0.0f);
   AiParameterFlt("amplitude", 1.0f);
   AiParameterFlt("frequency", 1.0f);
   AiParameterInt("octaves", 6);
   AiParameterFlt("lacunarity", 2.0f);
   AiParameterFlt("exponent", 1.0f);
   AiParameterFlt("offset", 1.0f);
   AiParameterFlt("gain", 2.0f);
   AiParameterInt("seed", 0);
   AiParameterEnum("quality", NQ_std, NoiseQualityNames);
   
   AiMetaDataSetBool(mds, "quality", "linkable", false);
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
   bool is_input_linked = (AiNodeGetLocalData(node) == (void*)1);
   
   float amplitude = AiShaderEvalParamFlt(p_amplitude);
   float frequency = AiShaderEvalParamFlt(p_frequency);
   float lacunarity = AiShaderEvalParamFlt(p_lacunarity);
   float exponent = AiShaderEvalParamFlt(p_exponent);
   float offset = AiShaderEvalParamFlt(p_offset);
   float gain = AiShaderEvalParamFlt(p_gain);
   int octaves = AiShaderEvalParamInt(p_octaves);
   NoiseQuality quality = (NoiseQuality) AiShaderEvalParamInt(p_quality);
   int seed = AiShaderEvalParamInt(p_seed);
   Input input = (Input) AiShaderEvalParamInt(p_input);
   
   AtPoint P = (is_input_linked ? AiShaderEvalParamPnt(p_custom_input) : GetInput(input, sg, node));
   
   P *= frequency;
   
   sg->out.FLT = 0.0f;
   
   float f = frequency;
   float w = 1.0f;
   float nx, ny, nz, s;
   
   for (int curOctave=0; curOctave<octaves; ++curOctave)
   {
      // Make sure that these floating-point values have the same range as a 32-
      // bit integer so that we can pass them to the coherent-noise functions.
      nx = noise::MakeInt32Range(P.x);
      ny = noise::MakeInt32Range(P.y);
      nz = noise::MakeInt32Range(P.z);
      
      // Get the coherent-noise value from the input value and add it to the final result.
      seed = (seed + curOctave) & 0xffffffff;
      
      s = amplitude * noise::GradientCoherentNoise3D(nx, ny, nz, seed, (noise::NoiseQuality)quality);
      
      // making rigdes
      s = fabsf(s);
      s = offset - s;
      s *= s;
      s *= w;
      
      w = CLAMP(s * gain, 0.0f, 1.0f);
      
      sg->out.FLT += (s * powf(f, -exponent));
      
      P *= lacunarity;
      f *= lacunarity;
   }
   
   sg->out.FLT = 1.25f * sg->out.FLT - 1.0f;
   
   // re-normalize
   sg->out.FLT = std::max(0.0f, 0.5f * (1.0f + sg->out.FLT));
}
