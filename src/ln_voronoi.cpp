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
   OM_noise = 0,
   OM_dist1,
   OM_dist2,
   OM_dist_add,
   OM_dist_sub,
   OM_dist_mul,
   OM_dist_div,
   OM_dist_avg
};

static const char *OutputModeNames[] =
{
   "noise",
   "dist1",
   "dist2",
   "dist_add",
   "dist_sub",
   "dist_mul",
   "dist_div",
   "dist_avg",
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
   AiParameterEnum("output_mode", OM_noise, OutputModeNames);
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
   
   AtPoint P = (is_input_linked ? AiShaderEvalParamPnt(p_custom_input) : GetInput(input, sg, node));
   
   P *= frequency;
   
   int x_base = int(floorf(P.x));
   int y_base = int(floorf(P.y));
   int z_base = int(floorf(P.z));
   
   AtPoint P_first = P;
   AtPoint P_second = P;
   AtPoint P_cur;
   float first_dist = 2147483647.0f;
   float second_dist = 2147483647.0f;
   
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

            if (dist < first_dist)
            {
               second_dist = first_dist;
               P_second = P_first;
               
               first_dist = dist;
               P_first = P_cur;
            }
            else if (dist < second_dist)
            {
               second_dist = dist;
               P_second = P_cur;
            }
         }
      }
   }
   
   switch (output_mode)
   {
   case OM_noise:
      sg->out.FLT = displacement * 0.5f * (1.0f + noise::ValueNoise3D(int(floorf(P_first.x)), int(floorf(P_first.y)), int(floorf(P_first.z))));
      break;
   case OM_dist1:
      sg->out.FLT = displacement * first_dist;
      break;
   case OM_dist2:
      sg->out.FLT = displacement * second_dist;
      break;
   case OM_dist_add:
      sg->out.FLT = displacement * (second_dist + first_dist);
      break;
   case OM_dist_sub:
      sg->out.FLT = displacement * (second_dist - first_dist);
      break;
   case OM_dist_mul:
      sg->out.FLT = displacement * (first_dist * second_dist);
      break;
   case OM_dist_div:
      sg->out.FLT = displacement * (first_dist / second_dist);
      break;
   case OM_dist_avg:
      sg->out.FLT = displacement * 0.5f * (first_dist + second_dist);
      break;
   default:
      sg->out.FLT = 0.0f;
      break;
   }
}
