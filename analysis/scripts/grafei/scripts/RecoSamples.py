from pathlib import Path
import argparse

import basf2 as b2
import modularAnalysis as ma

# Necessary to run argparse
from ROOT import PyConfig
PyConfig.IgnoreCommandLineOptions = True  # noqa

from ROOT import Belle2

import stdPhotons
from variables import variables as vm

from grafei.modules.FlagBDecayModule import FlagBDecayModule
from grafei.modules.isMostLikelyTempVarsModule import isMostLikelyTempVars
from grafei.modules.RootSaverModule import RootSaverModule


def get_args():
    parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
        description='Reconstruct most likely particles and save their features and LCA matrix'
    )
    parser.add_argument('-t', '--type', choices=['B0', 'B+', 'Ups'], required=True,
                        help='Training target', metavar='type',
                        dest='type')
    # parser.add_argument('-b', '--bkg_prob', required=False, default=0., type=float,
    #                     help="Choose probability of an event to be constructed as background",
    #                     dest='bkg_prob')
    return parser.parse_args()


if __name__ == '__main__':
    b2.conditions.prepend_globaltag(ma.getAnalysisGlobaltag())

    args = get_args()

    # assert args.bkg_prob >= 0 and args.bkg_prob <= 1, "Background probability should be in [0, 1]"

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

    # Fill particle list with optimized cuts
    priors = [0.068, 0.050, 0.7326, 0.1315, 0.0183, 0.00006]

    charged_cuts = ['nCDCHits>20',
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

    most_likely_module = isMostLikelyTempVars(charged_lists, priors)
    path.add_module(most_likely_module)

    for list in charged_lists:
        ma.applyCuts(list, 'extraInfo(isMostLikelyTempVars)==1', path=path)

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
        # 'electronID',
        # 'kaonID',
        # 'protonID',
        # 'muonID',
        # 'pionID',
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
        # 'dx',
        # 'dy',
        'dz',
        'x',
        'y',
        'z',
        # 'chiProb',
        # 'E',
        'charge',
        # 'M',
        # 'PDG',
        # Neutral variables
        # 'clusterReg',
        'clusterNHits',
        'clusterTiming',
        'clusterE9E25',
    ]

    # Set up particle lists we'll work with
    p_lists = charged_lists + ['gamma:final']
    p_names = [x.split(':')[0] for x in p_lists]

    # ##### TAG AND SAVE #######
    b_parent_var = 'BParentGenID'

    # Flag each particle according to the B meson and decay it came from
    for i, p_list in enumerate(p_lists):
        # Match MC particles for all lists
        ma.matchMCTruth(p_list, path=path)

        # Add extraInfo to each particle indicating parent B genID and
        # whether it belongs to a semileptonic decay
        flag_decay_module = FlagBDecayModule(
            p_list,
            b_parent_var=b_parent_var,
        )
        path.add_module(flag_decay_module)

    root_saver_module = RootSaverModule(
        particle_lists=p_lists,
        features=save_vars,
        b_parent_var=b_parent_var,
        mcparticle_list=mc_particle_name[args.type],
        output_file=f'output_{input_file.stem}.root',
        # bkg_prob=args.bkg_prob,
    )
    path.add_module(root_saver_module)

    # Actually run everything
    b2.process(path)
    print(b2.statistics)
