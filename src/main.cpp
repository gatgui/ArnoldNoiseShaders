#include <ai.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>

const char* NoiseQualityNames[] = 
{
   "fast",
   "standard",
   "best",
   NULL
};

extern AtNodeMethods *PerlinMtd;
extern AtNodeMethods *BillowMtd;

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
   default:
      return false;
   }
}
