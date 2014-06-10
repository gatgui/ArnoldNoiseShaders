#include "ln_common.h"

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

node_parameters
{
   AiParameterEnum("input", I_P, InputNames);
   AiParameterPnt("custom_input", 0.0f, 0.0f, 0.0f);
   AiParameterFlt("displacement", 0.5f);
   AiParameterFlt("frequency", 1.0f);
   AiParameterEnum("distance_func", DF_euclidian, DistanceFuncNames);
   AiParameterEnum("output_mode", OM_constant, OutputModeNames);
   AiParameterFlt("weight1", -1.0f);
   AiParameterFlt("weight2", 1.0f);
   AiParameterFlt("weight3", 0.0f);
   AiParameterFlt("weight4", 0.0f);
   AiParameterInt("seed", 0);
   
   AiMetaDataSetBool(mds, "input", "linkable", false);
   AiMetaDataSetBool(mds, "distance_func", "linkable", false);
   AiMetaDataSetBool(mds, "output_mode", "linkable", false);
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
   
   float displacement = AiShaderEvalParamFlt(p_displacement);
   float frequency = AiShaderEvalParamFlt(p_frequency);
   DistanceFunc distance_func = (DistanceFunc) AiShaderEvalParamInt(p_distance_func);
   OutputMode output_mode = (OutputMode) AiShaderEvalParamInt(p_output_mode);
   int seed = AiShaderEvalParamInt(p_seed);
   
   float (*eval_dist)(const AtPoint&, const AtPoint&);
   
   switch (distance_func)
   {
   case DF_manhattan:
      eval_dist = &ManhattanDistance;
      break;
   case DF_chebyshev:
      eval_dist = &ChebyshevDistance;
      break;
   case DF_euclidian:
   default:
      eval_dist = &EuclidianDistance;
   }
   
   P *= frequency;
   
   int x_base = int(floorf(P.x));
   int y_base = int(floorf(P.y));
   int z_base = int(floorf(P.z));
   
   AtPoint Pf[4];
   float f[4] = {2147483647.0f, 2147483647.0f, 2147483647.0f, 2147483647.0f};
   AtPoint P_cur;
   
   // Inside each unit cube, there is a seed point at a random position.  Go
   // through each of the nearby cubes until we find a cube with a seed point
   // that is closest to the specified position.
   for (int z_cur=z_base-2; z_cur<=z_base+2; ++z_cur)
   {
      for (int y_cur=y_base-2; y_cur<=y_base+2; ++y_cur)
      {
         for (int x_cur=x_base-2; x_cur<=x_base+2; ++x_cur)
         {
            // Calculate the position and distance to the seed point inside of this unit cube.
            P_cur.x = x_cur + noise::ValueNoise3D(x_cur, y_cur, z_cur, seed);
            P_cur.y = y_cur + noise::ValueNoise3D(x_cur, y_cur, z_cur, seed+1);
            P_cur.z = z_cur + noise::ValueNoise3D(x_cur, y_cur, z_cur, seed+2);
            
            float dist = eval_dist(P, P_cur);

            if (dist < f[0])
            {
               Pf[3] = Pf[2];
               f[3] = f[2];
               
               Pf[2] = Pf[1];
               f[2] = f[1];
               
               Pf[1] = Pf[0];
               f[1] = f[0];
               
               Pf[0] = P_cur;
               f[0] = dist;
            }
            else if (dist < f[1])
            {
               Pf[3] = Pf[2];
               f[3] = f[2];
               
               Pf[2] = Pf[1];
               f[2] = f[1];
               
               Pf[1] = P_cur;
               f[1] = dist;
            }
            else if (dist < f[2])
            {
               Pf[3] = Pf[2];
               f[3] = f[2];
               
               Pf[2] = P_cur;
               f[2] = dist;
            }
            else if (dist < f[3])
            {
               Pf[3] = P_cur;
               f[3] = dist;
            }
         }
      }
   }
   
   switch (output_mode)
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
