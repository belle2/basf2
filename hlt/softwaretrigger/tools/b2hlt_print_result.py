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
    parser.add_argument("--also-rejected", action="store_true", default=False)
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

    path = basf2.Path()

    if args.input.endswith(".sroot"):
        path.add_module("SeqRootInput", inputFileName=args.input)
    else:
        path.add_module("RootInput", inputFileName=args.input)
    path.add_module("SoftwareTriggerResultPrinter", outputFileName=args.output)

    basf2.process(path)

    df = read_root(args.output)

    df.accept_or_reject = df.accept_or_reject.astype("str")
    df.prescaled = df.prescaled.astype("str")

    df = df.set_index(["accept_or_reject", "prescaled"]).T
    df.index = df.index.str.replace("software_trigger_cut_", "")
    df.index = df.index.str.replace("_", " ")

    # TODO: also-rejected
    df = df["True"]
    df = df[["True", "False"]]
    df.columns = ["Prescaled", "Non Prescaled"]

    pd.set_option("display.max_rows", 500)
    pd.set_option("display.max_colwidth", 200)

    def format(x, total_events):
        if np.isnan(x):
            return ""
        return f"{x} ({x/total_events:.2%})"

    df["Prescaled"] = df["Prescaled"].apply(lambda x: format(x, df["Prescaled"]["total events"]))
    df["Non Prescaled"] = df["Non Prescaled"].apply(lambda x: format(x, df["Non Prescaled"]["total events"]))

    if args.format == "human-readable":
        print(df[["Prescaled", "Non Prescaled"]])
    elif args.format == "jira":
        print(tabulate(df, tablefmt="jira", showindex=True, headers="keys"))
    elif args.format == "stash":
        print(tabulate(df, tablefmt="pipe", showindex=True, headers="keys"))
    elif args.format == "grid":
        print(tabulate(df, tablefmt="grid", showindex=True, headers="keys"))
