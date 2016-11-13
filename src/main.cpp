#include <ai.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>

extern AtNodeMethods *DistortPointMtd;
extern AtNodeMethods *VoronoiMtd;
extern AtNodeMethods *FractalMtd;

namespace SSTR
{
   AtString Pref("Pref");
   AtString linkable("linkable");
   AtString input("input");
   AtString distance_func("distance_func");
   AtString output_mode("output_mode");
   AtString custom_input("custom_input");
   AtString base_noise("base_noise");
}

node_loader
{
   switch (i)
   {
   case 0:
      node->name = PREFIX "fractal";
      node->node_type = AI_NODE_SHADER;
      node->output_type = AI_TYPE_FLOAT;
      node->methods = FractalMtd;
      strcpy(node->version, AI_VERSION);
      return true;
   case 1:
      node->name = PREFIX "distort_point";
      node->node_type = AI_NODE_SHADER;
      node->output_type = AI_TYPE_POINT;
      node->methods = DistortPointMtd;
      strcpy(node->version, AI_VERSION);
      return true;
   case 2:
      node->name = PREFIX "voronoi";
      node->node_type = AI_NODE_SHADER;
      node->output_type = AI_TYPE_FLOAT;
      node->methods = VoronoiMtd;
      strcpy(node->version, AI_VERSION);
      return true;
   default:
      return false;
   }
}
