#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Descriptor: MC16rd_proc16 (GitLab: Issue-87) - Phase: eph3 taupair (Run Dependent BG)

#############################################################
# Steering file for official MC production of signal samples
#
# April 2021 - Belle II Collaboration
# ############################################################

import basf2 as b2
import generators as ge
import simulation as si

import reconstruction as re
import mdst as mdst
import glob as glob


# set database conditions (in addition to default)

# b2.conditions.prepend_testing_payloads("/home/t2k/taichiro/public/localdb_grl_rel10_iov0011_ver2/database.txt")

b2.conditions.prepend_testing_payloads("/home/belle/nkzw/basf2/payloads/MCri.taunn/database.txt")
# b2.conditions.prepend_testing_payloads("/home/belle2/zhaoxy/basf2/localdb/database.txt")
b2.conditions.prepend_testing_payloads("/home/t2k/taichiro/public/localdb_trggrl_20260421//database.txt")
# b2.conditions.prepend_testing_payloads("/home/belle2/zhaoxy/development/trg/grl/tools/localdb/database.txt")
# b2.conditions.prepend_testing_payloads("/home/belle/liyb/service/trigger/development/basf2/localdb/database.txt")
# b2.conditions.prepend_testing_payloads("/home/t2k/taichiro/public/localdb_grl_rel10_iov0011_20250829/database.txt")
b2.conditions.prepend_testing_payloads("/home/belle2/liuzd/group/zhaoxinyue/development/trg/grl/tools/localdb/database.txt")

# background (collision) files
bg = glob.glob('./*.root')
# if running locally
bg_local = glob.glob(
    "/group/belle2/dataprod/BGOverlay/BGOrd/rel8/BGOExp32rel8/bucket39/release-08-01-07/e0032/4S_offres/r00088/beambg/sub00/")
# create path
main = b2.create_path()

# specify number of events to be generated
main.add_module("EventInfoSetter", expList=1003, runList=0, evtNumList=10)
# main.add_module("EventInfoSetter", expList=1003, runList=0, evtNumList=11)

# events generator
ge.add_kkmc_generator(path=main, finalstate='tau+tau-', eventType="taupair")

# detector simulation
si.add_simulation(main)

# reconstruction
re.add_reconstruction(main)

# Finally add mdst output
# mdst.add_mdst_output(main, additionalBranches=['EventExtraInfo'], filename="10new_steer_mdst.root")
# unpacker = b2.register_module('TRGECLUnpacker')
# main.add_module(unpacker)

# mdst.add_mdst_output(main,
#                      additionalBranches=['EventExtraInfo', 'TRGGRLObjects','TRGECLCluster', 'TRGECLClusters'],
#                      filename="100000_new_steer_mdst.root")

mdst.add_mdst_output(main,
                     additionalBranches=['EventExtraInfo', 'TRGGRLObjects', 'TRGECLCluster', 'TRGECLClusters'],
                     filename="/home/belle2/liuzd/group/zhaoxinyue/try/53steer_try.root")


# mdst.add_mdst_output(main, additionalBranches=['EventExtraInfo'], filename=output_file)
main.add_module("GRLNeuro",
                TrgGrlInformation="TRGGRLObjects",
                TRGECLClusters="TRGECLClusters",
                HistFileName="hist_test.root",
                SaveHist=True)


# process events and print call statistics
b2.process(main)
print(b2.statistics)
