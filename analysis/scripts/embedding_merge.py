import basf2
from mdst import add_mdst_output
import argparse


def prepare_path(File1, File2, FileOut):
    """Prepare path for event embedding.
    Input:
      File1: primary input file (ROE)
      File2: secondary input file (truth-matched signal)
      FileOut: output embedded file.
    """
    main = basf2.Path()
    indep = basf2.Path()
    input1 = basf2.register_module('RootInput')
    input1.param('inputFileName', File1)
    main.add_module(input1).set_name("input1")

    # and the other input
    input2 = basf2.register_module('RootInput')
    # input2.param('inputFileName', 'mcjpsi_skim.root')
    input2.param('inputFileName', File2)
    input2.param('isSecondaryInput', True)
    indep.add_module(input2).set_name("input2")

    branches = ['ALL']

    # merge it!
    main.add_independent_merge_path(
        indep,
        merge_back_event=branches)

    main.add_module('FixMergedObjects')
    main.add_module("PostMergeUpdater")

    # write out further reduced set

    additionalBranches = ['MergedArrayIndices',
                          'EventLevelClusteringInfo_indepPath',
                          'EventLevelTrackingInfo_indepPath',
                          'EventMetaData_indepPath',
                          'TRGSummary_indepPath',
                          'SoftwareTriggerResult_indepPath'
                          ]

    # output
    add_mdst_output(main, filename=FileOut, additionalBranches=additionalBranches)

    return main


def get_parser():
    """Handles the command-line argument parsing.

    Returns:
        argparse.Namespace: The parsed arguments.
    """
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('--filePrimary', help='Input file to be used as a primary (tag-side data or MC)', required=True)
    parser.add_argument('--fileSecondary', help='Input file to be used as a secondary (signal-side MC)', required=True)
    parser.add_argument("--fileOut", default='merged.root', help='Output file name')
    return parser


if __name__ == '__main__':
    args = get_parser().parse_args()

    path = prepare_path(args.filePrimary, args.fileSecondary, args.fileOut)

    path.add_module('Progress')
    basf2.process(path)
    print(basf2.statistics)
