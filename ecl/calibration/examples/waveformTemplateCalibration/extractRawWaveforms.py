# /**************************************************************************
# * basf2 (Belle II Analysis Software Framework)                           *
# * Author: The Belle II Collaboration                                     *
# *                                                                        *
# * See git log for contributors and copyright holders.                    *
# * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
# **************************************************************************/

import basf2
import sys

main_path = basf2.create_path()

inID = sys.argv[1]

indir = sys.argv[2]

raw_branches = ["EventMetaData", "RawECLs"]

main_path.add_module("RootInput", inputFileNames="", branchNames=raw_branches)

eclunpacker = basf2.register_module('ECLUnpacker')
eclunpacker.param("storeTrigTime", True)
main_path.add_module(eclunpacker)

main_path.add_module('ECLWaveformFit')

eclCovMat = basf2.register_module('ECLWaveformCalibCollector')
eclCovMat.param("OutputFileName", indir+"outWaveInfo_"+inID+".root")

main_path.add_module(eclCovMat)

basf2.print_path(main_path)
basf2.process(main_path)
