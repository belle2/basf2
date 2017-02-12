import numbers
import pickle

import basf2
import collections
from ROOT import Belle2

from tracking.harvest.harvesting import HarvestingModule, coroutine
from tracking.root_utils import root_save_name


# Helper modules to get the stored information of the SoftwareTriggerModule from the data store after the processing
# and write them out to a ROOT file for analysis. Uses the tracking harvester for this.


class PickleHarvestingModule(HarvestingModule):
    """
    Overloaded harvester, that stores its data into a pandas data frame instead of a numpy array,
    because they are more flexible when it comes to changing columns and value types to be stored.
    """
    @coroutine
    def barn(self):
        """Coroutine that receives the dictionaries of names and values from peel and store them into a pandas df."""
        crop = (yield)
        crops = []

        if isinstance(crop, numbers.Number):
            try:
                while True:
                    crops.append({"value": crop})
                    # next crop
                    crop = (yield)
            except GeneratorExit:
                pass

        elif isinstance(crop, collections.MutableMapping):
            try:
                while True:
                    crops.append(crop)
                    crop = (yield)
            except GeneratorExit:
                pass

        else:
            msg = "Unrecognised crop {} of type {}".format(
                crop,
                type(crop)
            )
            raise ValueError(msg)

        #: The gathered crops until now
        self.crops = crops

    def refine(self, crops):
        """Receive the gathered crops and saves them into a ROOT file."""
        with open(self.output_file_name, "wb") as f:
            pickle.dump(crops, f)


class SummarizeTriggerResults(PickleHarvestingModule):
    """
    Read in the trigger results stored in the data store and write them out into a ROOT file after all events have
    passed.
    """

    def __init__(self, root_file_name="save_results.pkl", store_object_name="SoftwareTriggerResult"):
        """
        Create a new module to get the stored trigger decisions from the data store and save them to a root file.
        :param root_file_name: The file name where to store the results.
        :param store_object_name: The store object name where the trigger results an be found in the data store.
        """
        super().__init__(foreach=store_object_name, output_file_name=root_file_name)

    def peel(self, result):
        """
        For each result ( = event), get the list of all trigger decisions and also add the total decision.
        Write them back into the internal storage ( = pandas.DataFrame) of all events.
        """
        return_dict = {identifier: result for identifier, result in result.getResults()}

        # Write out some convenience variables: the fast reco/hlt/calibration and the full decision
        fast_reco_result = Belle2.SoftwareTriggerResult()
        hlt_result = Belle2.SoftwareTriggerResult()
        calib_result = Belle2.SoftwareTriggerResult()

        for trigger_name, trigger_result in return_dict.items():
            if "fast_reco" in trigger_name:
                result = fast_reco_result
            elif "hlt" in trigger_name:
                result = hlt_result
            elif "calib" in trigger_name:
                result = calib_result
            else:
                raise ValueError("Do not know", trigger_name)

            result.addResult(trigger_name, Belle2.SoftwareTriggerCutResult(trigger_result))

        return_dict["fast_reco_result"] = fast_reco_result.getTotalResult(True) != -1
        return_dict["hlt_result"] = hlt_result.getTotalResult(False) == 1
        return_dict["calib_result"] = calib_result.getTotalResult(False) == 1

        return_dict["total_result"] = return_dict["fast_reco_result"] & return_dict["hlt_result"]

        yield return_dict


class SummarizeTriggerVariables(PickleHarvestingModule):
    """
    Read in the trigger variables stored in the data store and write them out into a ROOT file after all events have
    passed.
    """

    def __init__(self, root_file_name="save_vars.pkl", store_object_name="SoftwareTriggerVariables"):
        """
        Create a new module to get the stored trigger variables from the data store and save them to a root file.
        :param root_file_name: The file name where to store the variables.
        :param store_object_name: The store object name where the trigger variables an be found in the data store.
        """
        super().__init__(foreach=store_object_name, output_file_name=root_file_name)

    def peel(self, variables):
        """
        For each collection of variables ( = event), get the list of all variables.
        Write them back into the internal storage ( = pandas.DataFrame) of all events.
        """
        yield {identifier: value for identifier, value in variables.get()}

if __name__ == "__main__":
    path = basf2.create_path()

    # Take the output of the __init__.py script
    path.add_module("RootInput", inputFileName="output.root")

    path.add_module(SummarizeTriggerResults())
    path.add_module(SummarizeTriggerVariables())

    basf2.process(path)
