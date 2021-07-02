#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2

# ---------------------------------------------------------------
# example of using OpticalGun to simulate the TOP laser calibration
# system. Nine sources are located outside of the prism, in front
# of its slanted face, pointing to the PMTs
#
# Contributors: Marko Staric
#               Stefano Lacaprara
#               Umberto Tamponi
# ---------------------------------------------------------------


def addSource(x, angle, slotID, path):
    '''
    Adds a laser source to the path
    @param x local x coordinate of teh source in the bar frame
    @param angle vertical tilt of the source
    @param slotID 1-16, slot number. If it's 0, then all the coorinates are in the BelleII frame
    '''
    path.add_module('OpticalGun',
                    minAlpha=0.0,  # not used if angulardistribution == 'Gaussian'
                    maxAlpha=33.0,  # not used if angulardistribution == 'Gaussian'
                    na=0.50,       # used only if angulardistribution == 'Gaussian'
                    startTime=0,
                    pulseWidth=10.0e-3,  # laser time jitter (1 Gaussian sigma), [ns]
                    numPhotons=10,
                    diameter=10.0e-3,  # source diameter in cm
                    slotID=slotID,  # if nonzero, local (slot) frame, otherwise Belle II
                    x=x,
                    y=-3.26,
                    z=-131.33,
                    theta=180 + angle,
                    phi=0.0,
                    psi=0.0,
                    angularDistribution='uniform'
                    # angularDistribution='(40-x)*TMath::Sin(x)' # You can have whatever distribution you like
                    )


# Create path
main = b2.create_path()


# Set number of events to generate
main.add_module('EventInfoSetter',
                expList=[1003],  # 0 for nominal phase 3, 1002 for phase II, 1003 for early phase III
                evtNumList=[100])

# Gearbox: access to database (xml files)
main.add_module('Gearbox')

# Geometry
main.add_module('Geometry')

# Optical sources
for slotId in range(1, 17):
    for pos in [0.9, 5.7, 11.3, 16.9, 22.5, 28.1, 33.7, 39.3, 44.1]:
        angle = 17
        x = -45. / 2. + pos
        addSource(x, angle, slotId, main)

# Simulation
main.add_module('FullSim')

# TOP digitization
main.add_module('TOPDigitizer')

# Output
main.add_module('RootOutput',
                outputFileName='opticalGun.root')

# Show progress of processing
main.add_module('Progress')

# Process events
b2.process(main)

# Print call statistics
print(b2.statistics)
