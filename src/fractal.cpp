/*
MIT License

Copyright (c) 2016 Gaetan Guidet

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "common.h"

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


