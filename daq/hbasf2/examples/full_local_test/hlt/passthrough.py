##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import basf2
import hbasf2

from argparse import ArgumentParser

if __name__ == '__main__':
    basf2.reset_database()
    basf2.use_central_database("data_reprocessing_prompt")
    basf2.set_nprocesses(2)

    parser = ArgumentParser()
    parser.add_argument("--input", required=True)
    parser.add_argument("--output", required=True)
    parser.add_argument("--dqm", required=True)

    args = parser.parse_args()

    path = basf2.Path()

    input_module = path.add_module("HLTZMQ2Ds", input=args.input)

    reco_path = basf2.Path()
    reco_path.add_module("HLTDQM2ZMQ", output=args.dqm, sendOutInterval=5)

    input_module.if_value("==0", reco_path, basf2.AfterConditionPath.CONTINUE)

    path.add_module("HLTDs2ZMQ", output=args.output, raw=True)

    hbasf2.process(path, [args.output, args.dqm], False)
