##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import sys
import basf2

if len(sys.argv) != 2:
    print("Usage: {} [bbbrem|bhwide|bhwide_largeangle]".format(sys.argv[0]), file=sys.stderr)
    sys.exit(1)

generator = sys.argv[1].lower()

kill = basf2.create_path()
main = basf2.create_path()
main.add_module("EventInfoSetter", expList=0, runList=1, evtNumList=1000000)
main.add_module("EventInfoPrinter")

basf2.set_log_level(basf2.LogLevel.DEBUG)
basf2.set_debug_level(250)
basf2.logging.package("framework").log_level = basf2.LogLevel.WARNING


def add_cut(name, minParticles, maxParticles, minTheta, maxTheta=None):
    """Add a generator level cut and kill the event if the cut is not passed.  In
    this case the cut is on the min/max charged particles which have a
    center-of-mass theta angle between minTheta and maxTheta. If maxTheta is not
    given assume it to be 180-minTheta for a symmetric window"""

    # if only one angle make it symmetric
    if maxTheta is None:
        maxTheta = 180 - minTheta
    selection = main.add_module("GeneratorPreselection", applyInCMS=True, nChargedMin=minParticles, nChargedMax=maxParticles,
                                MinChargedTheta=minTheta, MaxChargedTheta=maxTheta, MinChargedP=0., MinChargedPt=0.)
    selection.if_value("!=11", kill)
    selection.set_name("generator cut: " + name)


if generator == "bbbrem":
    main.add_module("BBBremInput", MinPhotonEnergyFraction=0.000001, Unweighted=True, MaxWeight=1.57001e+07)
    # at least one track below 0.5 degree means maximum one particle in 0.5-179.5
    add_cut("at least one track below 0.5 degree", 0, 1, 0.5)
elif generator == "bhwide":
    main.add_module("BHWideInput", ScatteringAngleRangeElectron=[0.5, 179.5], ScatteringAngleRangePositron=[0.5, 179.5])
    add_cut("both tracks at least 0.5 degree", 2, 2, 0.5)
    # but if one is above 1 and the other above 10 degree so we in 1-170 and
    # 10-179
    add_cut("max one track in 1-170", 0, 1, 1, 170)
    add_cut("max one track in 10-179", 0, 1, 10, 179)
elif generator == "bhwide_largeangle":
    main.add_module("BHWideInput", ScatteringAngleRangeElectron=[0.5, 179.5], ScatteringAngleRangePositron=[0.5, 179.5])
    add_cut("both tracks at least 1 degree", 2, 2, 1)
    add_cut("at least one 10 degree", 1, 2, 10)
else:
    print("unknown generation setting: {}".format(generator))

main.add_module("Progress")
main.add_module("RootOutput", outputFileName="%s.root" % generator)
basf2.process(main)
print(basf2.statistics)
