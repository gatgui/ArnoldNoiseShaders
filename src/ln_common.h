#ifndef __agNoises_quality_h__
#define __agNoises_quality_h__

#include <ai.h>
#include <algorithm>
#include "libnoise/noisegen.h"
#include "stegu/simplexnoise1234.h"

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


class fBmBase
{
public:
   
   struct Params
   {
      int octaves;
      float amplitude;
      float persistence;
      float frequency;
      float lacunarity;
   };
   
   struct Context
   {
      int octave;
      float amplitude;
      float frequency;
   };
   
   Params params;
   
   fBmBase(int octaves, float amplitude, float persistence, float frequency, float lacunarity)
   {
      params.octaves = octaves;
      params.amplitude = amplitude;
      params.persistence = persistence;
      params.frequency = frequency;
      params.lacunarity = lacunarity;
   }
   
   ~fBmBase()
   {   
   }
};

template <typename Noise, typename Modifier>
class fBm : public fBmBase
{
private:
   
   Noise _noise;
   Modifier _modifier;
   
public:
   
   typename Noise::Params noise_params;
   typename Modifier::Params modifier_params;

public:
   
   fBm(int octaves, float amplitude, float persistence, float frequency, float lacunarity)
      : fBmBase(octaves, amplitude, persistence, frequency, lacunarity)
   {
   }
   
   float eval(const AtPoint &inP)
   {
      Context ctx;
      
      ctx.amplitude = params.amplitude;
      ctx.frequency = params.frequency;
      ctx.octave = 0;
      
      float out = 0.0f;
      
      AtPoint P = inP * params.frequency;
      
      _noise.prepare(params, noise_params);
      _modifier.prepare(params, modifier_params);
      
      for (; ctx.octave<params.octaves; ctx.octave++)
      {
         // Make sure that these floating-point values have the same range as a 32-
         // bit integer so that we can pass them to the coherent-noise functions.
         float nx = noise::MakeInt32Range(P.x);
         float ny = noise::MakeInt32Range(P.y);
         float nz = noise::MakeInt32Range(P.z);
         
         out += _modifier.apply(ctx, _noise.value(ctx, nx, ny, nz));
         
         // Prepare the next octave.
         ctx.amplitude *= params.persistence;
         ctx.frequency *= params.lacunarity;
         
         P *= params.lacunarity;
      }
      
      out = _modifier.adjust(out);
      
      _modifier.cleanup();
      _noise.cleanup();
      
      return out;
   }
};

struct ValueNoise
{
   struct Params
   {
      int seed;
   };
   
   Params params;
   
   inline void prepare(const fBmBase::Params &, const Params &inParams)
   {
      params = inParams;
   }
   
   inline float value(const fBmBase::Context &ctx, float x, float y, float z)
   {
      params.seed = (params.seed + ctx.octave) & 0xFFFFFFFF;
      return noise::ValueNoise3D(x, y, z, params.seed);
   }
   
   inline void cleanup()
   {
   }
};

struct PerlinNoise
{
   struct Params
   {
      int seed;
      NoiseQuality quality;
   };
   
   Params params;
   
   inline void prepare(const fBmBase::Params &, const Params &inParams)
   {
      params = inParams;
   }
   
   inline float value(const fBmBase::Context &ctx, float x, float y, float z)
   {
      params.seed = (params.seed + ctx.octave) & 0xFFFFFFFF;
      return noise::GradientCoherentNoise3D(x, y, z, params.seed, (noise::NoiseQuality)params.quality);
   }
   
   inline void cleanup()
   {
   }
};

struct SimplexNoise
{
   struct Params
   {
   };
   
   Params params;
   
   inline void prepare(const fBmBase::Params &, const Params &)
   {
   }
   
   inline float value(const fBmBase::Context &, float x, float y, float z)
   {
      return SimplexNoise1234::noise(x, y, z);
   }
   
   inline void cleanup()
   {
   }
};

struct DefaultModifier
{
   struct Params
   {
   };
   
   inline void prepare(const fBmBase::Params &, const Params &)
   {
   }
   
   inline float apply(const fBmBase::Context &ctx, float noise_value) const
   {
      return ctx.amplitude * noise_value;
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
   struct Params
   {
      bool remap_range;
   };
   
   Params _params;
   
   inline void prepare(const fBmBase::Params &, const Params &params)
   {
      _params = params;
   }
   
   inline float apply(const fBmBase::Context &ctx, float noise_value) const
   {
      float out_value = fabsf(noise_value);
      return ctx.amplitude * (_params.remap_range ? (2.0f * out_value - 1.0f) : out_value);
   }
   
   inline float adjust(float noise_value) const
   {
      return (_params.remap_range ? noise_value + 0.5f : noise_value);
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
   mutable float _weight;
   
   inline RidgeModifier()
      : _base_amplitude(0.0f), _weight(1.0f)
   {
   }
   
   void prepare(const fBmBase::Params &fbmParams, const Params &params)
   {
      _params = params;
      _base_amplitude = fbmParams.amplitude;
      _weight = 1.0f;
   }
   
   float apply(const fBmBase::Context &ctx, float noise_value) const
   {
      // Do not use incoming amplitude as it will change with octaves because of persistence
      float s = _params.offset - _base_amplitude * fabsf(noise_value);
      
      s *= s * _weight;
      
      // update weight for next octave
      _weight = CLAMP(s * _params.gain, 0.0f, 1.0f);
      
      // apply octave spectral weight
      return (s * powf(ctx.frequency, -_params.exponent));
   }
   
   inline float adjust(float noise_value) const
   {
      return 1.25f * noise_value - 1.0f;
      //return noise_value;
   }
   
   inline void cleanup()
   {
   }
};

#endif
