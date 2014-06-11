#include <ai.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>

extern AtNodeMethods *TurbulenceMtd;
extern AtNodeMethods *VoronoiMtd;
extern AtNodeMethods *FractalMtd;

node_loader
{
   switch (i)
   {
   case 0:
      node->name = "ln_fractal";
      node->node_type = AI_NODE_SHADER;
      node->output_type = AI_TYPE_FLOAT;
      node->methods = FractalMtd;
      strcpy(node->version, AI_VERSION);
      return true;
   case 1:
      node->name = "ln_turbulence";
      node->node_type = AI_NODE_SHADER;
      node->output_type = AI_TYPE_POINT;
      node->methods = TurbulenceMtd;
      strcpy(node->version, AI_VERSION);
      return true;
   case 2:
      node->name = "ln_voronoi";
      node->node_type = AI_NODE_SHADER;
      node->output_type = AI_TYPE_FLOAT;
      node->methods = VoronoiMtd;
      strcpy(node->version, AI_VERSION);
      return true;
   default:
      return false;
   }
}
