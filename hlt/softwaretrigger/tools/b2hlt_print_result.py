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
    parser.add_argument(
        "input",
        help="Input file name (where to read the events from). "
        "If omitted, just use the already produced result by another SoftwareTriggerResultPrinter execution",
        default="",
        nargs="?")
    parser.add_argument("--output", help="Output file name (will be used internally). "
                                         "Defaults to trigger_results.root.",
                        default="software_trigger_results.root")
    choices = ["human-readable", "categorized"]
    try:
        from tabulate import tabulate
        choices += ['jira', 'grid', 'stash']
    except ImportError:
        pass

    parser.add_argument("--format", help="Choose the format how to print the trigger cuts. "
                        "To get access to more options please install the tabulate package using pip",
                        choices=choices, default="categorized")

    args = parser.parse_args()

    if args.input:
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
    df_print = df_print[["Prescaled", "Non Prescaled", "Prescales"]]

    if args.format == "human-readable":
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
            local_print_function("ECL - Prescaled", filter_categories.ECL_PRESCALED),
            local_print_function("CDC - Physics", filter_categories.CDC_PHYSICS),
            local_print_function("CDC - Prescaled", filter_categories.CDC_PRESCALED),
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
