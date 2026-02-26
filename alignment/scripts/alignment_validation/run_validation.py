##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
"""
Entry point for alignment validation.

Usage:
    python -m alignment_validation cosmics FILE [FILE ...] [--output-dir DIR] [--format FMT]
    python -m alignment_validation dimuon  FILE [FILE ...] [--output-dir DIR] [--format FMT]
                                           [--ip-correction {run_by_run,per_event}]
"""

import argparse

from alignment_validation import cosmics
from alignment_validation import dimuon


def main():
    """Parse command-line arguments and run the requested validation.

    Subcommands
    -----------
    cosmics
        Run cosmic-track validation via :func:`alignment_validation.cosmics.run_validation`.
    dimuon
        Run di-muon validation via :func:`alignment_validation.dimuon.run_validation`.
    """
    parser = argparse.ArgumentParser(
        description="Alignment validation for Belle II track reconstruction."
    )
    sub = parser.add_subparsers(dest="command", required=True)

    for name, default_out in [("cosmics", "cosmics_plots"), ("dimuon", "dimuon_plots")]:
        p = sub.add_parser(name, help=f"Run {name} validation.")
        p.add_argument("files", nargs="+", help="Input ROOT file(s).")
        p.add_argument("--output-dir", default=default_out, help="Output directory for plots.")
        p.add_argument("--format", default="pdf", dest="file_format", help="Plot image format.")
        if name == "dimuon":
            p.add_argument(
                "--ip-correction",
                default="run_by_run",
                dest="ip_correction",
                choices=["run_by_run", "per_event"],
                help=(
                    "IP correction strategy: 'run_by_run' (default) averages the IP over each "
                    "(experiment, run) pair; 'per_event' uses the per-event IP directly."
                ),
            )

    args = parser.parse_args()

    if args.command == "cosmics":
        cosmics.run_validation(args.files, args.output_dir, args.file_format)
    elif args.command == "dimuon":
        dimuon.run_validation(args.files, args.output_dir, args.file_format, args.ip_correction)


if __name__ == "__main__":
    main()
