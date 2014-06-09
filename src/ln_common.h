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

// redesigned fractale module
// NoiseBase is a class with 2 operator() defined taking 3 floats or a single float
// first variant is called per octave, second to adjust final value
// the class must have a constructor that takes 2 parameters: the noise seed and quality
struct ValueGenerator
{
   inline float value(float x, float y, float z, int seed, NoiseQuality)
   {
      return noise::ValueNoise3D(x, y, z, seed);
   }
};

struct PerlinGenerator
{
   inline float value(float x, float y, float z, int seed, NoiseQuality q)
   {
      return noise::GradientCoherentNoise3D(x, y, z, seed, (noise::NoiseQuality)q);
   }
};

struct fBm_Params
{
   int octaves;
   float amplitude;
   float persistence;
   float frequency;
   float lacunarity;
};

struct PassthroughModifier
{
   typedef void* Params;
   
   inline void prepare(const fBm_Params &, const Params &)
   {
   }
   
   inline float apply(float noise_value, int, float amplitude, float) const
   {
      return amplitude * noise_value;
   }
   
   inline float adjust(float noise_value) const
   {
      return noise_value;
   }
   
   inline void cleanup()
   {
   }
};

struct AbsoluteModifier
{
   typedef void* Params;
   
   inline void prepare(const fBm_Params &, const Params &)
   {
   }
   
   inline float apply(float noise_value, int, float amplitude, float) const
   {
      return amplitude * (2.0f * fabsf(noise_value) - 1.0f);
   }
   
   inline float adjust(float noise_value) const
   {
      return noise_value + 0.5f;
   }
   
   inline void cleanup()
   {
   }
};

struct RidgeModifier
{
   struct Params
   {
      float offset;
      float gain;
      float exponent;
   };
   
   Params _params;
   float _base_amplitude;
   mutable float _cur_weight;
   float *_octave_weights;
   
   inline RidgeModifier()
      : _base_amplitude(0.0f), _cur_weight(1.0f), _octave_weights(0)
   {
   }
   
   void prepare(const fBm_Params &fbm_params, const Params &params)
   {
      _base_amplitude = fbm_params.amplitude;
      
      _params = params;
      _cur_weight = 1.0f;
      
      float f = fbm_params.frequency;
      
      /*
      _octave_weights = new float[fbm_params.octaves];
      for (int i=0; i<fbm_params.octaves; ++i)
      {
         _octave_weights[i] = powf(f, -params.exponent);
         f *= fbm_params.lacunarity;
      }
      */
   }
   
   float apply(float noise_value, int octave, float amplitude, float frequency) const
   {
      // Do not use incoming amplitude as it will change with octaves because of persistence
      float s = _params.offset - _base_amplitude * fabsf(noise_value);
      
      s *= s * _cur_weight;
      
      // update weight for next octave
      _cur_weight = CLAMP(s * _params.gain, 0.0f, 1.0f);
      
      // apply octave spectral weight
      //return (s * _octave_weights[octave]);
      return (s * powf(frequency, -_params.exponent));
   }
   
   inline float adjust(float noise_value) const
   {
      return 1.25f * noise_value - 1.0f;
   }
   
   inline void cleanup()
   {
      //delete[] _octave_weights;
   }
};

template <typename Generator, typename Modifier>
class fBm
{
private:
   int _seed;
   NoiseQuality _quality;
   Generator _noise;
   Modifier _mod;
   fBm_Params _params;
   typename Modifier::Params _mod_params;

public:
   fBm(int octaves, float amplitude, float persistence, float frequency, float lacunarity,
       int seed=0, NoiseQuality quality=NQ_std)
      : _seed(seed), _quality(quality)
   {
      _params.octaves = octaves;
      _params.amplitude = amplitude;
      _params.persistence = persistence;
      _params.frequency = frequency;
      _params.lacunarity = lacunarity;
   }
   
   inline typename Modifier::Params& extra_params() { return _mod_params; }
   
   float eval(const AtPoint &inP)
   {
      float out = 0.0f;
      float curPersistence = _params.amplitude;
      float curFrequency = _params.frequency;
      AtPoint P = inP * _params.frequency;
      int seed = _seed;
      
      _mod.prepare(_params, _mod_params);
      
      for (int curOctave=0; curOctave<_params.octaves; ++curOctave)
      {
         // Make sure that these floating-point values have the same range as a 32-
         // bit integer so that we can pass them to the coherent-noise functions.
         float nx = noise::MakeInt32Range(P.x);
         float ny = noise::MakeInt32Range(P.y);
         float nz = noise::MakeInt32Range(P.z);
         
         // Get the coherent-noise value from the input value and add it to the final result.
         seed = (seed + curOctave) & 0xffffffff;
         out += _mod.apply(_noise.value(nx, ny, nz, seed, _quality), curOctave, curPersistence, curFrequency);
         
         // Prepare the next octave.
         curFrequency *= _params.lacunarity;
         curPersistence *= _params.persistence;
         
         P *= _params.lacunarity;
      }
      
      out = _mod.adjust(out);
      
      _mod.cleanup();
      
      return out;
   }
};

/*
fBm<PerlinGenerator, RidgeModifier> fbm(8, 1.0f, 0.5f, 1.0f, 2.0f);

fbm.extra_params().exponent = 1.0f;
fbm.extra_params().gain = 2.0f;
fbm.extra_params().offset = 1.0f;

return fbm.eval(P);
*/

#endif
