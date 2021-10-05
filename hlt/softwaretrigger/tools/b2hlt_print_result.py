#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
from ROOT import PyConfig
PyConfig.IgnoreCommandLineOptions = True  # noqa
PyConfig.StartGuiThread = False  # noqa

import basf2
from argparse import ArgumentParser
import uproot
import pandas as pd
import numpy as np


if __name__ == "__main__":
    parser = ArgumentParser(description="Print the results of the SoftwareTrigger decision for a certain file.")
    parser.add_argument(
        "input",
        help="Input file name (where to read the events from). "
        "If omitted, just use the already produced result by another SoftwareTriggerResultPrinter execution",
        default="",
        nargs="?")
    parser.add_argument("--output", help="Output file name (will be used internally). "
                                         "Defaults to trigger_results.root.",
                        default="software_trigger_results.root")
    choices = ["list", "categorized"]
    try:
        from tabulate import tabulate
        choices += ['jira', 'grid', 'stash']
    except ImportError:
        pass

    parser.add_argument("--format", help="Choose the format how to print the trigger cuts. "
                        "To get access to more options please install the tabulate package using pip",
                        choices=choices, default="categorized")
    parser.add_argument("--override-globaltags", dest="override", action="store_true", default=False,
                        help="Use this option in case the data file does not provide globaltag information. "
                             "The only case where this should occur is when analyzing raw data.")
    parser.add_argument('--local-db-path', type=str,
                        help="set path to the local payload locations to use for the ConditionDB",
                        default=None)

    args = parser.parse_args()

    if args.input:
        # For data, the prescales are only valid when using the online database!
        if args.local_db_path is not None:
            basf2.conditions.metadata_providers = ["file://" + basf2.find_file(args.local_db_path + "/metadata.sqlite")]
            basf2.conditions.payload_locations = [basf2.find_file(args.local_db_path)]

        if args.override:
            basf2.conditions.override_globaltags(["online"])

        path = basf2.Path()

        if args.input.endswith(".sroot"):
            path.add_module("SeqRootInput", inputFileName=args.input)
        else:
            path.add_module("RootInput", inputFileName=args.input)
        path.add_module("SoftwareTriggerResultPrinter", outputFileName=args.output)

        basf2.process(path)

    df = uproot.open(args.output)["software_trigger_results"].arrays(library="pd")

    # Make sure to cope with strings rather than bools (which is a bit strange in pandas)
    df[["accept_or_reject", "prescaled", "cut"]] = df[["accept_or_reject", "prescaled", "cut"]].astype("str")

    # Group and order as we need it
    df = df.set_index(["cut", "accept_or_reject", "prescaled"]).T
    df.index = df.index.str.replace("software_trigger_cut_", "")
    df.index = df.index.str.replace("_", " ")

    # Separate cuts and prescaling
    df_prescales = df["False"].copy()
    df_cuts = df["True"].copy()

    # For the prescaling, the total_events is nonsense...
    df_prescales.loc["total events"] = np.NAN

    # Now also separate out only the accepted results
    df_cuts = df_cuts["True"].copy()

    # Give the columns some meaningful names
    df_cuts = df_cuts[["True", "False"]]
    df_cuts.columns = ["Prescaled", "Non Prescaled"]

    # Make sure to print all information
    pd.set_option("display.max_rows", 500)
    pd.set_option("display.max_colwidth", 200)
    pd.set_option('display.max_columns', 500)
    pd.set_option('display.width', 1000)

    # Function used for formatting
    def format(x, total_events):
        if np.isnan(x):
            return ""
        return f"{int(x):d} ({x/total_events:7.2%})"

    # Create a new dataframe just for printing
    df_print = pd.DataFrame(index=df_cuts.index)

    df_print["Prescaled"] = df_cuts["Prescaled"].apply(lambda x: format(x, df_cuts["Prescaled"]["total events"]))
    df_print["Non Prescaled"] = df_cuts["Non Prescaled"].apply(lambda x: format(x, df_cuts["Non Prescaled"]["total events"]))
    df_print["Prescales"] = df_prescales.fillna("NaN")
    df_print = df_print[["Prescaled", "Non Prescaled", "Prescales"]]

    if args.format == "list":
        print(df_print)
    elif args.format == "categorized":
        from softwaretrigger import filter_categories

        def local_print_function(title, categories):
            empty_row = {key: "" for key in df_print.columns}
            tmp = pd.DataFrame(columns=df_print.columns)
            tmp = tmp.append(pd.Series(empty_row, name=title))
            tmp = tmp.append(df_print.reindex(categories))
            tmp = tmp.append(pd.Series(empty_row, name=""))

            return tmp

        df_sorted = pd.concat([
            local_print_function("Overview", filter_categories.RESULTS),
            local_print_function("ECL - Physics", filter_categories.ECL_PHYSICS),
            local_print_function("ECL - Potentially Prescaled", filter_categories.ECL_PRESCALED),
            local_print_function("CDC - Physics", filter_categories.CDC_PHYSICS),
            local_print_function("CDC - Potentially Prescaled", filter_categories.CDC_PRESCALED),
            local_print_function("Targeted Physics Lines", filter_categories.PHYSICS),
            local_print_function("QED / Control Samples", filter_categories.QED),
            local_print_function("Level 1 Passthrough ", filter_categories.LEVEL1),
            local_print_function("Prescaled Vetoes", filter_categories.VETOES),
            local_print_function("Skims", [index for index in df_print.index if index.startswith("skim ")]),
        ])

        remaining_columns = set(df_print.index) - set(df_sorted.index)
        if remaining_columns:
            df_sorted = df_sorted.append(local_print_function("Uncategorized", remaining_columns))

        print(df_sorted)

    elif args.format == "jira":
        print(tabulate(df_print, tablefmt="jira", showindex=True, headers="keys"))
    elif args.format == "stash":
        print(tabulate(df_print, tablefmt="pipe", showindex=True, headers="keys"))
    elif args.format == "grid":
        print(tabulate(df_print, tablefmt="grid", showindex=True, headers="keys"))
