import basf2

from tracking.run.event_generation import StandardEventGenerationRun
from tracking.run.mixins import BrowseTFileOnTerminateRunMixin


class RecordingRun(BrowseTFileOnTerminateRunMixin, StandardEventGenerationRun):
    recording_finder_module = basf2.register_module("TrackFinderCDCAutomatonDev")

    recording_filter_parameter_name = "FillMeFilterParameters"
    root_output_file_name = "Records.root"
    varsets = ["truth", ]

    def create_argument_parser(self, **kwds):
        argument_parser = super(RecordingRun, self).create_argument_parser(**kwds)

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

        return argument_parser

    def configure(self, arguments):
        super(RecordingRun, self).configure(arguments)

        self.recording_finder_module.param({
            self.recording_filter_parameter_name: {
                "rootFileName": self.root_output_file_name,
                "varSets": self.varsets,
            },
        })

    def create_path(self):
        # Sets up a path that plays back pregenerated events or generates events
        # based on the properties in the base class.
        main_path = super(RecordingRun, self).create_path()

        main_path.add_module("WireHitTopologyPreparer")

        recording_finder_module = self.get_basf2_module(self.recording_finder_module)
        main_path.add_module(recording_finder_module)
        return main_path
