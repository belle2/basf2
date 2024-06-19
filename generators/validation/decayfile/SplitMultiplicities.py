#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#############################################################
# Python basf2 module to determine the multiplicities of a particle of a certain pdg code
# (antiparticles not included) on generator level. We loop over the number of mcParticles and
# count the number of particles of a certain pdg code. In addition, the mutiplicities are split
# into categories depending on whether they originate from a B+/B-/B0/antiB0.
# We need to write the information as extraInfo to reconstructed particles, hence the need for
# a reco particle list as input.
#############################################################

"""
<header>
    <noexecute>Definition of helper class</noexecute>
</header>
"""

from ROOT import Belle2
import basf2


class SplitMultiplicities(basf2.Module):
    """
    Module to determine the multiplicities of a particle of a certain pdg code
    """

    def __init__(self, pdgcode):
        """
        Initialise the class.
        :param pdgcode: pdg code to be studied
        """
        super().__init__()
        #: pdg code to be studied
        self.pdgcode = pdgcode
        #: event extra info object
        self.eventExtraInfo = Belle2.PyStoreObj("EventExtraInfo")

    def event(self):
        """ Event function """

        # set counters to 0
        gen_counter = 0
        Bp_counter = 0
        Bm_counter = 0
        B0_counter = 0
        antiB0_counter = 0

        # get generator particles and loop over them until reaching a B+/B-/B0/antiB0 mother
        mcParticles = Belle2.PyStoreArray('MCParticles')
        for mcparticle in mcParticles:
            if (mcparticle.isPrimaryParticle()) and (mcparticle.getPDG() == self.pdgcode):
                gen_counter += 1
                mcMother = mcparticle.getMother()
                while mcMother:
                    pdg = mcMother.getPDG()
                    if pdg == 521:  # B+
                        Bp_counter += 1
                        break
                    elif pdg == -521:  # B-
                        Bm_counter += 1
                        break
                    elif pdg == 511:  # B0
                        B0_counter += 1
                        break
                    elif pdg == -511:  # antiB0
                        antiB0_counter += 1
                        break
                    else:
                        mcMother = mcMother.getMother()

        # create the extra info names and save the corresponding multiplicities
        extraInfoName = f'nGen_{self.pdgcode}'
        extraInfoName_Bp = f'nGen_{self.pdgcode}_Bp'
        extraInfoName_Bm = f'nGen_{self.pdgcode}_Bm'
        extraInfoName_B0 = f'nGen_{self.pdgcode}_B0'
        extraInfoName_antiB0 = f'nGen_{self.pdgcode}_antiB0'

        self.eventExtraInfo.create()
        self.eventExtraInfo.setExtraInfo(extraInfoName, gen_counter)
        self.eventExtraInfo.setExtraInfo(extraInfoName_Bp, Bp_counter)
        self.eventExtraInfo.setExtraInfo(extraInfoName_Bm, Bm_counter)
        self.eventExtraInfo.setExtraInfo(extraInfoName_B0, B0_counter)
        self.eventExtraInfo.setExtraInfo(extraInfoName_antiB0, antiB0_counter)
