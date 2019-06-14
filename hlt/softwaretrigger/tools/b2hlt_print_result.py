#!/usr/bin/env python3
from ROOT import PyConfig
PyConfig.IgnoreCommandLineOptions = True
PyConfig.StartGuiThread = False

import basf2
from argparse import ArgumentParser
from root_pandas import read_root
import pandas as pd
import numpy as np


if __name__ == "__main__":
    parser = ArgumentParser(description="Print the results of the SoftwareTrigger decision for a certain file.")
    parser.add_argument("input", help="Input file name (where to read the events from)")
    parser.add_argument("--output", help="Output file name (will be used internally). "
                                         "Defaults to trigger_results.root.",
                        default="trigger_results.root")
    choices = ["human-readable"]
    try:
        from tabulate import tabulate
        choices += ['jira', 'grid', 'stash']
    except ImportError:
        pass

    parser.add_argument("--format", help="Choose the format how to print the trigger cuts. "
                        "To get access to more options please install the tabulate package using pip",
                        choices=choices, default="human-readable")

    args = parser.parse_args()

    # The prescales are only valid when using the online database!
    basf2.reset_database()
    basf2.use_central_database("online")

    path = basf2.Path()

    if args.input.endswith(".sroot"):
        path.add_module("SeqRootInput", inputFileName=args.input)
    else:
        path.add_module("RootInput", inputFileName=args.input)
    path.add_module("SoftwareTriggerResultPrinter", outputFileName=args.output)

    basf2.process(path)

    df = read_root(args.output)

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
    df_print["Prescales"] = df_prescales.fillna("")

    if args.format == "human-readable":
        print(df_print[["Prescaled", "Non Prescaled", "Prescales"]])
    elif args.format == "jira":
        print(tabulate(df_print, tablefmt="jira", showindex=True, headers="keys"))
    elif args.format == "stash":
        print(tabulate(df_print, tablefmt="pipe", showindex=True, headers="keys"))
    elif args.format == "grid":
        print(tabulate(df_print, tablefmt="grid", showindex=True, headers="keys"))
