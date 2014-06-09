#include <ai.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>

extern AtNodeMethods *PerlinMtd;
extern AtNodeMethods *BillowMtd;
extern AtNodeMethods *TurbulenceMtd;
extern AtNodeMethods *VoronoiMtd;
extern AtNodeMethods *RidgedMtd;

node_loader
{
   switch (i)
   {
   case 0:
      node->name = "ln_perlin";
      node->node_type = AI_NODE_SHADER;
      node->output_type = AI_TYPE_FLOAT;
      node->methods = PerlinMtd;
      strcpy(node->version, AI_VERSION);
      return true;
   case 1:
      node->name = "ln_billow";
      node->node_type = AI_NODE_SHADER;
      node->output_type = AI_TYPE_FLOAT;
      node->methods = BillowMtd;
      strcpy(node->version, AI_VERSION);
      return true;
   case 2:
      node->name = "ln_turbulence";
      node->node_type = AI_NODE_SHADER;
      node->output_type = AI_TYPE_POINT;
      node->methods = TurbulenceMtd;
      strcpy(node->version, AI_VERSION);
      return true;
   case 3:
      node->name = "ln_voronoi";
      node->node_type = AI_NODE_SHADER;
      node->output_type = AI_TYPE_FLOAT;
      node->methods = VoronoiMtd;
      strcpy(node->version, AI_VERSION);
      return true;
   case 4:
      node->name = "ln_ridged";
      node->node_type = AI_NODE_SHADER;
      node->output_type = AI_TYPE_FLOAT;
      node->methods = RidgedMtd;
      strcpy(node->version, AI_VERSION);
      return true;
   default:
      return false;
   }
}
