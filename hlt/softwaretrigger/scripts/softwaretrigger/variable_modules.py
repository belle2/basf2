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

        # Make it easier for the user to get the total result of everything
        getFinalTriggerDecision = Belle2.SoftwareTrigger.FinalTriggerDecisionCalculator.getFinalTriggerDecision
        return_dict["final_decision"] = getFinalTriggerDecision(result)
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
    path.add_module("SeqRootInput", inputFileName="raw.sroot")

    path.add_module(SummarizeTriggerResults())
    path.add_module(SummarizeTriggerVariables())

    basf2.process(path)
