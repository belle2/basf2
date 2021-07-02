#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

##########################################################################
#                                                                        #
# Stuck? Ask for help at questions.belle2.org                            #
#                                                                        #
# This tutorial demonstrates how to weight tracks using                  #
# Belle LooKUpTable from the database                                    #
#                                                                        #
##########################################################################

import basf2 as b2
import modularAnalysis as ma
import variables as va
import os
from b2biiConversion import convertBelleMdstToBelleIIMdst

# Usual b2bii magic
os.environ['PGUSER'] = 'g0db'
os.environ['BELLE_POSTGRES_SERVER'] = 'can01'
os.environ['USE_GRAND_REPROCESS_DATA'] = '1'

my_path = b2.create_path()

# Adding database with weight tables
b2.conditions.prepend_globaltag("BellePID")

# Read in and convert Belle mdst
inputfile = b2.find_file('b2bii_input_evtgen_exp_07_BptoD0pip-D0toKpipi0-0.mdst', 'examples', False)
convertBelleMdstToBelleIIMdst(inputfile, path=my_path)

"""
In this example, we add information from three types of weight tables to the track.
Since there are only three types of such tables, the example is quite full.
Each weight table contains variables listed in the example.
Meaning of the variables can be found in Belle documentation (see links in the body of the example).
"""

ma.fillParticleList(decayString='pi+:all', cut='', path=my_path)

"""
Lepton ID

Several weight tables were produced for lepton ID correction at Belle.
Naming scheme for payloads is:

BelleLID<e|mu>_<LID cut>

Study was done for different cut set for electrons and muons.
Possible cuts for electron:
0.01, 0.10, 0.50, 0.60, 0.80, 0.90

Possible cuts for muons:
0.10, 0.80, 0.90, 0.95, 0.97

Sources of original Belle results are here:
https://belle.kek.jp/group/pid_joint/lid/final/

Here we add PID correction information that is important for
electrons selected with "eIDBelle>0.9" cut.
"""
lid_table = "BelleLIDe_0.90"

va.variables.addAlias('LID_ratio', 'extraInfo('+lid_table+'_ratio)')
va.variables.addAlias('LID_ratio_stat_err', 'extraInfo('+lid_table+'_ratio_stat_err)')
va.variables.addAlias('LID_ratio_syst_err', 'extraInfo('+lid_table+'_ratio_syst_err)')
va.variables.addAlias('LID_ratio_syst_err_from_2photon', 'extraInfo('+lid_table+'_ratio_syst_err_from_2photon)')
va.variables.addAlias('LID_ratio_syst_err_from_JPsi', 'extraInfo('+lid_table+'_ratio_syst_err_from_JPsi)')
va.variables.addAlias('LID_run_bin', 'extraInfo('+lid_table+'_run_bin)')
va.variables.addAlias('LID_kinematic_bin', 'extraInfo('+lid_table+'_kinematic_bin)')

lid_weights = ['LID_ratio',
               'LID_ratio_stat_err',
               'LID_ratio_syst_err',
               'LID_ratio_syst_err_from_2photon',
               'LID_ratio_syst_err_from_JPsi',
               'LID_run_bin',
               'LID_kinematic_bin']

reweighter = b2.register_module('ParticleWeighting')
reweighter.param('tableName', lid_table)
reweighter.param('particleList', 'pi+:all')
my_path.add_module(reweighter)


"""
Kaon ID

Several weight tables were produced for kaon/pion ID corrections at Belle.
Naming scheme for payloads is:

BelleKID<K|Pi><Eff|Fake><Combined|Plus|Minus>_<PID cut:[1..9]>

Here,
  - K or Pi refers to correction information obtained for kaons or pions, accordingly
  - Eff or Fake refers to efficiency or fake rate corrections
  - Combined, Plus or Minus refers to the charge of tracks used for the study
  - PID cut is an integer number from 1 to 9 that correspond to "cut > 0.number"

Information is taken from here:
  https://belle.kek.jp/group/pid_joint/kid/files-2006/

Here we add PID correction information for pion efficiency selected as kaons with
"kaonIDBelle>0.1" cut.

"""
kid_table = "BelleKIDPiFakePlus_1"

