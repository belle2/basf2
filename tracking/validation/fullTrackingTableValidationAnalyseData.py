#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact>tracking@belle2.kek.jp</contact>
  <output>fullTrackingValidationTable.root</output>
  <input>matching_validation.root</input>
  <description>This module generates events for the validation using the full tracking with a tabular output.</description>
</header>
"""
import basf2
from ROOT import TFile, TNamed, Belle2

VALIDATION_OUTPUT_FILE = "fullTrackingTableValidation.root"

try:
    from root_pandas import read_root
    import pandas as pd
except ImportError:
    basf2.B2FATAL("You need to have pandas installed for this validation script to run.")


def load_data():
    df_matching = read_root("../matching_validation.root")
    df_tracking = read_root("../tracking_validation.root", tree_key="mc_tree/mc_tree")
    df_tracking_prefit = read_root("../tracking_validation_prefit.root", tree_key="mc_tree/mc_tree")

    df_fitted = pd.merge(df_matching, df_tracking, left_index=True, right_index=True)
    df_prefit = pd.merge(df_matching, df_tracking_prefit, left_index=True, right_index=True)

    return df_fitted, df_prefit


def reducelist(list_of_cuts, df, current_name=None, current_cut=None, x=0, y=0):
    if current_name is not None:
        yield (y, x, current_name + "_missing"), (
            current_cut & ((df.is_merged == 1) | (df.is_missing == 1))).sum()
        yield (y, x, current_name), current_cut.sum()

    if not list_of_cuts:
        return

    name, cut = list_of_cuts[0]

    if cut is None:
        # Make a "always true" cut
        def cut(x):
            return x.is_missing == x.is_missing

    if current_name is None:
        yield from reducelist(list_of_cuts[1:], df, name, cut(df),
                              x + 2 ** (len(list_of_cuts) - 1), y + 1)
    else:
        yield from reducelist(list_of_cuts[1:], df, current_name + "_no_" + name, current_cut & (~cut(df)),
                              x, y + 1)
        yield from reducelist(list_of_cuts[1:], df, current_name + "_" + name, current_cut & (cut(df)),
                              x + 2 ** (len(list_of_cuts) - 1), y + 1)


def get_result(df, test):
    """
    Take a data frame with matching results and successively apply the cuts defined in test
    to the data. In each step, store the amount of rows that have survived or not survived the
    cut and apply on *both* the next cut (so we end up with 2^(number of cuts) categories in the end).
    """
    results = pd.DataFrame(dict(reducelist(test, df[df.is_primary == 1])), index=[0])
    results = results.unstack()
    results = pd.DataFrame(results).set_index(results.index.get_level_values(2)).T
    return results


if __name__ == '__main__':
    df_fitted, df_prefit = load_data()

    # These are the categories to be tested successively
    test = [
        ("all", None),
        ("has_vxd", lambda x: (x.n_svd_hits >= 2) | (x.n_pxd_hits >= 2)),
        ("vxd_was_found", lambda x: x["vxd_was_found"] == 1),
        ("has_cdc", lambda x: x.n_cdc_hits >= 3),
        ("cdc_was_found", lambda x: x["cdc_was_found"] == 1),
    ]

    # Retrieve the results for both data frames
    results_fitted = get_result(df_fitted, test)
    results_prefit = get_result(df_prefit, test)

    # Merge the data frames together
    results_fitted.columns = [col + "_fitted" for col in results_fitted.columns]
    results = pd.merge(results_prefit, results_fitted, left_index=True, right_index=True)

    # Calculate the percentages
    for col in results.columns:
        if "missing" in col:
            results[col + "_percentage"] = 100 * results[col] / results["all_missing"]
        else:
            results[col + "_percentage"] = 100 * results[col] / results["all"]

    # Write back the results into an HTML table
    with open(Belle2.FileSystem.findFile("tracking/validation/table.html"), "r") as f:
        content = f.read()

    tfile = TFile(VALIDATION_OUTPUT_FILE, "RECREATE")
    html_content = TNamed("Tracking Table Validation", content.format(**dict(zip(results.columns, results.values[0]))))
    html_content.Write()
    tfile.Close()
