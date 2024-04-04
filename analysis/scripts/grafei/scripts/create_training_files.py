#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


from pathlib import Path
import argparse

import basf2 as b2
import modularAnalysis as ma

from ROOT import Belle2

import stdPhotons
from variables import variables as vm

from grafei import lcaSaver


def _get_args():
    parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
        description='Reconstruct most likely particles and save their features and LCA matrix'
    )
    parser.add_argument('-t', '--type', choices=['B0', 'B+', 'Ups'], required=True,
                        help='Training target', metavar='type',
                        dest='type')
    return parser.parse_args()


if __name__ == '__main__':
    b2.conditions.prepend_globaltag(ma.getAnalysisGlobaltag())

    args = _get_args()

    input_file = Path(Belle2.Environment.Instance().getInputFilesOverride()[0])

    path = b2.create_path()
    ma.inputMdst(str(input_file), path=path)

    mc_particle_name = {
        "B0": "B0:MC",
        "B+": "B+:MC",
        "Ups": "Upsilon(4S)"
    }

    # ###### BUILD MC B/Ups FOR LCA/TAGGING ######
    ma.fillParticleListFromMC(mc_particle_name[args.type], '', path=path)

    # These priors were obtained by counting truth-matched tracks in BB mixed MC
    # It could be modified by the user if needed
    priors = [0.068, 0.050, 0.7326, 0.1315, 0.0183, 0.00006]

    charged_cuts = [f"pidIsMostLikely({','.join(str(p) for p in priors)})>0", 'nCDCHits>20',
                    'thetaInCDCAcceptance', 'abs(dz)<1.0',
                    'dr<0.5', 'p<5', 'pt>0.2']

    photon_cuts = ['beamBackgroundSuppression>0.4', 'fakePhotonSuppression>0.3',
                   'abs(clusterTiming)<100', 'abs(formula(clusterTiming/clusterErrorTiming))<2.0',
                   '[[clusterReg==1 and E>0.09] or [clusterReg==2 and E>0.09] or [clusterReg==3 and E>0.14]]']

    charged_lists = [f'{c}:final' for c in ['p+', 'e+', 'pi+', 'mu+', 'K+']]

    ma.fillParticleLists(
        [(c, ' and '.join(charged_cuts)) for c in charged_lists],
        writeOut=True,
        path=path,
    )

    stdPhotons.stdPhotons(
        listtype='tight',
        path=path,
    )
    ma.getBeamBackgroundProbability("gamma:all", "MC15ri", path=path)
    ma.getFakePhotonProbability("gamma:all", "MC15ri", path=path)
    ma.cutAndCopyList(
        'gamma:final',
        'gamma:tight',
        ' and '.join(photon_cuts),
        writeOut=True,
        path=path,
    )

    # Add requirements on total number of photons and charged in event
    vm.addAlias('n_gamma_in_evt', 'nParticlesInList(gamma:final)')
    vm.addAlias('n_p_in_evt', 'nParticlesInList(p+:final)')
    vm.addAlias('n_e_in_evt', 'nParticlesInList(e+:final)')
    vm.addAlias('n_mu_in_evt', 'nParticlesInList(mu+:final)')
    vm.addAlias('n_pi_in_evt', 'nParticlesInList(pi+:final)')
    vm.addAlias('n_K_in_evt', 'nParticlesInList(K+:final)')
    vm.addAlias('n_charged_in_evt', 'formula(n_p_in_evt+n_e_in_evt+n_mu_in_evt+n_pi_in_evt+n_K_in_evt)')

    ma.applyEventCuts('n_gamma_in_evt<20 and n_charged_in_evt<20', path=path)

    # Set up variables to save to NTuple
    save_vars = [
        # Charged variables
        'electronID_noSVD_noTOP',
        'kaonID_noSVD',
        'protonID_noSVD',
        'muonID_noSVD',
        'pionID_noSVD',
        'p',
        'pt',
        'px',
        'py',
        'pz',
        'dr',
        'dz',
        'x',
        'y',
        'z',
        'charge',
        # Neutral variables
        'clusterNHits',
        'clusterTiming',
        'clusterE9E25',
    ]

    # Set up particle lists we'll work with
    p_lists = charged_lists + ['gamma:final']
    p_names = [x.split(':')[0] for x in p_lists]

    # ##### TAG AND SAVE #######

    # Flag each particle according to the B meson and decay it came from
    for i, p_list in enumerate(p_lists):
        # Match MC particles for all lists
        ma.matchMCTruth(p_list, path=path)

    lcaSaver(
        particle_lists=p_lists,
        features=save_vars,
        mcparticle_list=mc_particle_name[args.type],
        output_file=f'graFEI_train_{input_file.stem}.root',
        path=path,
    )

    # Actually run everything
    b2.process(path)
    print(b2.statistics)
