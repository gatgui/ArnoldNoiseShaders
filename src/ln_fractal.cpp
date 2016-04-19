#include "ln_common.h"

AI_SHADER_NODE_EXPORT_METHODS(FractalMtd);

enum FractalParams
{
   p_input = 0,
   p_custom_input,
   
   // fBm parameters
   p_amplitude,
   p_frequency,
   p_octaves,
   p_persistence,
   p_lacunarity,
   
   p_base_noise,
   // value noise parameters
   p_value_seed,
   p_value_quality,
   // perlin noise parameters
   p_perlin_seed,
   p_perlin_quality,
   // flow noise parameters
   p_flow_power,
   p_flow_time,
   
   p_turbulent,
   // turbulence params
   p_turbulence_offset,
   p_turbulence_scale,
   
   p_ridged,
   // ridge params
   p_ridge_offset,
   p_ridge_gain,
   p_ridge_exponent,
   
   p_dampen_output,
   
   p_remap_output,
   p_fractal_min,
   p_fractal_max,
   p_output_min,
   p_output_max,
   p_clamp_output
};

template <typename TNoise, typename TModifier>
void SetupNoise(AtNode *, AtShaderGlobals *, fBm<TNoise, TModifier> &)
{
}
template <typename TModifier>
void SetupNoise(AtNode *node, AtShaderGlobals *sg, fBm<ValueNoise, TModifier> &fbm)
{
   fbm.noise_params.seed = AiShaderEvalParamInt(p_value_seed);
   fbm.noise_params.quality = (NoiseQuality) AiShaderEvalParamInt(p_value_quality);
}
template <typename TModifier>
void SetupNoise(AtNode *node, AtShaderGlobals *sg, fBm<PerlinNoise, TModifier> &fbm)
{
   fbm.noise_params.seed = AiShaderEvalParamInt(p_perlin_seed);
   fbm.noise_params.quality = (NoiseQuality) AiShaderEvalParamInt(p_perlin_quality);
}
template <typename TModifier>
void SetupNoise(AtNode *node, AtShaderGlobals *sg, fBm<FlowNoise, TModifier> &fbm)
{
   fbm.noise_params.t = AiShaderEvalParamFlt(p_flow_time);
   fbm.noise_params.power = AiShaderEvalParamFlt(p_flow_power);
}

template <typename TNoise, typename TModifier>
void SetupModifier(AtNode *, AtShaderGlobals *, fBm<TNoise, TModifier> &)
{
}
template <typename TNoise>
void SetupModifier(AtNode *node, AtShaderGlobals *sg, fBm<TNoise, TurbulenceModifier> &fbm)
{
   fbm.modifier_params.offset = AiShaderEvalParamFlt(p_turbulence_offset);
   fbm.modifier_params.scale = AiShaderEvalParamFlt(p_turbulence_scale);
}
template <typename TNoise>
void SetupModifier(AtNode *node, AtShaderGlobals *sg, fBm<TNoise, RidgeModifier> &fbm)
{
   fbm.modifier_params.offset = AiShaderEvalParamFlt(p_ridge_offset);
   fbm.modifier_params.gain = AiShaderEvalParamFlt(p_ridge_gain);
   fbm.modifier_params.exponent = AiShaderEvalParamFlt(p_ridge_exponent);
}
template <typename TNoise>
void SetupModifier(AtNode *node, AtShaderGlobals *sg, fBm<TNoise, CombineModifier<TurbulenceModifier, RidgeModifier> > &fbm)
{
   fbm.modifier_params.mod1.offset = AiShaderEvalParamFlt(p_turbulence_offset);
   fbm.modifier_params.mod1.scale = AiShaderEvalParamFlt(p_turbulence_scale);
   fbm.modifier_params.mod2.offset = AiShaderEvalParamFlt(p_ridge_offset);
   fbm.modifier_params.mod2.gain = AiShaderEvalParamFlt(p_ridge_gain);
   fbm.modifier_params.mod2.exponent = AiShaderEvalParamFlt(p_ridge_exponent);
}

