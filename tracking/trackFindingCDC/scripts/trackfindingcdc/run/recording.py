import basf2

from tracking.run.utilities import extend_path
from tracking.run.event_generation import StandardEventGenerationRun
from tracking.run.mixins import BrowseTFileOnTerminateRunMixin

import argparse


class RecordingRun(BrowseTFileOnTerminateRunMixin, StandardEventGenerationRun):
    recording_finder_module = basf2.register_module("TrackFinderCDCAutomaton")
    n_processes = 4

    flight_time_estimation = ""
    n_loops = float("nan")

    recording_filter_parameter_name = ""
    root_output_file_name = "Records.root"
    varsets = ["truth", ]
    skim = ""

    def create_argument_parser(self, **kwds):
        argument_parser = super().create_argument_parser(**kwds)

        argument_parser.add_argument(
            "-ft",
            "--flight-time-estimation",
            choices=["none", "outwards", "downwards"],
            default=argparse.SUPPRESS,
            dest="flight_time_estimation",
            help=("Choose which estimation method for the time of flight should be use. \n"
                  "* 'none' no time of flight corrections\n"
                  "* 'outwards' means the minimal time needed to travel to the wire from the interaction point \n"
                  "* 'downwards' means the minimal time needed to travel to the wire from the y = 0 plane downwards \n")
        )

        argument_parser.add_argument(
            "-l",
            "--mc-loop-limit",
            type=float,
            dest="n_loops",
            help=("Choose to block all wire hits but the ones located on the {mc_loops} first loops")
        )

        argument_parser.add_argument(
            '-sk',
            '--skim',
            dest='skim',
            default=argparse.SUPPRESS,
            help=('Name of the filter to skim recorded object')
        )

        argument_parser.add_argument(
            '-v',
            '--varset',
            nargs="+",
            dest='varsets',
            default=argparse.SUPPRESS,
            help=('Add a varset to the recording. Multiple repeatition adds more varsets.'
                  'If not given use the default settings of the run: %s' % self.varsets)
        )

        argument_parser.add_argument(
            '-o',
            '--root-output',
            default=argparse.SUPPRESS,
            dest='root_output_file_name',
            help='File to which the recorded varsets should be written',
        )

        return argument_parser

    def configure(self, arguments):
        super().configure(arguments)
        if self.recording_filter_parameter_name:
            self.recording_finder_module.param({
                self.recording_filter_parameter_name: {
                    "rootFileName": self.root_output_file_name,
                    "varSets": self.varsets,
                    "skim": self.skim
                    },
                })

    def create_path(self):
        # Sets up a path that plays back pregenerated events or generates events
        # based on the properties in the base class.
        path = super().create_path()

        wire_hit_preparer = path.add_module("WireHitPreparer",
                                            UseNLoops=self.n_loops)
        if self.flight_time_estimation:
            wire_hit_preparer.param(dict(flightTimeEstimation=self.flight_time_estimation))

        extend_path(path, self.recording_finder_module)
        return path
