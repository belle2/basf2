##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
In the past we had issues with some wide resonances (e.g. rho) generated with a wrong mass value by PYTHIA.
This test checks that the mass of the rho is generated with a shift w.r.t. the evt.pdl value within 15 MeV.
'''

import basf2 as b2
import generators as ge
import pdg

b2.set_log_level(b2.LogLevel.ERROR)

main = b2.Path()

main.add_module('EventInfoSetter')

ge.add_inclusive_continuum_generator(main, finalstate='uubar', particles=['rho+'], userdecfile='', include_conjugates=True)


class CheckRhoMass(b2.Module):
    '''Check the mass of the rho resonance.'''

    def initialize(self):
        '''Initialize.'''

        import ROOT  # noqa
        #: the PDG value of the rho mass
        self.rho_pdg_mass = pdg.get('rho+').Mass()
        #: histogram of the rho mass shift
        self.rho_mass_shift = ROOT.TH1F('rho_mass_shift', 'rho mass shift', 1000, -0.25, 0.25)

    def event(self):
        '''Event.'''

        import ROOT  # noqa
        mc_particles = ROOT.Belle2.PyStoreArray('MCParticles')
        for mc_particle in mc_particles:
            if abs(mc_particle.getPDG()) == 213:  # rho+ or rho-
                self.rho_mass_shift.Fill(mc_particle.getMass() - self.rho_pdg_mass)

    def terminate(self):
        '''Terminate.'''

        import ROOT  # noqa
        maximum_bin = self.rho_mass_shift.GetMaximumBin()
        maximum_value = self.rho_mass_shift.GetXaxis().GetBinCenter(maximum_bin)
        message_maximum = f'The peak of the rho mass shift distributuion is at {int(maximum_value * 1000)} MeV'
        message_mean = f'The mean of the rho mass shift distribution is {int(self.rho_mass_shift.GetMean() * 1000)} MeV'
        if abs(maximum_value) < 0.015:  # let's accept a shift withing 15 MeV
            b2.B2INFO(message_mean)
            b2.B2INFO(message_maximum)
        else:
            b2.B2ERROR(message_mean)
            b2.B2ERROR(message_maximum)
            b2.B2FATAL('The tolerance on the peak position is 15 MeV')


main.add_module(CheckRhoMass()).set_log_level(b2.LogLevel.INFO)

b2.process(main)