template <typename TNoise>
float EvalNoise(AtNode *node, AtShaderGlobals *sg, const AtPoint &P)
{
   float amplitude = AiShaderEvalParamFlt(p_amplitude);
   float frequency = AiShaderEvalParamFlt(p_frequency);
   int octaves = AiShaderEvalParamInt(p_octaves);
   float persistence = AiShaderEvalParamFlt(p_persistence);
   float lacunarity = AiShaderEvalParamFlt(p_lacunarity);
   bool turbulent = AiShaderEvalParamBool(p_turbulent);
   bool ridged = AiShaderEvalParamBool(p_ridged);
   bool damp = AiShaderEvalParamBool(p_dampen_output);
   bool remap_output = AiShaderEvalParamBool(p_remap_output);
   
   float out = 0.0f;
   
   if (turbulent)
   {
      if (ridged)
      {
         fBm<TNoise, CombineModifier<TurbulenceModifier, RidgeModifier> > fbm(octaves, amplitude, persistence, frequency, lacunarity);
         SetupNoise(node, sg, fbm);
         SetupModifier(node, sg, fbm);
         out = fbm.eval(P, damp);
      }
      else
      {
         fBm<TNoise, TurbulenceModifier> fbm(octaves, amplitude, persistence, frequency, lacunarity);
         SetupNoise(node, sg, fbm);
         SetupModifier(node, sg, fbm);
         out = fbm.eval(P, damp);
      }
   }
   else
   {
      if (ridged)
      {
         fBm<TNoise, RidgeModifier> fbm(octaves, amplitude, persistence, frequency, lacunarity);
         SetupNoise(node, sg, fbm);
         SetupModifier(node, sg, fbm);
         out = fbm.eval(P, damp);
      }
      else
      {
         fBm<TNoise, DefaultModifier> fbm(octaves, amplitude, persistence, frequency, lacunarity);
         SetupNoise(node, sg, fbm);
         SetupModifier(node, sg, fbm);
         out = fbm.eval(P, damp);
      }
   }
   
   if (remap_output)
   {
      float fractal_min = AiShaderEvalParamFlt(p_fractal_min);
      float fractal_max = AiShaderEvalParamFlt(p_fractal_max);
      float output_min = AiShaderEvalParamFlt(p_output_min);
      float output_max = AiShaderEvalParamFlt(p_output_max);
      bool clamp_output = AiShaderEvalParamBool(p_clamp_output);
      
      out = output_min + (output_max - output_min) * (out - fractal_min) / (fractal_max - fractal_min);
      
      if (clamp_output)
      {
         out = CLAMP(out, output_min, output_max);
      }
   }
   
   return out;
}

namespace SSTR
{
   extern AtString input;
   extern AtString custom_input;
   extern AtString base_noise;
   extern AtString linkable;
}

