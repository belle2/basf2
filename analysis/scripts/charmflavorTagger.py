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

def CharmFlavorTagger(particle_lists,
                     path=None):
    """
    Interfacing for the CharmFlavorTagger.

    This function requires a reconstructed D meson signal particle list with a built RestOfEvent.

    :param particle_lists:  string or list[string], particle list(s) of the reconstructed signal D meson
    :param path: basf2 path obj
    :return: None
    """

    if isinstance(particle_lists, str):
        particle_lists = [particle_lists]

    # create roe specific paths
    roe_path = basf2.create_path()
    dead_end_path = basf2.create_path()

    # define cft specific lists to enable multiple calls, if someone really wants to do that
    extension = particle_lists[0].replace(':', '_to_')
    roe_particle_list_cut = 'isInRestOfEvent == 1 and dr < 1 and abs(dz) < 3'
    roe_particle_list = 'pi+:cft' + '_' + extension

    # filter rest of events only for specific particle list
    ma.signalSideParticleListsFilter(particle_lists, '', roe_path, dead_end_path)

    # create final state particle lists, split by charge
    ma.fillParticleList(roe_particle_list, roe_particle_list_cut, path=roe_path)

    cft_particle_lists = ['pi+:pos_charged', 'pi+:neg_charged']

    pos_cut = 'charge > 0 and p < infinity'
    neg_cut = 'charge < 0 and p < infinity'

    ma.cutAndCopyList(cft_particle_lists[0], roe_particle_list, pos_cut, writeOut=True, path=roe_path)
    ma.cutAndCopyList(cft_particle_lists[1], roe_particle_list, neg_cut, writeOut=True, path=roe_path)

    # rank tracks according to opening angle w.r.t signal list
    va.variables.addAlias("opang_shift","formula(abs(formula(angleToClosestInList("+particle_lists[0]+") - 3.14159265359/2)))")
    rank_variable = 'opang_shift'

    for particles in cft_particle_lists:
        ma.rankByHighest(particles, rank_variable, path=roe_path)

    # compute additional CFT input variables, PID_diff=pionID-kaonID, deltaR=sqrt(deltaPhi**2+deltaEta**2)
    va.variables.addAlias("eta","formula(-1*log(formula(tan(formula(theta/2)))))")
    va.variables.addAlias("phi_sig","particleRelatedToCurrentROE(phi)")
    va.variables.addAlias("eta_sig","particleRelatedToCurrentROE(eta)")
    va.variables.addAlias("deltaPhi_temp","formula(abs(formula(phi-phi_sig)))")
    va.variables.addAlias("deltaPhi","conditionalVariableSelector(deltaPhi_temp>3.14159265359,formula(deltaPhi_temp-2*3.14159265359),deltaPhi_temp)")
    va.variables.addAlias("deltaR","formula(((deltaPhi)**2+(eta-eta_sig)**2)**0.5)")
    va.variables.addAlias("PID_diff","formula(pionID-kaonID)")

    # write CFT input to extraInfo of signal particle
    roe_dict_p = {}
    roe_dict_n = {}
    var_list=['mRecoil','PID_diff','deltaR']
    roe_vars=[]
    for var in var_list:
        for i_num in range(1, 3 + 1):
            roe_dict_p['eventCached(getVariableByRank(' + 'pi+:pos_charged' + ', ' + 'opang_shift' + ', ' + var + ', ' +str(i_num) + '))'] = 'pi_'+str(i_num)+'_p_'+var
            va.variables.addAlias('pi_'+str(i_num)+'_p_'+var,'extraInfo(pi_'+str(i_num)+'_p_'+var+')')
            roe_vars += ['pi_'+str(i_num)+'_p_'+var]
            roe_dict_n['eventCached(getVariableByRank(' + 'pi+:neg_charged' + ', ' + 'opang_shift' + ', ' + var + ', ' +str(i_num) + '))'] = 'pi_'+str(i_num)+'_n_'+var
            va.variables.addAlias('pi_'+str(i_num)+'_n_'+var,'extraInfo(pi_'+str(i_num)+'_n_'+var+')')
            roe_vars += ['pi_'+str(i_num)+'_n_'+var]

    ma.variableToSignalSideExtraInfo('pi+:pos_charged', roe_dict_p, path=roe_path)
    ma.variableToSignalSideExtraInfo('pi+:neg_charged', roe_dict_n, path=roe_path)

    # apply CFT with MVAExpert module and write output to extraInfo
    expert_module = basf2.register_module('MVAExpert')
    expert_module.param('listNames', particle_lists)
    expert_module.param('identifier', 'CFT.xml')

    expert_module.param('extraInfoName', 'CFT_out')

    roe_path.add_module(expert_module)

    # The CFT ouptut probability should be 0.5 when no track is reconstructed in the ROE
    va.variables.addAlias('CFT_prob', 'conditionalVariableSelector(isNAN(pi_1_p_deltaR) and isNAN(pi_1_n_deltaR),0.5,formula(1-extraInfo(CFT_out)))')
    va.variables.addAlias('CFT_qr', 'formula(2*CFT_prob-1)')

    path.for_each('RestOfEvent', 'RestOfEvents', roe_path)
