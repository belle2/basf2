#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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


from ROOT import Belle2
import basf2


class SplitMultiplicities(basf2.Module):
    """
    Module to determine the multiplicities of a particle of a certain pdg code
    """
    def __init__(self, listname, pdgcode):
        """
        Initialise the class.
        :param listname: particle list name
        :param pdgcode: pdg code to be studied
        """
        super().__init__()
        #: Particle list name
        self.listname = listname
        #: pdg code to be studied
        self.pdgcode = pdgcode

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

        # set the extra info names and save them to the reconstructed  particles
        extraInfoName = 'nGen_{}'.format(self.pdgcode)
        extraInfoName_Bp = 'nGen_{}_Bp'.format(self.pdgcode)
        extraInfoName_Bm = 'nGen_{}_Bm'.format(self.pdgcode)
        extraInfoName_B0 = 'nGen_{}_B0'.format(self.pdgcode)
        extraInfoName_antiB0 = 'nGen_{}_antiB0'.format(self.pdgcode)

        for particle in Belle2.PyStoreObj(self.listname).obj():
            if particle.hasExtraInfo(extraInfoName):
                particle.setExtraInfo(extraInfoName, gen_counter)
            else:
                particle.addExtraInfo(extraInfoName, gen_counter)

            if particle.hasExtraInfo(extraInfoName_Bp):
                particle.setExtraInfo(extraInfoName_Bp, Bp_counter)
            else:
                particle.addExtraInfo(extraInfoName_Bp, Bp_counter)

            if particle.hasExtraInfo(extraInfoName_Bm):
                particle.setExtraInfo(extraInfoName_Bm, Bm_counter)
            else:
                particle.addExtraInfo(extraInfoName_Bm, Bm_counter)

            if particle.hasExtraInfo(extraInfoName_B0):
                particle.setExtraInfo(extraInfoName_B0, B0_counter)
            else:
                particle.addExtraInfo(extraInfoName_B0, B0_counter)

            if particle.hasExtraInfo(extraInfoName_antiB0):
                particle.setExtraInfo(extraInfoName_antiB0, antiB0_counter)
            else:
                particle.addExtraInfo(extraInfoName_antiB0, antiB0_counter)