node_parameters
{
   AiParameterEnum(SSTR::input, I_P, InputNames);
   AiParameterPnt(SSTR::custom_input, 0.0f, 0.0f, 0.0f);
   
   AiParameterFlt("amplitude", 1.0f);
   AiParameterFlt("frequency", 1.0f);
   AiParameterInt("octaves", 6);
   AiParameterFlt("persistence", 0.5f);
   AiParameterFlt("lacunarity", 2.0f);
   AiParameterEnum(SSTR::base_noise, NT_simplex, NoiseTypeNames);
   AiParameterInt("value_seed", 0);
   AiParameterEnum("value_quality", NQ_std, NoiseQualityNames);
   AiParameterInt("perlin_seed", 0);
   AiParameterEnum("perlin_quality", NQ_std, NoiseQualityNames);
   AiParameterFlt("flow_power", 0.25f);
   AiParameterFlt("flow_time", 0.0f);
   AiParameterBool("turbulent", false);
   AiParameterFlt("turbulence_offset", -0.5f);
   AiParameterFlt("turbulence_scale", 2.0f);
   AiParameterBool("ridged", false);
   AiParameterFlt("ridge_offset", 1.0f);
   AiParameterFlt("ridge_gain", 2.0f);
   AiParameterFlt("ridge_exponent", 0.0f);
   AiParameterBool("dampen_output", true);
   AiParameterBool("remap_output", true);
   AiParameterFlt("fractal_min", -1.0f);
   AiParameterFlt("fractal_max", 1.0f);
   AiParameterFlt("output_min", 0.0f);
   AiParameterFlt("output_max", 1.0f);
   AiParameterBool("clamp_output", true);
   
   // Common metadata
   AiMetaDataSetStr(mds, NULL, "desc", "Fractal Noise");
   AiMetaDataSetBool(mds, SSTR::input, SSTR::linkable, false);
   AiMetaDataSetBool(mds, SSTR::base_noise, SSTR::linkable, false);
   AiMetaDataSetInt(mds, "octaves", "min", 1);
   AiMetaDataSetInt(mds, "octaves", "softmax", 10);
   AiMetaDataSetFlt(mds, "amplitude", "min", 0.0f);
   AiMetaDataSetFlt(mds, "amplitude", "softmax", 5.0f);
   AiMetaDataSetFlt(mds, "frequency", "min", 0.0f);
   AiMetaDataSetFlt(mds, "frequency", "softmax", 5.0f);
   AiMetaDataSetFlt(mds, "persistence", "min", 0.0f);
   AiMetaDataSetFlt(mds, "persistence", "softmax", 1.0f);
   AiMetaDataSetFlt(mds, "lacunarity", "min", 0.0f);
   AiMetaDataSetFlt(mds, "lacunarity", "softmax", 5.0f);
   AiMetaDataSetInt(mds, "value_seed", "softmin", 0);
   AiMetaDataSetInt(mds, "value_seed", "softmax", 10);
   AiMetaDataSetBool(mds, "value_quality", SSTR::linkable, false);
   AiMetaDataSetInt(mds, "perlin_seed", "softmin", 0);
   AiMetaDataSetInt(mds, "perlin_seed", "softmax", 10);
   AiMetaDataSetBool(mds, "perlin_quality", SSTR::linkable, false);
   AiMetaDataSetFlt(mds, "flow_power", "softmin", 0.0f);
   AiMetaDataSetFlt(mds, "flow_power", "softmax", 1.0f);
   AiMetaDataSetFlt(mds, "flow_time", "softmin", 0.0f);
   AiMetaDataSetFlt(mds, "flow_time", "softmax", 10.0f);
   AiMetaDataSetFlt(mds, "turbulence_offset", "softmin", -1.0f);
   AiMetaDataSetFlt(mds, "turbulence_offset", "softmax", 1.0f);
   AiMetaDataSetFlt(mds, "turbulence_scale", "softmin", -2.0f);
   AiMetaDataSetFlt(mds, "turbulence_scale", "softmax", 2.0f);
   AiMetaDataSetFlt(mds, "ridge_offset", "softmin", 0.0f);
   AiMetaDataSetFlt(mds, "ridge_offset", "softmax", 1.0f);
   AiMetaDataSetFlt(mds, "ridge_gain", "softmin", 0.0f);
   AiMetaDataSetFlt(mds, "ridge_gain", "softmax", 5.0f);
   AiMetaDataSetFlt(mds, "ridge_exponent", "softmin", 0.0f);
   AiMetaDataSetFlt(mds, "ridge_exponent", "softmax", 5.0f);
   AiMetaDataSetFlt(mds, "fractal_min", "softmin", -1.0f);
   AiMetaDataSetFlt(mds, "fractal_min", "softmax", 1.0f);
   AiMetaDataSetFlt(mds, "fractal_max", "softmin", -1.0f);
   AiMetaDataSetFlt(mds, "fractal_max", "softmax", 1.0f);
   AiMetaDataSetFlt(mds, "output_min", "softmin", 0.0f);
   AiMetaDataSetFlt(mds, "output_min", "softmax", 1.0f);
   AiMetaDataSetFlt(mds, "output_max", "softmin", 0.0f);
   AiMetaDataSetFlt(mds, "output_max", "softmax", 1.0f);
   
   // Houdini specifics
   AiMetaDataSetStr(mds, "turbulence_offset", "houdini.disable_when", "{ turbulent == 0 }");
   AiMetaDataSetStr(mds, "turbulence_scale", "houdini.disable_when", "{ turbulent == 0 }");
   AiMetaDataSetStr(mds, "ridge_offset", "houdini.disable_when", "{ ridged == 0 }");
   AiMetaDataSetStr(mds, "ridge_gain", "houdini.disable_when", "{ ridged == 0 }");
   AiMetaDataSetStr(mds, "ridge_exponent", "houdini.disable_when", "{ ridged == 0 }");
   AiMetaDataSetStr(mds, "value_seed", "houdini.hide_when", "{ base_noise != value }");
   AiMetaDataSetStr(mds, "value_quality", "houdini.hide_when", "{ base_noise != value }");
   AiMetaDataSetStr(mds, "perlin_seed", "houdini.hide_when", "{ base_noise != perlin }");
   AiMetaDataSetStr(mds, "perlin_quality", "houdini.hide_when", "{ base_noise != perlin }");
   AiMetaDataSetStr(mds, "flow_power", "houdini.hide_when", "{ base_noise != flow }");
   AiMetaDataSetStr(mds, "flow_time", "houdini.hide_when", "{ base_noise != flow }");
   AiMetaDataSetStr(mds, "fractal_min", "houdini.disable_when", "{ remap_output == 0 }");
   AiMetaDataSetStr(mds, "fractal_max", "houdini.disable_when", "{ remap_output == 0 }");
   AiMetaDataSetStr(mds, "output_min", "houdini.disable_when", "{ remap_output == 0 }");
   AiMetaDataSetStr(mds, "output_max", "houdini.disable_when", "{ remap_output == 0 }");
   AiMetaDataSetStr(mds, "clamp_output", "houdini.disable_when", "{ remap_output == 0 }");
}

