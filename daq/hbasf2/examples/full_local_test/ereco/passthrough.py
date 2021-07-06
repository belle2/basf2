##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import basf2
import hbasf2

from zmq_daq.example_support import add_input_module, add_reco_modules, add_output_module

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
    reco_path = basf2.Path()

    input_module = add_input_module(path, input_address=args.input, add_expressreco_objects=True)

    add_reco_modules(reco_path, dqm_address=args.dqm, mimik_startup=False)

    input_module.if_value("==0", reco_path, basf2.AfterConditionPath.CONTINUE)

    add_output_module(path, output_address=args.output, raw=False)

    hbasf2.process(path, [args.output, args.dqm], False)
