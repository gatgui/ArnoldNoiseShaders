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

namespace SSTR
{
   extern AtString Pref;
}

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

AtVector GetInput(Input which, AtShaderGlobals *sg, AtNode *)
{
   AtVector P;
   
   switch (which)
   {
   case I_P:
      P = sg->P;
      break;
   case I_Po:
      P = sg->Po;
      break;
   case I_Pref:
      if (!AiUDataGetVec(SSTR::Pref, P))
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
