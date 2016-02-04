# Test for TLS issue
# Workaround for the issue in case you're affected
# import ctypes
# ctypes.cdll.LoadLibrary('./lib/Linux_x86_64/opt/libarich_dataobjects.so')

from ROOT import Belle2

from basf2 import *
from modularAnalysis import *

path = create_path()
path.add_module('RootInput')

fillParticleList('mu+', 'muid > 0.5', path=path)
fitVertex('mu+', 0.0, path=path)
