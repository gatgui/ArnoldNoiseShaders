#ifndef __agNoises_quality_h__
#define __agNoises_quality_h__

#include <ai.h>
#include <algorithm>
#include "libnoise/noisegen.h"
#include "stegu/simplexnoise1234.h"
#include "stegu/srdnoise23.h"

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


enum NoiseType
{
   NT_value = 0,
   NT_perlin,
   NT_simplex,
   NT_flow
};

extern const char* NoiseTypeNames[];


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
         out += _modifier.apply(ctx, _noise.value(ctx, P.x, P.y, P.z));
         
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
      NoiseQuality quality;
   };
   
   Params _params;
   
   inline void prepare(const fBmBase::Params &, const Params &inParams)
   {
      _params = inParams;
   }
   
   inline float value(const fBmBase::Context &ctx, float x, float y, float z)
   {
      // Make sure that these floating-point values have the same range as a 32-
      // bit integer so that we can pass them to the coherent-noise functions.
      float nx = noise::MakeInt32Range(x);
      float ny = noise::MakeInt32Range(y);
      float nz = noise::MakeInt32Range(z);
      _params.seed = (_params.seed + ctx.octave) & 0xFFFFFFFF;
      return noise::ValueCoherentNoise3D(nx, ny, nz, _params.seed, (noise::NoiseQuality)_params.quality);
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
   
   Params _params;
   
   inline void prepare(const fBmBase::Params &, const Params &inParams)
   {
      _params = inParams;
   }
   
   inline float value(const fBmBase::Context &ctx, float x, float y, float z)
   {
      // Make sure that these floating-point values have the same range as a 32-
      // bit integer so that we can pass them to the coherent-noise functions.
      float nx = noise::MakeInt32Range(x);
      float ny = noise::MakeInt32Range(y);
      float nz = noise::MakeInt32Range(z);
      _params.seed = (_params.seed + ctx.octave) & 0xFFFFFFFF;
      return noise::GradientCoherentNoise3D(nx, ny, nz, _params.seed, (noise::NoiseQuality)_params.quality);
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
   
   Params _params;
   
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

struct FlowNoise
{
   struct Params
   {
      float t;
      float power;
   };
   
   Params _params;
   float _dx;
   float _dy;
   float _dz;
   float _power;
   float _persistence;
   
   inline void prepare(const fBmBase::Params &fbmparams, const Params &params)
   {
      _params = params;
      _power = params.power;
      _persistence = fbmparams.persistence;
      _dx = 0.0f;
      _dy = 0.0f;
      _dz = 0.0f;
   }
   
   inline float value(const fBmBase::Context &ctx, float x, float y, float z)
   {
      // the new derivatives
      float dx = 0.0f;
      float dy = 0.0f;
      float dz = 0.0f;
      
      float rv = srdnoise3(x+_dx, y+_dy, z+_dz, _params.t, &dx, &dy, &dz);
      
      // update derivatives
      _dx += _power * dx;
      _dy += _power * dy;
      _dz += _power * dz;
      _power *= _persistence;
      
      return rv;
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
