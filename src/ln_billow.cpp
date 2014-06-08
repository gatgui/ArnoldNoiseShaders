#include <ai.h>
#include "libnoise/noisegen.h"
#include "ln_quality.h"

AI_SHADER_NODE_EXPORT_METHODS(BillowMtd);

enum BillowParams
{
   p_input = 0,
   p_custom_input,
   p_amplitude,
   p_frequency,
   p_octaves,
   p_persistence,
   p_lacunarity,
   p_seed,
   p_quality
};

enum Input
{
   I_P = 0,
   I_Pref,
   I_Po,
   I_UV,
   I_Custom
};

static const char* InputNames[] =
{
   "P",
   "Pref",
   "Po",
   "UV",
   "Custom",
   NULL
};

node_parameters
{
   AiParameterEnum("input", I_P, InputNames);
   AiParameterPnt("custom_input", 0.0f, 0.0f, 0.0f);
   AiParameterFlt("amplitude", 1.0f);
   AiParameterFlt("frequency", 1.0f);
   AiParameterInt("octaves", 6);
   AiParameterFlt("persistence", 0.5f);
   AiParameterFlt("lacunarity", 2.0f);
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
}

node_finish
{
}

shader_evaluate
{
   float amplitude = AiShaderEvalParamFlt(p_amplitude);
   float frequency = AiShaderEvalParamFlt(p_frequency);
   float lacunarity = AiShaderEvalParamFlt(p_lacunarity);
   int octaves = AiShaderEvalParamInt(p_octaves);
   float persistence = AiShaderEvalParamFlt(p_persistence);
   noise::NoiseQuality quality = (noise::NoiseQuality) AiShaderEvalParamInt(p_quality);
   int seed = AiShaderEvalParamInt(p_seed);
   
   Input input = (Input) AiShaderEvalParamInt(p_input);
   AtPoint P;
   
   switch (input)
   {
   case I_P:
      P = sg->P;
      break;
   case I_Pref:
      if (!AiUDataGetPnt("Pref", &P))
      {
         P = sg->P;
      }
      break;
   case I_Po:
      P = sg->Po;
      break;
   case I_UV:
      P.x = sg->u;
      P.y = sg->v;
      P.z = 0.0f;
      break;
   case I_Custom:
      P = AiShaderEvalParamPnt(p_custom_input);
      break;
   default:
      break;
   }
   
   sg->out.FLT = 0.0f;
   
   float curAmplitude = amplitude;
   float x = P.x * frequency;
   float y = P.y * frequency;
   float z = P.z * frequency;
   float nx, ny, nz;
   float nval;
   
   for (int curOctave=0; curOctave<octaves; ++curOctave)
   {
      // Make sure that these floating-point values have the same range as a 32-
      // bit integer so that we can pass them to the coherent-noise functions.
      nx = noise::MakeInt32Range(x);
      ny = noise::MakeInt32Range(y);
      nz = noise::MakeInt32Range(z);

      // Get the coherent-noise value from the input value and add it to the final result.
      seed = (seed + curOctave) & 0xffffffff;
      nval = noise::GradientCoherentNoise3D(nx, ny, nz, seed, quality);
      nval = 2.0f * fabsf(nval) - 1.0f;
      sg->out.FLT += curAmplitude * nval;

      // Prepare the next octave.
      x *= lacunarity;
      y *= lacunarity;
      z *= lacunarity;
      
      curAmplitude *= persistence;
   }
   
   sg->out.FLT += 0.5f;
}
