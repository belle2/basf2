##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import ROOT
from ROOT import Belle2
from basf2 import Module, register_module


def neutralHadron4MomentumCalculator(list_name, path=None, use_prototype=False):
    module = register_module('NeutralHadron4MomentumCalculator')
    module.param('listName', list_name)
    path.add_module(module if not use_prototype else NeutralHadron4MomentumCalculatorPrototype(list_name))


class NeutralHadron4MomentumCalculatorPrototype(Module):
    '''Calculates 4-momentum of a neutral hadron in a given decay chain e.g. B0 -> J/Psi K_L0, or anti-B0 -> p+ K- anti-n0.

    The momenta of the neutral hadron and its mother will be updated accordingly, as in the case of vertex-fitting.
    '''

    def __init__(self, list_name):
        '''Constructor for NeutralHadron4MomentumCalculator

        Parameters:
            list_name: ParticleList in which the neutron hadron is 1st daughter in the decay
            path: path to put the module in
        '''

        super().__init__()
        #: ParticleList
        self._list_name = list_name
        #: Direction of the neutral particle
        self._neutralDirection = ROOT.TVector3()
        #: Momentum of the neutral particle
        self._neutral3Momentum = ROOT.TVector3()
        #: 4-momentum of the nuetral particle
        self._neutral4Momentum = ROOT.TLorentzVector()
        #: A vector to store indices of Particle in the Particle List to remove
        self._toRemove = ROOT.std.vector('unsigned int')()

    def event(self):
        '''Function to execute in each event'''
        particleList = Belle2.PyStoreObj(self._list_name).obj()
        for particle in particleList:
            charged = particle.getDaughter(0)
            neutral = particle.getDaughter(1)
            self._neutralDirection = neutral.getECLCluster().getClusterPosition().Unit()
            a = charged.getMomentum() * self._neutralDirection
            b = (particle.getPDGMass()**2 - neutral.getMass()**2 - charged.get4Vector().Mag2()) / 2.
            c = charged.getEnergy()
            d = neutral.getMass()**2
            D = (a**2 - c**2) * d + b**2
            if D >= 0:
                neutralP = (-1. * a * b - c * D**(0.5)) / (a**2 - c**2)
                self._neutral3Momentum.SetMagThetaPhi(neutralP, self._neutralDirection.Theta(), self._neutralDirection.Phi())
                self._neutral4Momentum.SetVectM(self._neutral3Momentum, neutral.getMass())
                neutral.set4Vector(self._neutral4Momentum)
                particle.set4Vector(neutral.get4Vector() + charged.get4Vector())
            else:  # Remove this Particle from its ParticleList
                self._toRemove.push_back(particle.getArrayIndex())
        particleList.removeParticles(self._toRemove)
        self._toRemove.clear()
