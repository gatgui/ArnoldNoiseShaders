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

AI_SHADER_NODE_EXPORT_METHODS(VoronoiMtd);

enum VoronoiParams
{
   p_input = 0,
   p_custom_input,
   p_displacement,
   p_frequency,
   p_distance_func,
   p_output_mode,
   p_weight1,
   p_weight2,
   p_weight3,
   p_weight4,
   p_seed
};

enum DistanceFunc
{
   DF_euclidian = 0,
   DF_manhattan,
   DF_chebyshev
};

static const char *DistanceFuncNames[] =
{
   "euclidian",
   "manhattan",
   "chebyshev",
   NULL
};

enum OutputMode
{
   OM_constant = 0,
   OM_f1,
   OM_f2,
   OM_f3,
   OM_f4,
   OM_add,
   OM_sub,
   OM_mul,
   OM_weighted
};

static const char *OutputModeNames[] =
{
   "constant",
   "f1",
   "f2",
   "f3",
   "f4",
   "f1+f2",
   "f2-f1",
   "f1*f2",
   "weighted",
   NULL
};

float ManhattanDistance(const AtPoint &p1, const AtPoint &p2)
{
   return fabsf(p1.x - p2.x) + fabsf(p1.y - p2.y) + fabsf(p1.z - p2.z);
}

float EuclidianDistance(const AtPoint &p1, const AtPoint &p2)
{
   return AiV3Length(p1 - p2);
}

float ChebyshevDistance(const AtPoint &p1, const AtPoint &p2)
{
   AtVector diff = p1 - p2;
   return std::max(std::max(fabsf(diff.x), fabsf(diff.y)), fabsf(diff.z));
}

namespace SSTR
{
   extern AtString linkable;
   extern AtString input;
   extern AtString distance_func;
   extern AtString output_mode;
   extern AtString custom_input;
}

node_parameters
{
   AiParameterEnum(SSTR::input, I_P, InputNames);
   AiParameterPnt(SSTR::custom_input, 0.0f, 0.0f, 0.0f);
   AiParameterFlt("displacement", 0.5f);
   AiParameterFlt("frequency", 1.0f);
   AiParameterEnum(SSTR::distance_func, DF_euclidian, DistanceFuncNames);
   AiParameterEnum(SSTR::output_mode, OM_constant, OutputModeNames);
   AiParameterFlt("weight1", -1.0f);
   AiParameterFlt("weight2", 1.0f);
   AiParameterFlt("weight3", 0.0f);
   AiParameterFlt("weight4", 0.0f);
   AiParameterInt("seed", 0);
}

struct NodeData
{
   Input input;
   bool evalCustomInput;
   DistanceFunc distanceFunc;
   OutputMode outputMode;
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
   data->distanceFunc = (DistanceFunc) AiNodeGetInt(node, SSTR::distance_func);
   data->outputMode = (OutputMode) AiNodeGetInt(node, SSTR::output_mode);
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
   
   float displacement = AiShaderEvalParamFlt(p_displacement);
   float frequency = AiShaderEvalParamFlt(p_frequency);
   int seed = AiShaderEvalParamInt(p_seed);
   
   float (*evalDist)(const AtPoint&, const AtPoint&);
   
   switch (data->distanceFunc)
   {
   case DF_manhattan:
      evalDist = &ManhattanDistance;
      break;
   case DF_chebyshev:
      evalDist = &ChebyshevDistance;
      break;
   case DF_euclidian:
   default:
      evalDist = &EuclidianDistance;
   }
   
   P *= frequency;
   
   int xbase = int(floorf(P.x));
   int ybase = int(floorf(P.y));
   int zbase = int(floorf(P.z));
   
   AtPoint Pf[4] = {P, P, P, P};
   float f[4] = {2147483647.0f, 2147483647.0f, 2147483647.0f, 2147483647.0f};
   AtPoint Pcur;
   
   // Inside each unit cube, there is a seed point at a random position.  Go
   // through each of the nearby cubes until we find a cube with a seed point
   // that is closest to the specified position.
   for (int zcur=zbase-2; zcur<=zbase+2; ++zcur)
   {
      for (int ycur=ybase-2; ycur<=ybase+2; ++ycur)
      {
         for (int xcur=xbase-2; xcur<=xbase+2; ++xcur)
         {
            // Calculate the position and distance to the seed point inside of this unit cube.
            Pcur.x = xcur + noise::ValueNoise3D(xcur, ycur, zcur, seed);
            Pcur.y = ycur + noise::ValueNoise3D(xcur, ycur, zcur, seed+1);
            Pcur.z = zcur + noise::ValueNoise3D(xcur, ycur, zcur, seed+2);
            
            float dist = evalDist(P, Pcur);

            if (dist < f[0])
            {
               Pf[3] = Pf[2];
               f[3] = f[2];
               
               Pf[2] = Pf[1];
               f[2] = f[1];
               
               Pf[1] = Pf[0];
               f[1] = f[0];
               
               Pf[0] = Pcur;
               f[0] = dist;
            }
            else if (dist < f[1])
            {
               Pf[3] = Pf[2];
               f[3] = f[2];
               
               Pf[2] = Pf[1];
               f[2] = f[1];
               
               Pf[1] = Pcur;
               f[1] = dist;
            }
            else if (dist < f[2])
            {
               Pf[3] = Pf[2];
               f[3] = f[2];
               
               Pf[2] = Pcur;
               f[2] = dist;
            }
            else if (dist < f[3])
            {
               Pf[3] = Pcur;
               f[3] = dist;
            }
         }
      }
   }
   
   switch (data->outputMode)
   {
   case OM_constant:
      sg->out.FLT = displacement * 0.5f * (1.0f + noise::ValueNoise3D(int(floorf(Pf[0].x)), int(floorf(Pf[0].y)), int(floorf(Pf[0].z))));
      break;
   case OM_f1:
      sg->out.FLT = displacement * f[0];
      break;
   case OM_f2:
      sg->out.FLT = displacement * f[1];
      break;
   case OM_f3:
      sg->out.FLT = displacement * f[2];
      break;
   case OM_f4:
      sg->out.FLT = displacement * f[3];
      break;
   case OM_add:
      sg->out.FLT = displacement * (f[0] + f[1]);
      break;
   case OM_sub:
      sg->out.FLT = displacement * (f[1] - f[0]);
      break;
   case OM_mul:
      sg->out.FLT = displacement * (f[0] * f[1]);
      break;
   case OM_weighted:
      {
         float w1 = AiShaderEvalParamFlt(p_weight1);
         float w2 = AiShaderEvalParamFlt(p_weight2);
         float w3 = AiShaderEvalParamFlt(p_weight3);
         float w4 = AiShaderEvalParamFlt(p_weight4);
         sg->out.FLT = displacement * (w1 * f[0] + w2 * f[1] + w3 * f[2] + w4 * f[3]);
      }
      break;
   default:
      sg->out.FLT = 0.0f;
      break;
   }
}
