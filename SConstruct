import sys
import glob
import excons
import excons.config
from excons.tools import arnold


env = excons.MakeBaseEnv()

arniver = arnold.Version(asString=False)
if arniver[0] < 4 or (arniver[0] == 4 and (arniver[1] < 2 or (arniver[1] == 2 and arniver[2] < 12))):
  print("Arnold 4.2.12.0 or above required")
  sys.exit(1)

def toMayaName(name):
  spl = name.split("_")
  return spl[0] + "".join(map(lambda x: x[0].upper() + x[1:], spl[1:]))

prefix = excons.GetArgument("prefix", "gf_")
name = "%snoise" % prefix
fractal_maya_name = toMayaName(prefix + "fractal")
distort_point_maya_name = toMayaName(prefix + "distort_point")
voronoi_maya_name = toMayaName(prefix + "voronoi")
opts = {"PREFIX": prefix,
        "FRACTAL_MAYA_NODENAME": fractal_maya_name,
        "DISTORT_POINT_MAYA_NODENAME": distort_point_maya_name,
        "VORONOI_MAYA_NODENAME": voronoi_maya_name}

GenerateMtd = excons.config.AddGenerator(env, "mtd", opts)
GenerateMayaAE = excons.config.AddGenerator(env, "mayaAE", opts)
mtd = GenerateMtd("src/%s.mtd" % name, "src/noise.mtd.in")
ae  = GenerateMayaAE("maya/%sTemplate.py" % fractal_maya_name, "maya/FractalTemplate.py.in")
ae += GenerateMayaAE("maya/%sTemplate.py" % distort_point_maya_name, "maya/DistortPointTemplate.py.in")
ae += GenerateMayaAE("maya/%sTemplate.py" % voronoi_maya_name, "maya/VoronoiTemplate.py.in")

if sys.platform != "win32":
   env.Append(CPPFLAGS=" -Wno-unused-parameter")
else:
   env.Append(CPPFLAGS=" /wd4100") # unreferenced format parameter

prjs = [
  {"name": name,
   "type": "dynamicmodule",
   "prefix": "arnold",
   "ext": arnold.PluginExt(),
   "defs": ["PREFIX=\\\"%s\\\"" % prefix],
   "srcs": glob.glob("src/*.cpp") + glob.glob("src/libnoise/*.cpp") + glob.glob("src/stegu/*.cpp"),
   "install": {"arnold": mtd,
               "maya/ae": ae},
   "custom": [arnold.Require]
  }
]

targets = excons.DeclareTargets(env, prjs)

targets[name].extend(mtd)
targets["maya"] = ae

excons.EcosystemDist(env, "noise.env", {name: "", "maya": "/maya/ae"}, name=name)

Default([name])
