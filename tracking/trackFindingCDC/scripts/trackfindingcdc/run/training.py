##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import os
import os.path
import shutil
import subprocess

from tracking.run.mixins import BrowseTFileOnTerminateRunMixin
from tracking.run.mixins import PostProcessingRunMixin
from tracking.run.utilities import NonstrictChoices


class TrainingRunMixin(BrowseTFileOnTerminateRunMixin, PostProcessingRunMixin):
    """Prepare and execute a basf2 job to train neural network, postprocess, and inspect"""

    #: Recording / training task selected
    task = "train"

    #: Input variable for the training or the classification analysis. Default use all variables
    variables = None

    #: Input groupby the classification analysis. Default use no groupby
    groupby = None

    #: Input auxiliaries the classification analysis. Default use none
    auxiliaries = None

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
        #: Process each event according to the user's desired task (train, eval, explore)
        if self.task == "train":
            cmd = [
                "trackfindingcdc_teacher",
            ]

            if self.variables:
                cmd += ["--variables"]
                cmd += self.variables

            cmd += [
                "--identifier=" + self.identifier,
                "--truth=" + self.truth,
                self.sample_file_name,
            ]
            print("Running", cmd)
            subprocess.call(cmd)

            # Move training file to the right location
            if self.identifier.endswith(".xml"):
                tracking_data_dir_path = os.path.join(os.environ["BELLE2_LOCAL_DIR"], "tracking", "data")
                shutil.copy(self.identifier, tracking_data_dir_path)

        else:
            cmd = [
                "trackfindingcdc-classification-overview",
            ]

            if self.variables:
                cmd += ["-v"]
                cmd += self.variables

            if self.groupby is not None:
                cmd += ["-g"]
                if isinstance(self.groupby, str):
                    cmd += [self.groupby]
                else:
                    cmd += self.groupby

            if self.auxiliaries is not None:
                cmd += ["-a"]
                if isinstance(self.auxiliaries, str):
                    cmd += [self.auxiliaries]
                else:
                    cmd += self.auxiliaries

            cmd += [
                "--truth=" + self.truth,
                self.sample_file_name,
            ]
            print("Running", cmd)
            subprocess.call(cmd)
            #: Set file name for the TBrowser to show if demanded
            self.output_file_name = self.sample_file_name[:-len(".root")] + ".overview.root"

        super().postprocess()
