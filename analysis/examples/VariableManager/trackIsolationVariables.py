#!/usr/bin/env python3

# Doxygen should skip this script
# @cond

"""
Example script to calculate track isolation variables.

For each particle's track in the input charged stable particle list,
calculate the minimal distance to the other candidates' tracks at a given detector entry surface.
"""

__author__ = 'Marco Milesi'
__copyright__ = 'Copyright 2020 - Belle II Collaboration'
__maintainer__ = 'Marco Milesi'
__email__ = 'marco.milesi@unimelb.edu.au'


import argparse


def argparser():
    """ Argument parser
    """

    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawTextHelpFormatter)

    parser.add_argument("--std_charged",
                        type=str,
                        nargs="+",
                        choices=["e", "mu", "pi", "K"],
                        default=["e", "mu", "pi", "K"],
                        help="The base name of the standard charged particle list to consider.")
    parser.add_argument("--detectors",
                        type=str,
                        nargs="+",
                        default=["CDC", "PID", "ECL", "KLM"],
                        choices=["CDC", "PID", "ECL", "KLM"],
                        help="List of detectors at whose entry surface track isolation variables will be calculated.\n"
                        "Pass a space-separated list of names.\n"
                        "NB: 'PID' indicates TOP+ARICH entry surface.")
    parser.add_argument("-d", "--debug",
                        action="store",
                        default=0,
                        type=int,
                        choices=list(range(11, 20)),
                        help="Run the TrackIsoCalculator module in debug mode. Pass the desired DEBUG level integer.")

    return parser


def register_trackiso_modules(path, args, charges=["+", "-"]):
    """
    Prepare basf2 path.

    Args:
        path (basf2.Path): the input path.
        args (Namespace): the CLI arguments.
        charges (Optional[list(str)]): the particle list charges to check.
                                       Default: ['+', '-']
                                       Note that this is just to test module behaviour against different user's input:
                                       in fact, for any choice of particle list's charge, the charge-conjugated one
                                       gets loaded automatically.

    Returns:
        basf2.Path: the updated path.
    """

    ntup_vars = [f"dist3DToClosestTrkAtSurface{det}" for det in args.detectors]
    ntup_vars += [f"dist2DRhoPhiToClosestTrkAtSurface{det}" for det in args.detectors]

    for std_charged in args.std_charged:

        for charge in charges:

            # Fill a particle list of charged stable particles.
            # Apply (optionally) some quality selection.
            plist_name = f"{std_charged}{charge}:my_std_charged"
            ma.fillParticleList(plist_name, "", path=path)
            ma.applyCuts(plist_name, "abs(dr) < 2.0 and abs(dz) < 5.0 and p > 0.1", path=path)

            # 3D distance (default).
            ma.calculateTrackIsolation(plist_name,
                                       path,
                                       *args.detectors,
                                       alias="dist3DToClosestTrkAtSurface")
            # 2D distance on rho-phi plane (chord length).
            ma.calculateTrackIsolation(plist_name,
                                       path,
                                       *args.detectors,
                                       use2DRhoPhiDist=True,
                                       alias="dist2DRhoPhiToClosestTrkAtSurface")

            # Dump isolation variables in a ntuple.
            ma.variablesToNtuple(plist_name,
                                 ntup_vars,
                                 treename=f"{std_charged}{charge}",
                                 filename="TrackIsolationVariables.root",
                                 path=path)

    return path


if __name__ == "__main__":

    # Argparse options.
    #
    # NB: Command line arguments are parsed before importing basf2, to avoid PyROOT hijacking them
    # in case of overlapping option names.
    args = argparser().parse_args()

    import basf2 as b2
    import modularAnalysis as ma

    # Create path. Register necessary modules to this path.
    path = b2.create_path()

    # Add input data and ParticleLoader modules to the path.
    ma.inputMdstList("default", filelist=[b2.find_file("mdst14.root", "validation")], path=path)

    path = register_trackiso_modules(path, args)

    # Optionally activate debug mode for the TrackIsoCalculator module(s).
    if args.debug:
        for m in path.modules():
            if "TrackIsoCalculator" in m.name():
                m.set_log_level(b2.LogLevel.DEBUG)
                m.set_debug_level(args.debug)

    path.add_module("Progress")

    # Process the data.
    b2.process(path)

    print(b2.statistics)

# @endcond
