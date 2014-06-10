import sys
import glob
import excons
from excons.tools import arnold

env = excons.MakeBaseEnv()

prjs = [
  {"name": "agNoises",
   "type": "dynamicmodule",
   "ext": arnold.PluginExt(),
   "srcs": glob.glob("src/*.cpp") + glob.glob("src/libnoise/*.cpp") + glob.glob("src/stegu/*.cpp"),
   "custom": [arnold.Require]
  }
]

excons.DeclareTargets(env, prjs)
