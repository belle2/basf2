import numbers

import basf2
import collections
from ROOT import Belle2

from root_pandas import to_root
import pandas as pd
from tracking.harvest.harvesting import HarvestingModule, coroutine
from tracking.root_utils import root_save_name


class PandasHarvestingModule(HarvestingModule):
    @coroutine
    def barn(self):
        """Coroutine that receives the dictionaries of names and values from peel and store them."""
        crop = (yield)
        crops = pd.DataFrame()

        if isinstance(crop, numbers.Number):
            try:
                while True:
                    crops = crops.append(pd.DataFrame({"value": crop}, index=[0]), ignore_index=True)
                    # next crop
                    crop = (yield)
            except GeneratorExit:
                pass

        elif isinstance(crop, collections.MutableMapping):
            try:
                while True:
                    crops = crops.append(pd.DataFrame(crop, index=[0]), ignore_index=True)
                    crop = (yield)
            except GeneratorExit:
                pass

        else:
            msg = "Unrecognised crop {} of type {}".format(
                crop,
                type(crop)
            )
            raise ValueError(msg)

        self.crops = crops

    def refine(self, crops):
        """Receive the gathered crops and forward them to the refiners."""

        to_root(crops, tree_key=root_save_name(self.name()), path=self.output_file_name)


def pandas_harvest(foreach="", pick=None, name=None, output_file_name=None):
    def harvest_decorator(peel_func):
        name_or_default = name or peel_func.__name__
        output_file_name_or_default = output_file_name or "{}.root".format(name_or_default)
        harvesting_module = PandasHarvestingModule(foreach=foreach,
                                                   output_file_name=output_file_name_or_default,
                                                   name=name_or_default)
        harvesting_module.peel = peel_func
        if pick:
            harvesting_module.pick = pick
        return harvesting_module
    return harvest_decorator


class SummarizeTriggerVariables(basf2.Module):
    """
    Small helper module to export the software trigger variables saved in the data store to
    a ROOT file for later analysis.
    """

    def __init__(self, root_file_name="save_vars.root", store_object_name="SoftwareTriggerVariable"):
        """
        Create a new SummarizeTriggerVariables module.
        :param root_file_name: The root file name to save to results to.
        :param store_object_name: The name of the object in the data store from where to get the variables.
        """
        basf2.Module.__init__(self)

        self.df = pd.DataFrame()
        self.root_file_name = root_file_name
        self.store_object_name = store_object_name

        self.event_number_counter = 0

    def event(self):
        """
        Extract the variables from the data store and save them in an internal pandas data frame.
        Also increment the internal event number counter.
        """
        variables = Belle2.PyStoreObj(self.store_object_name)

        return_dict = {identifier: value for identifier, value in variables.get()}
        self.df = self.df.append(pd.DataFrame(return_dict, index=[self.event_number_counter]))

        self.event_number_counter += 1

    def terminate(self):
        """
        Save the internal pandas data frame to a ROOT file.
        """
        to_root(self.df, self.store_object_name)
