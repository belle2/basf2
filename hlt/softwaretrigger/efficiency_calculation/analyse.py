"""
Script to be called by gridcontrol to extract STM variables and results out of a reconstructed file.
"""

import basf2
import os

from softwaretrigger.variables import SummarizeTriggerResults, SummarizeTriggerVariables


def main():
    # Get all parameters for this calculation
    input_file = os.environ.get("input_file")
    output_file = os.environ.get("output_file")

    output_variables_file = output_file.replace(".root", "_variables.pkl")
    output_results_file = output_file.replace(".root", "_results.pkl")

    log_file = output_file.replace(".root", ".log")

    path = basf2.create_path()

    path.add_module("RootInput", inputFileName=input_file)
    path.add_module(SummarizeTriggerResults(output_results_file))
    path.add_module(SummarizeTriggerVariables(output_variables_file))

    basf2.log_to_file(log_file)
    basf2.print_path(path)
    basf2.process(path)


if __name__ == "__main__":
    main()
