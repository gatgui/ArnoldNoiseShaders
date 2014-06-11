#include "ln_common.h"

const char* NoiseQualityNames[] = 
{
   "fast",
   "standard",
   "best",
   NULL
};

const char* InputNames[] =
{
   "P",
   "Po",
   "Pref",
   "UV",
   NULL
};

const char* NoiseTypeNames[] =
{
   "value",
   "perlin",
   "simplex",
   "flow",
   NULL
};

AtPoint GetInput(Input which, AtShaderGlobals *sg, AtNode *)
{
   AtPoint P;
   
   switch (which)
   {
   case I_P:
      P = sg->P;
      break;
   case I_Po:
      P = sg->Po;
      break;
   case I_Pref:
      if (!AiUDataGetPnt("Pref", &P))
      {
         AiMsgWarning("Pref node defined, defaults to P");
         P = sg->P;
      }
      break;
   case I_UV:
      P.x = sg->u;
      P.y = sg->v;
      P.z = 0.0f;
      break;
   default:
      AiMsgWarning("Unknown input, defaults to P");
      P = sg->P;
      break;
   }
   
   return P;
}
