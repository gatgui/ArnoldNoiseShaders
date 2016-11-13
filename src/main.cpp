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
