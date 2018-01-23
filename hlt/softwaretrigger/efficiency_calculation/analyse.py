"""
Script to be called by gridcontrol to extract STM variables and results out of a reconstructed file.
"""

import basf2
import os

from softwaretrigger.variable_modules import SummarizeTriggerResults, SummarizeTriggerVariables, PickleHarvestingModule
from effCalculation import EffModule


class SummarizeL1TriggerResults(PickleHarvestingModule):
    def __init__(self, root_file_name="save_results.pkl", store_object_name="TRGSummary"):
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

        # note: this code only works if only the lower 32 trigger bits are used. See L1TriggerVariables.cc
        # on how to acces the upper ones. This code and L1TriggerVariables needs to be refactored to provide
        # one common method for easy trigger bit access
        summary = result.getTRGSummary(0)

        labels = EffModule.trglog_phase3
        # note for future change: prescales can also be read from TRGSummary directly
        prescales = EffModule.prescale_phase3

        if len(labels) > 32:
            basf2.B2FATAL("More than 32 l1 trigger bits not supported by this code.")

        return_dict = {"final_decision": summary >= 1}

        for i in range(len(labels)):
            trigger_decision = (summary & (1 << i) > 0)
            label = labels[i]
            prescale = prescales[i]

            return_dict[label] = trigger_decision
            return_dict[label + "_prescale"] = prescale
        yield return_dict


def main():
    # Get all parameters for this calculation
    input_file = os.environ.get("input_file")
    output_file = os.environ.get("output_file")

    output_variables_file = output_file.replace(".root", "_variables.pkl")
    output_results_file = output_file.replace(".root", "_hlt_results.pkl")
    output_l1_results_file = output_file.replace(".root", "_l1_results.pkl")

    log_file = output_file.replace(".root", ".log")

    path = basf2.create_path()

    path.add_module("RootInput", inputFileName=input_file)
    path.add_module(SummarizeTriggerResults(output_results_file))
    path.add_module(SummarizeL1TriggerResults(output_l1_results_file))
    path.add_module(SummarizeTriggerVariables(output_variables_file))

    basf2.log_to_file(log_file)
    basf2.print_path(path)
    basf2.process(path)


if __name__ == "__main__":
    main()
