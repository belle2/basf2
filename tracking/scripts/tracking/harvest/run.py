import argparse
import pickle

from tracking.run.event_generation import StandardEventGenerationRun
from tracking.run.mixins import BrowseTFileOnTerminateRunMixin


class HarvestingRun(BrowseTFileOnTerminateRunMixin, StandardEventGenerationRun):
    output_file_name = None
    refiners_only = False

    def harvesting_module(self, path=None):
        raise RuntimeError("Override the harvesting_module method")

    def create_argument_parser(self, **kwds):
        argument_parser = super().create_argument_parser(**kwds)
        harvesting_argument_group = argument_parser.add_argument_group("Harvest arguments")

        harvesting_argument_group.add_argument(
            "-o",
            "--output",
            dest="output_file_name",
            default=argparse.SUPPRESS,
            help="File name for the harvest products"
        )

        harvesting_argument_group.add_argument(
            "-ro",
            "--refiners-only",
            action="store_true",
            help="Skip the creation of the crops and only rerun the refiners on the previously collected ones"
        )

        return argument_parser

    def pickle_crops(self, harvesting_module, crops, **kwds):
        "Save the raw crops as a pickle file"
        with open(self.output_file_name + ".pickle", "wb") as pickle_file:
            pickle.dump(crops, pickle_file)

    def unpickle_crops(self):
        "Load the raw crops from a pickle file"
        with open(self.output_file_name + ".pickle", "rb") as pickle_file:
            return pickle.load(pickle_file)

    def execute(self):
        if self.refiners_only:
            harvesting_module = self.harvesting_module()
            if self.output_file_name:
                harvesting_module.output_file_name = self.output_file_name
            try:
                crops = self.unpickle_crops()
            except FileNotFoundError:
                print("Crops pickle file not found. Create it now.")
            else:
                harvesting_module.show_results = self.show_results
                harvesting_module.refine(crops)
                return

        super().execute()

    def adjust_path(self, path):
        super().adjust_path(path)
        harvesting_module = self.harvesting_module()
        if self.output_file_name:
            harvesting_module.output_file_name = self.output_file_name
        harvesting_module.refiners.append(self.pickle_crops)
        path.add_module(harvesting_module)
        return path
