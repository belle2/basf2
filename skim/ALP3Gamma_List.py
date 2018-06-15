from basf2 import *
from modularAnalysis import *  # Standard Analysis Tools
import pdg
import numpy as np

pdg.add_particle('beam', 55, 999., 999., 0, 0)
pdg.add_particle('ALP', 9000006, 999., 999., 0, 0)

# python function for reconstructing an ALP


def initialALP():

    # no cuts applied on ALP
    ALPcuts = ''

    # applying a lab frame energy cut to the daughter photons
    fillParticleList('gamma:all', 'E >= 0.1 and Theta >= formula(' + str(np.pi) +
                     '*17/180) and Theta <= formula(' + str(np.pi) + '*150/180)', True)

    # defining the decay string
    ALPchannels = ['gamma:all  gamma:all']
    ALPList = []

    # creating an ALP from the daughter photons
    for chID, channel in enumerate(ALPchannels):
        mode = 'ALP:' + str(chID) + ' -> ' + channel
        print(mode)
        reconstructDecay(mode, ALPcuts, chID)

        ALPList.append('ALP:' + str(chID))

    Lists = ALPList
    return Lists

# python function for reconstructing a beam particle


def beam():

    # applying invariant mass cut on the beam particle
    beamcuts = 'InvM >= formula(0.8 * Ecms) and InvM <= formula(1.05 * Ecms) and maxWeightedDistanceFromAverageECLTime <= 2'

    ALPList = initialALP()

    # applying a lab frame energy cut to the recoil photon
    fillParticleList('gamma:all', 'E >= 0.1', True)
    beamList = []

    # reconstructing decay using the reconstructed ALP
    # from previous function and adding the recoil photon
    for chID, channel in enumerate(ALPList):
        mode = 'beam:' + str(chID) + ' -> gamma:all ' + channel
        print(mode)
        reconstructDecay(mode, beamcuts, chID)
        beamList.append('beam:' + str(chID))
    return beamList
