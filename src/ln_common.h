#ifndef __agNoises_quality_h__
#define __agNoises_quality_h__

#include <ai.h>
#include <algorithm>
#include "libnoise/noisegen.h"

enum NoiseQuality
{
   NQ_fast = 0,
   NQ_std,
   NQ_best
};

extern const char* NoiseQualityNames[];


enum Input
{
   I_P = 0,
   I_Po,
   I_Pref,
   I_UV
};

extern const char* InputNames[];


AtPoint GetInput(Input which, AtShaderGlobals *sg, AtNode *node);

float Fractal(const AtPoint &inP, int octaves, float amplitude, float persistence, float frequency, float lacunarity, int seed=0, NoiseQuality quality=NQ_std);
float AbsFractal(const AtPoint &inP, int octaves, float amplitude, float persistence, float frequency, float lacunarity, int seed=0, NoiseQuality quality=NQ_std);

#endif
