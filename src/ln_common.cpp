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

float Fractal(const AtPoint &inP, int octaves, float amplitude, float persistence, float frequency, float lacunarity, int seed, NoiseQuality quality)
{
   float out = 0.0f;
   
   float curPersistence = amplitude;
   float nx, ny, nz;
   
   AtPoint P = inP;
   
   P *= frequency;
   
   for (int curOctave=0; curOctave<octaves; ++curOctave)
   {
      // Make sure that these floating-point values have the same range as a 32-
      // bit integer so that we can pass them to the coherent-noise functions.
      nx = noise::MakeInt32Range(P.x);
      ny = noise::MakeInt32Range(P.y);
      nz = noise::MakeInt32Range(P.z);
      
      // Get the coherent-noise value from the input value and add it to the final result.
      seed = (seed + curOctave) & 0xffffffff;
      out += curPersistence * noise::GradientCoherentNoise3D(nx, ny, nz, seed, (noise::NoiseQuality)quality);
      
      // Prepare the next octave.
      P *= lacunarity;
      curPersistence *= persistence;
   }
   
   return out;
}

float AbsFractal(const AtPoint &inP, int octaves, float amplitude, float persistence, float frequency, float lacunarity, int seed, NoiseQuality quality)
{
   float out = 0.0f;
   
   float curPersistence = amplitude;
   float nx, ny, nz;
   
   AtPoint P = inP;
   
   P *= frequency;
   
   for (int curOctave=0; curOctave<octaves; ++curOctave)
   {
      // Make sure that these floating-point values have the same range as a 32-
      // bit integer so that we can pass them to the coherent-noise functions.
      nx = noise::MakeInt32Range(P.x);
      ny = noise::MakeInt32Range(P.y);
      nz = noise::MakeInt32Range(P.z);
      
      // Get the coherent-noise value from the input value and add it to the final result.
      seed = (seed + curOctave) & 0xffffffff;
      out += curPersistence * (2.0f * fabsf(noise::GradientCoherentNoise3D(nx, ny, nz, seed, (noise::NoiseQuality)quality)) - 1.0f);
      
      // Prepare the next octave.
      P *= lacunarity;
      curPersistence *= persistence;
   }
   
   return out + 0.5f;
}    

AtPoint GetInput(Input which, AtShaderGlobals *sg, AtNode *node)
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