va.variables.addAlias('KID_eff_data', 'extraInfo('+kid_table+'_eff_data)')
va.variables.addAlias('KID_eff_data_stat_err', 'extraInfo('+kid_table+'_eff_data_stat_err)')
va.variables.addAlias('KID_eff_data_syst_err', 'extraInfo('+kid_table+'_eff_data_syst_err)')
va.variables.addAlias('KID_eff_mc', 'extraInfo('+kid_table+'_eff_mc)')
va.variables.addAlias('KID_eff_mc_stat_err', 'extraInfo('+kid_table+'_eff_mc_stat_err)')
va.variables.addAlias('KID_fit_flag', 'extraInfo('+kid_table+'_fit_flag)')
va.variables.addAlias('KID_kinematic_bin', 'extraInfo('+kid_table+'_kinematic_bin)')
va.variables.addAlias('KID_ratio', 'extraInfo('+kid_table+'_ratio)')
va.variables.addAlias('KID_ratio_stat_err', 'extraInfo('+kid_table+'_ratio_stat_err)')
va.variables.addAlias('KID_ratio_syst_err', 'extraInfo('+kid_table+'_ratio_syst_err)')
va.variables.addAlias('KID_run_bin', 'extraInfo('+kid_table+'_run_bin)')

kid_weights = ['KID_eff_data',
               'KID_eff_data_stat_err',
               'KID_eff_data_syst_err',
               'KID_eff_mc',
               'KID_eff_mc_stat_err',
               'KID_fit_flag',
               'KID_ratio',
               'KID_ratio_stat_err',
               'KID_ratio_syst_err',
               'KID_run_bin',
               'KID_kinematic_bin']

reweighter2 = b2.register_module('ParticleWeighting')
reweighter2.param('tableName', kid_table)
reweighter2.param('particleList', 'pi+:all')
my_path.add_module(reweighter2)


"""
Proton ID

Several weight tables were produced for proton ID corrections at Belle.
Naming scheme for payloads is:

BellePID<Plus|Minus>_<PID cut>

Here,
  - Plus or Minus refers to the charge of tracks used for the study
  - PID cut can be 0.6, 0.7, 0.8 or 0.9

Information is taken from here:
  https://belle.kek.jp/group/pid_joint/protonid/

Here we add PID correction information for proton efficiency selected with
"protonIDBelle>0.6" cut.
"""
pid_table = "BellePIDPlus_0.6"

va.variables.addAlias("PID_eff_data", 'extraInfo('+pid_table+"_eff_data)")
va.variables.addAlias("PID_eff_data_stat_err", 'extraInfo('+pid_table+"_eff_data_stat_err)")
va.variables.addAlias("PID_eff_mc", 'extraInfo('+pid_table+"_eff_mc)")
va.variables.addAlias("PID_eff_mc_stat_err", 'extraInfo('+pid_table+"_eff_mc_stat_err)")
va.variables.addAlias("PID_ratio", 'extraInfo('+pid_table+"_ratio)")
va.variables.addAlias("PID_ratio_stat_err", 'extraInfo('+pid_table+"_ratio_stat_err)")
va.variables.addAlias("PID_ratio_syst_err", 'extraInfo('+pid_table+"_ratio_syst_err)")
va.variables.addAlias("PID_kinematic_bin", 'extraInfo('+pid_table+"_kinematic_bin)")
va.variables.addAlias("PID_run_bin", 'extraInfo('+pid_table+"_run_bin)")

pid_weights = ["PID_eff_data",
               "PID_eff_data_stat_err",
               "PID_eff_mc",
               "PID_eff_mc_stat_err",
               "PID_ratio",
               "PID_ratio_stat_err",
               "PID_ratio_syst_err",
               "PID_run_bin",
               "PID_kinematic_bin"]


reweighter3 = b2.register_module('ParticleWeighting')
reweighter3.param('tableName', pid_table)
reweighter3.param('particleList', 'pi+:all')
my_path.add_module(reweighter3)

"""
All weighting modules are configured.
Let's add some tracks and add the information from weighting tables to them.
"""
pivars = ['p', 'pz']
pivars += kid_weights
pivars += lid_weights
pivars += pid_weights


# Saving variables to ntuple
output_file = 'ApplyBelleWeightsToTracks.root'
ma.variablesToNtuple(decayString='pi+:all',
                     variables=pivars,
                     treename='pion',
                     filename=output_file,
                     path=my_path)

# Process the events
b2.process(my_path)

# print out the summary
print(b2.statistics)