struct NodeData
{
   Input input;
   bool evalCustomInput;
   NoiseType type;
};

node_initialize
{
   AiNodeSetLocalData(node, new NodeData());
}

node_update
{
   NodeData *data = (NodeData*) AiNodeGetLocalData(node);

   data->input = (Input) AiNodeGetInt(node, SSTR::input);
   data->evalCustomInput = AiNodeIsLinked(node, SSTR::custom_input);
   data->type = (NoiseType) AiNodeGetInt(node, SSTR::base_noise);
}

node_finish
{
   NodeData *data = (NodeData*) AiNodeGetLocalData(node);
   delete data;
}

shader_evaluate
{
   NodeData *data = (NodeData*) AiNodeGetLocalData(node);
   
   AtPoint P;
   if (data->evalCustomInput)
   {
      P = AiShaderEvalParamPnt(p_custom_input);
   }
   else
   {
      P = GetInput(data->input, sg, node);
   }
   
   switch (data->type)
   {
   case NT_value:
      sg->out.FLT = EvalNoise<ValueNoise>(node, sg, P);
      break;
   case NT_perlin:
      sg->out.FLT = EvalNoise<PerlinNoise>(node, sg, P);
      break;
   case NT_flow:
      sg->out.FLT = EvalNoise<FlowNoise>(node, sg, P);
      break;
   case NT_simplex:
   default:
      sg->out.FLT = EvalNoise<SimplexNoise>(node, sg, P);
      break;
   }
}


