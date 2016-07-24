# Loading some common libraries
import basf2
from ROOT import gSystem
gSystem.Load('libcdc')
gSystem.Load('libtracking')
gSystem.Load('libtracking_trackFindingCDC')
gSystem.Load('libgenfit2')

from ROOT import Belle2  # make Belle2 namespace available
from ROOT import std
