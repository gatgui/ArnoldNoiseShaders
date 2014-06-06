#include <ai.h>
#include "libnoise/perlin.h"
#include "ln_quality.h"

AI_SHADER_NODE_EXPORT_METHODS(PerlinMtd);

enum PerlinParams
{
   p_frequency = 0,
   p_lacunarity,
   p_octaves,
   p_persistence,
   p_quality,
   p_seed,
   p_input,
   p_custom_input
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
   AiParameterFlt("frequency", 1.0f);
   AiParameterFlt("lacunarity", 2.0f);
   AiParameterInt("octaves", 6);
   AiParameterFlt("persistence", 0.5f);
   AiParameterEnum("quality", NQ_std, NoiseQualityNames);
   AiParameterInt("seed", 0);
   AiParameterEnum("input", I_P, InputNames);
   AiParameterPnt("custom_input", 0.0f, 0.0f, 0.0f);
   
   AiMetaDataSetBool(mds, "quality", "linkable", false);
   AiMetaDataSetBool(mds, "input", "linkable", false);
}

node_initialize
{
   noise::module::Perlin *ln_mod = new noise::module::Perlin();
   AiNodeSetLocalData(node, ln_mod);
}

node_update
{
}

node_finish
{
   noise::module::Perlin *ln_mod = (noise::module::Perlin*) AiNodeGetLocalData(node);
   delete ln_mod;
}

shader_evaluate
{
   noise::module::Perlin *ln_mod = (noise::module::Perlin*) AiNodeGetLocalData(node);
   
   ln_mod->SetFrequency(AiShaderEvalParamFlt(p_frequency));
   ln_mod->SetLacunarity(AiShaderEvalParamFlt(p_lacunarity));
   ln_mod->SetOctaveCount(AiShaderEvalParamInt(p_octaves));
   ln_mod->SetPersistence(AiShaderEvalParamFlt(p_persistence));
   ln_mod->SetNoiseQuality((noise::NoiseQuality) AiShaderEvalParamInt(p_quality));
   ln_mod->SetSeed(AiShaderEvalParamInt(p_seed));
   
   AtPoint P;
   
   Input input = (Input) AiShaderEvalParamInt(p_input);
   
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
   
   sg->out.FLT = ln_mod->GetValue(P.x, P.y, P.z);
}
