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


class SummarizeTriggerResults(PandasHarvestingModule):

    def __init__(self, root_file_name="save_results.root", store_object_name="SoftwareTriggerResults"):
        super().__init__(foreach=store_object_name, output_file_name=root_file_name)

    def peel(self, result):
        return_dict = {identifier: result for identifier, result in result.getResults()}

        return_dict["total_result_true"] = result.getTotalResult(True)
        return_dict["total_result_false"] = result.getTotalResult(False)

        yield return_dict


class SummarizeTriggerVariables(PandasHarvestingModule):

    def __init__(self, root_file_name="save_vars.root", store_object_name="SoftwareTriggerVariable"):
        super().__init__(foreach=store_object_name, output_file_name=root_file_name)

    def peel(self, variables):
        yield {identifier: value for identifier, value in variables.get()}
