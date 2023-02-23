#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2
import variables as va
import modularAnalysis as ma

def charmFlavorTagger(particle_list,uniqueIdentifier='CFT.xml',
                     path=None):
    """
    Interfacing for the Charm Flavor Tagger.

    This function requires a reconstructed D meson signal particle list with a built RestOfEvent.

    :param particle_list:  string, particle list of the reconstructed signal D meson
    :param uniqueIdentifier: string, database identifier for the method
    :param path: basf2 path obj
    :return: None
    """

    # create roe specific paths
    roe_path = basf2.create_path()
    dead_end_path = basf2.create_path()

    # define cft specific lists to enable multiple calls, if someone really wants to do that
    extension = particle_list.replace(':', '_to_')
    roe_particle_list_cut = 'isInRestOfEvent == 1 and dr < 1 and abs(dz) < 3'
    roe_particle_list = 'pi+:cft' + '_' + extension

    # filter rest of events only for specific particle list
    ma.signalSideParticleFilter(particle_list, '', roe_path, dead_end_path)

    # create final state particle lists
    ma.fillParticleList(roe_particle_list, roe_particle_list_cut, path=roe_path)

    # compute ranking variable and additional CFT input variables, PID_diff=pionID-kaonID, deltaR=sqrt(deltaPhi**2+deltaEta**2)
    rank_variable = 'opang_shift'
    va.variables.addAlias(rank_variable,f"abs(formula(angleToClosestInList({particle_list}) - 3.14159265359/2))")
    va.variables.addAlias("eta","formula(-1*log(tan(formula(theta/2))))")
    va.variables.addAlias("phi_sig","particleRelatedToCurrentROE(phi)")
    va.variables.addAlias("eta_sig","particleRelatedToCurrentROE(eta)")
    va.variables.addAlias("deltaPhi_temp","abs(formula(phi-phi_sig))")
    va.variables.addAlias("deltaPhi","conditionalVariableSelector(deltaPhi_temp>3.14159265359,formula(deltaPhi_temp-2*3.14159265359),deltaPhi_temp)")
    va.variables.addAlias("deltaR","formula(((deltaPhi)**2+(eta-eta_sig)**2)**0.5)")
    va.variables.addAlias("PID_diff","formula(pionID-kaonID)")

    # split tracks by charge, rank them (keep only the three highest ranking) and write CFT input to extraInfo of signal particle
    var_list=['mRecoil','PID_diff','deltaR']
    cft_particle_dict = {'pi+:pos_charge':['charge > 0 and p < infinity', 'p'], 'pi+:neg_charge':['charge < 0 and p < infinity', 'n']}

    for listName, config in cft_particle_dict.items():
        ma.cutAndCopyList(listName, roe_particle_list, config[0], writeOut=True, path=roe_path)
        ma.rankByHighest(listName, rank_variable, numBest=3, path=roe_path)
        roe_dict = {}
        suffix = config[1]
        for var in var_list:
            for i_num in range(1, 3 + 1):
                roe_dict[f'eventCached(getVariableByRank({listName}, {rank_variable}, {var}, {i_num}))'] = f'pi_{i_num}_{suffix}_{var}'
                va.variables.addAlias(f'pi_{i_num}_{suffix}_{var}', f'extraInfo(pi_{i_num}_{suffix}_{var})')

        ma.variableToSignalSideExtraInfo(listName, roe_dict, path=roe_path)
    
    # apply CFT with MVAExpert module and write output to extraInfo
    expert_module = basf2.register_module('MVAExpert')
    expert_module.param('listNames', [particle_list])
    expert_module.param('identifier', uniqueIdentifier)

    expert_module.param('extraInfoName', 'CFT_out')

    roe_path.add_module(expert_module)

    # The CFT output probability should be 0.5 when no track is reconstructed in the ROE
    va.variables.addAlias('CFT_prob', 'conditionalVariableSelector(isNAN(pi_1_p_deltaR) and isNAN(pi_1_n_deltaR),0.5,formula(1-extraInfo(CFT_out)))')
    va.variables.addAlias('CFT_qr', 'formula(2*CFT_prob-1)')

    path.for_each('RestOfEvent', 'RestOfEvents', roe_path)
