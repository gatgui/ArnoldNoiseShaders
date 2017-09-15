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

#ifndef __noise_common_h__
#define __noise_common_h__

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


AtVector GetInput(Input which, AtShaderGlobals *sg, AtNode *node);


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
   
   float eval(const AtVector &inP, bool dampen=true)
   {
      Context ctx;
      
      ctx.amplitude = params.amplitude;
      ctx.frequency = params.frequency;
      ctx.octave = 0;
      
      float out = 0.0f;
      // use to dampen fractal output
      float tmp = 1.0f;
      float dampfactor = 0.0f;
      
      AtVector P = inP * params.frequency;
      
      _noise.prepare(params, noise_params);
      _modifier.prepare(params, modifier_params);
      
      for (; ctx.octave<params.octaves; ctx.octave++)
      {
         out += ctx.amplitude * _modifier.apply(ctx, _noise.value(ctx, P.x, P.y, P.z));
         
         // Prepare the next octave.
         dampfactor += tmp;
         
         ctx.amplitude *= params.persistence;
         ctx.frequency *= params.lacunarity;
         
         tmp *= params.persistence;
         P *= params.lacunarity;
      }
      
      if (dampen)
      {
         out /= dampfactor;
      }
      
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
   
   inline ValueNoise()
   {
      _params.seed = 0;
      _params.quality = NQ_std;
   }
   
   inline void prepare(const fBmBase::Params &, const Params &inParams)
   {
      _params = inParams;
   }
   
   inline float value(const fBmBase::Context &ctx, float x, float y, float z)
   {
      // Make sure that these floating-point values have the same range as a 32-
      // bit integer so that we can pass them to the coherent-noise functions.
      float nx = float(noise::MakeInt32Range(x));
      float ny = float(noise::MakeInt32Range(y));
      float nz = float(noise::MakeInt32Range(z));
      _params.seed = (_params.seed + ctx.octave) & 0xFFFFFFFF;
      return float(noise::ValueCoherentNoise3D(nx, ny, nz, _params.seed, (noise::NoiseQuality)_params.quality));
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
   
   inline PerlinNoise()
   {
      _params.seed = 0;
      _params.quality = NQ_std;
   }
   
   inline void prepare(const fBmBase::Params &, const Params &inParams)
   {
      _params = inParams;
   }
   
   inline float value(const fBmBase::Context &ctx, float x, float y, float z)
   {
      // Make sure that these floating-point values have the same range as a 32-
      // bit integer so that we can pass them to the coherent-noise functions.
      float nx = float(noise::MakeInt32Range(x));
      float ny = float(noise::MakeInt32Range(y));
      float nz = float(noise::MakeInt32Range(z));
      _params.seed = (_params.seed + ctx.octave) & 0xFFFFFFFF;
      return float(noise::GradientCoherentNoise3D(nx, ny, nz, _params.seed, (noise::NoiseQuality)_params.quality));
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
   
   inline FlowNoise()
      : _dx(0.0f), _dy(0.0f), _dz(0.0f), _power(0.0f), _persistence(1.0f)
   {
      _params.t = 0.0f;
      _params.power = 0.25f;
   }
   
   inline void prepare(const fBmBase::Params &fbmparams, const Params &params)
   {
      _params = params;
      _power = params.power;
      _persistence = fbmparams.persistence;
      _dx = 0.0f;
      _dy = 0.0f;
      _dz = 0.0f;
   }
   
   inline float value(const fBmBase::Context &, float x, float y, float z)
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

template <typename M1, typename M2>
struct CombineModifier
{
   struct Params
   {
      typename M1::Params mod1;
      typename M2::Params mod2;
   };
   
   M1 _mod1;
   M2 _mod2;
   
   inline void prepare(const fBmBase::Params &fbmparams, const Params &params)
   {
      _mod1.prepare(fbmparams, params.mod1);
      _mod2.prepare(fbmparams, params.mod2);
   }
   
   inline float apply(const fBmBase::Context &ctx, float noise_value) const
   {
      return _mod2.apply(ctx, _mod1.apply(ctx, noise_value));
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
   
   inline float apply(const fBmBase::Context &, float noise_value) const
   {
      return noise_value;
   }
   
   inline void cleanup()
   {
   }
};

struct TurbulenceModifier
{
   struct Params
   {
      float offset;
      float scale;
   };
   
   Params _params;
   
   inline void prepare(const fBmBase::Params &, const Params &params)
   {
      _params = params;
   }
   
   inline float apply(const fBmBase::Context &, float noise_value) const
   {
      return (_params.scale * (_params.offset + fabsf(noise_value)));
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
   mutable float _weight;
   
   inline RidgeModifier()
      : _weight(1.0f)
   {
   }
   
   void prepare(const fBmBase::Params &, const Params &params)
   {
      _params = params;
      _weight = 1.0f;
   }
   
   float apply(const fBmBase::Context &ctx, float noise_value) const
   {
      float s = _params.offset - noise_value;
      
      s *= s * _weight;
      
      // update weight for next octave
      _weight = AiClamp(s * _params.gain, 0.0f, 1.0f);
      
      // apply octave spectral weight
      return (s * powf(ctx.frequency, -_params.exponent));
   }
   
   inline void cleanup()
   {
   }
};

#endif
