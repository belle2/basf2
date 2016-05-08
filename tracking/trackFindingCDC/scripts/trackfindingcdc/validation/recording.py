import basf2

from tracking.run.event_generation import StandardEventGenerationRun
from tracking.run.mixins import BrowseTFileOnTerminateRunMixin


class RecordingRun(BrowseTFileOnTerminateRunMixin, StandardEventGenerationRun):
    recording_finder_module = basf2.register_module("TrackFinderCDCAutomatonDev")
    flight_time_estimation = "none"

    recording_filter_parameter_name = "FillMeFilterParameters"
    root_output_file_name = "Records.root"
    varsets = ["truth", ]
    skim = ""

    def create_argument_parser(self, **kwds):
        argument_parser = super(RecordingRun, self).create_argument_parser(**kwds)

        argument_parser.add_argument(
            "-ft",
            "--flight-time-estimation",
            choices=["none", "outwards", "downwards"],
            default=self.flight_time_estimation,
            dest="flight_time_estimation",
            help=("Choose which estimation method for the time of flight should be use. \n"
                  "* 'none' no time of flight corrections\n"
                  "* 'outwards' means the minimal time needed to travel to the wire from the interaction point \n"
                  "* 'downwards' means the minimal time needed to travel to the wire from the y = 0 plane downwards \n")
        )

        argument_parser.add_argument(
            '-v',
            '--varset',
            nargs="+",
            dest='varsets',
            default=self.varsets,
            help=('Add a varset to the recording. Multiple repeatition adds more varsets.'
                  'If not given use the default settings of the run: %s' % self.varsets)
        )

        argument_parser.add_argument(
            '--root-output',
            default=self.root_output_file_name,
            dest='root_output_file_name',
            help='File to which the recorded varsets should be written',
        )

        argument_parser.add_argument(
            '-sk',
            '--skim',
            dest='skim',
            default=self.skim,
            help=('Name of the filter to skim recorded object')
        )

        return argument_parser

    def configure(self, arguments):
        super(RecordingRun, self).configure(arguments)

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
        path = super(RecordingRun, self).create_path()

        path.add_module("WireHitTopologyPreparer",
                        flightTimeEstimation=self.flight_time_estimation)

        recording_finder_module = self.get_basf2_module(self.recording_finder_module)
        main_path.add_module(recording_finder_module)
        return main_path
