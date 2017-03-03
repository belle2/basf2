import os
import os.path
import shutil
import subprocess

from tracking.run.mixins import BrowseTFileOnTerminateRunMixin
from tracking.run.mixins import PostProcessingRunMixin
from tracking.run.utilities import NonstrictChoices


class TrainingRunMixin(BrowseTFileOnTerminateRunMixin, PostProcessingRunMixin):

    #: Recording / training task selected
    task = "train"

    #: Truth variable name
    truth = "truth"

    @property
    def identifier(self):
        """Database identifier of the filte being trained"""
        return "trackfindingcdc_" + self.__class__.__name__[:-len("TrainingRun")]

    @property
    def sample_file_name(self):
        """File name of the recorded sample to be trained on

        Defaults to the class name minus the mandatory TrainingRun postfix
        """
        return self.__class__.__name__[:-len("TrainingRun")] + '_' + self.task + '.root'

    def create_argument_parser(self, **kwds):
        """Create argument parser"""
        argument_parser = super().create_argument_parser(**kwds)

        argument_parser.add_argument(
            "--task",
            choices=NonstrictChoices(["train", "eval", "explore", ]),
            default=self.task,
            dest="task",
            help=("Select a prepared recording task")
        )

        return argument_parser

    def postprocess(self):
        """Run the training as post-processing job

        To run only the training run with --postprocess-only
        """
        if self.task == "train":
            cmd = [
                "trackfindingcdc_teacher",
                "--identifier=" + self.identifier,
                "--truth=" + self.truth,
                self.sample_file_name,
            ]
            subprocess.call(cmd)

            # Move training file to the right location
            if self.identifier.endswith(".xml"):
                tracking_data_dir_path = os.path.join(os.environ["BELLE2_LOCAL_DIR"], "tracking", "data")
                shutil.copy(self.identifier, tracking_data_dir_path)

        else:
            cmd = [
                "trackfindingcdc-classification-overview",
                "--truth=" + self.truth,
                self.sample_file_name,
            ]
            subprocess.call(cmd)
            # Set file name for the TBrowser to show if demanded
            self.output_file_name = self.sample_file_name[:-len(".root")] + ".overview.root"

        super().postprocess()
