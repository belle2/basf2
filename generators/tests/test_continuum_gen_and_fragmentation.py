##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''Test for checking if the continuum generation and fragmentation works.'''

import basf2 as b2
import b2test_utils as b2tu
from generators import add_continuum_generator, get_default_decayfile
import pdg


def run_continuum_generation(finalstate):
    with b2tu.clean_working_directory():
        n_events = 100
        b2.set_random_seed(57)
        main = b2.Path()
        main.add_module('EventInfoSetter')
        add_continuum_generator(
            finalstate=finalstate,
            path=main,
        )
        b2.process(main, n_events)


def run_fragmentation_with_add_particles():
    with b2tu.clean_working_directory():
        n_events = 100
        b2.set_random_seed(57)
        pdg.add_particle("ALP", 9000005, 0, 0, 0, 0)

        main = b2.Path()
        main.add_module('EventInfoSetter')

        kkmc_inputfile = b2.find_file('data/generators/kkmc/ccbar_nohadronization.input.dat')
        kkmc_logfile = 'kkmc_ccbar.txt'
        kkmc_config = b2.find_file('data/generators/kkmc/KK2f_defaults.dat')
        # use KKMC to generate ccbar events (no fragmentation at this stage)
        main.add_module(
            'KKGenInput',
            tauinputFile=kkmc_inputfile,
            KKdefaultFile=kkmc_config,
            taudecaytableFile='',
            kkmcoutputfilename=kkmc_logfile,
            eventType="ccbar",
        )

        pythia_config = b2.find_file('data/generators/modules/fragmentation/pythia_belle2_charm.dat')
        decay_user = b2.find_file('data/generators/modules/fragmentation/dec_belle2_qqbar.dec')
        decay_file = get_default_decayfile()
        # add the fragmentation module to fragment the generated quarks into hadrons
        # using PYTHIA8
        main.add_module(
            'Fragmentation',
            ParameterFile=pythia_config,
            ListPYTHIAEvent=0,
            UseEvtGen=1,
            DecFile=decay_file,
            UserDecFile=decay_user,
            # Would be better to use the ALP here as well, but this makes the generation more complex
            QuarkPairMotherParticle=23,
            AdditionalPDGCodes=[9000005],
        )
        b2.process(main, n_events)


for final_state in ["uubar", "ddbar", "ssbar", "ccbar"]:
    b2.B2INFO(f'Running the test for continuum generation and final state {final_state}')
    return_code = b2tu.run_in_subprocess(target=run_continuum_generation, finalstate=final_state)
    if return_code != 0:
        b2.B2FATAL(
            f'Continuum generation fails when generating events for final state {final_state}')

b2.B2INFO('Running the test for fragmentation with additional particles')
return_code = b2tu.run_in_subprocess(target=run_fragmentation_with_add_particles)
if return_code != 0:
    b2.B2FATAL('Fragmentation fails when adding additional particles to Pythia')
