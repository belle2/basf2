#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2
import math

# ------------------------------------------------------------------------
# A complete simulation and reconstruction of CDC-TOP cosmic tests
# ------------------------------------------------------------------------

# Suppress messages and warnings during processing:
set_log_level(LogLevel.ERROR)

alpha = -45  # rotation angle of trigger counters


class swimMCParticle(Module):

    '''
    Rotate by alpha around z and move generated MC particles backward
    to position outside CDC
    '''

    def event(self):
        '''
        Event processor
        '''

        phi = math.radians(alpha)
        cfi = math.cos(phi)
        sfi = math.sin(phi)

        particles = Belle2.PyStoreArray('MCParticles')
        for particle in particles:
            pos = particle.getVertex()
            x = pos.X() * cfi - pos.Y() * sfi
            y = pos.X() * sfi + pos.Y() * cfi
            pos.SetX(x)
            pos.SetY(y)
            mom = particle.getMomentum()
            px = mom.X() * cfi - mom.Y() * sfi
            py = mom.X() * sfi + mom.Y() * cfi
            mom.SetX(px)
            mom.SetY(py)

            leng = 250
            pos = pos - leng * mom.Unit()
            mass = particle.getMass()
            p = mom.Mag()
            beta = p / math.sqrt(p * p + mass * mass)
            time = particle.getProductionTime() - leng / (Belle2.Const.speedOfLight * beta)
            particle.setProductionVertex(pos)
            particle.setProductionTime(time)
            particle.setMomentum(mom)


def momDistribution(p, katera=0):
    '''
    momentum PDF
    '''
    if p == 0:
        return 0
    if katera == 0:
        b = 2.187907
        c = 0.205373
        d = 0.462988
        x = 3.0 - b / pow(c * p + d, 2)
        y = pow(p, -x)
    else:
        y = pow((5. / p), (1. + 0.04 * (p - 5.)))
    return y

# Construct polyline for ParticleGun describing momentum distribution
katera = 1
n = 500
p = [i / 10.0 for i in range(n)]
y = [momDistribution(p[i], katera) for i in range(n)]
for i in range(n):
    p.append(y[i])
polyline = p


# Create path
main = create_path()

# Number of events to be generated
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [10])
eventinfosetter.param('runList', [1])
eventinfosetter.param('expList', [1])
main.add_module(eventinfosetter)

# Particle gun: generate muons
particlegun = register_module('ParticleGun')
particlegun.param('pdgCodes', [13, -13])
particlegun.param('nTracks', 1)
particlegun.param('varyNTracks', False)
particlegun.param('momentumGeneration', 'polyline')
particlegun.param('momentumParams', polyline)
particlegun.param('thetaGeneration', 'uniformCos')
particlegun.param('thetaParams', [30, 125])
particlegun.param('phiGeneration', 'uniform')
particlegun.param('phiParams', [-100, -80])
particlegun.param('vertexGeneration', 'uniform')
particlegun.param('xVertexParams', [-3, 3])
particlegun.param('yVertexParams', [0, 0])
particlegun.param('zVertexParams', [-10, 10])
main.add_module(particlegun)

# rotate muons by aplha around z and propagate them back in time to start outside CDC
main.add_module(swimMCParticle())

# geometry parameters
gearbox = register_module('Gearbox')
gearbox.param('fileName', 'geometry/CDCcosmicTests.xml')
gearbox.param('override', [("/DetectorComponent[@name='TOP']//Nbar", '1', ''),
                           ("/DetectorComponent[@name='TOP']//Phi0", '-135', 'deg'),
                           ("/DetectorComponent[@name='TOP']//Bars/Radius", '148', 'cm'),
                           ("/DetectorComponent[@name='TOP']//QZBackward", '-103.5', 'cm'),
                           ])
main.add_module(gearbox)

# detector geometry
geometry = register_module('Geometry')
main.add_module(geometry)

# detector simulation
g4sim = register_module('FullSim')
main.add_module(g4sim)

# trigger simulation
trigger = register_module('TOPbeamTrigger')
trigger.param('detectorIDs', [1, 2])  # scintillator counters 1 and 2
trigger.param('thresholds', [0.5, 0.5])  # MeV
main.add_module(trigger)
emptyPath = create_path()
trigger.if_false(emptyPath)

# CDC digitization
cdc_digitizer = register_module('CDCDigitizer')
main.add_module(cdc_digitizer)

# TOP digitization
top_digitizer = register_module('TOPDigitizer')
top_digitizer.param('timeZeroJitter', 40e-3)
main.add_module(top_digitizer)

# Reconstruction

# CDC track finder
cdc_trackfinder = register_module('Trasan')
# cdc_trackfinder = register_module('TrackFinderCDCCosmics')
main.add_module(cdc_trackfinder)

# Dedicated track fitter (performs also track extrapolation to TOP and dE/dx measurement)
trackfitter = register_module('CDCStraightLineFitter')
trackfitter.param('alpha', alpha)
# trackfitter.param('smearDriftLength', 0.015)
# trackfitter.param('smearDedx', 0.30)
main.add_module(trackfitter)

# TOP reconstruction
topreco = register_module('TOPReconstructor')
main.add_module(topreco)

# Output to flat ntuple
ntuple = register_module('CDCTOPNtuple')
main.add_module(ntuple)

# Standard output (uncomment if you need it)
# output = register_module('RootOutput')
# output.param('outputFileName', 'simrecCDCcosmic.root')
# main.add_module(output)

# Event display (uncomment if you want to use)
# display = main.add_module('Display') # or AsyncDisplay
# display.param('showCDCHits', True)
# display.param('fullGeometry', True)

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print call statistics
print(statistics)
