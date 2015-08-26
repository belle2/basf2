# For safety since we encountered a hanging process
# when this file is executed with basf2
import basf2
import ROOT
from ROOT import gSystem
gSystem.Load('libtracking_trackFindingCDC')

from ROOT import Belle2  # make Belle2 namespace available

wire = Belle2.TrackFindingCDC.CDCWire()

Vector2D = Belle2.TrackFindingCDC.Vector2D
CDCDerivedGenHit = Belle2.TrackFindingCDC.CDCDerivedGenHit

derivedGenHit = CDCDerivedGenHit(wire, Vector2D())

# CDCDerivedGenHit has all methods of its restrained base accessable with ->.
pos = derivedGenHit.getDummyPos2D()

# Also all of its own methods are accessable.
derivedGenHit.setDummyPos2D(pos)
derivedGenHit.setFlag(True)
print derivedGenHit.getFlag()
