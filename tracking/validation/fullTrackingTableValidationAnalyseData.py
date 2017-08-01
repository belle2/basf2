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
import os

try:
    from root_pandas import read_root
    import pandas as pd
except ImportError:
    basf2.B2FATAL("You need to have pandas installed for this validation script to run.")

FORMAT_STRING = "&nbsp;{:.2%} <br/> -{:.2%} <br/> <b>-{:.2%}</b> <br/> Matching: {:.2%} <br/> CDC: {:.2%} <br/> VXD: {:.2%}"
SHORT_FORMAT_STRING = "MC: &nbsp;{:.2%} <br/>Missing: -{:.2%} <br/>Missing and !fit: <b>-{:.2%}</b>"


def reducelist(list_of_cuts, df, current_name=None, current_cut=None, x=0, y=0):
    if current_name is not None:
        if y == 5:
            return_string = FORMAT_STRING.format(current_cut.mean(),
                                                 (current_cut & (df.is_matched == 0)).mean(),
                                                 (current_cut & (df.fitted_is_matched == 0)).mean(),
                                                 (current_cut & (df.both_related == 1)).mean(),
                                                 (current_cut & (df.cdc_has_related == 1)).mean(),
                                                 (current_cut & (df.vxd_has_related == 1)).mean())
        else:
            return_string = SHORT_FORMAT_STRING.format(current_cut.mean(),
                                                       (current_cut & (df.is_matched == 0)).mean(),
                                                       (current_cut & (df.fitted_is_matched == 0)).mean())
        yield (y, x, current_name), return_string

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


def make_chunks(l, n):
    return [l[i:i + n] for i in range(0, len(l), n)]


def write_value_cell(key, value):
    y, x, name, _ = key
    colspan = 2 ** int(5 - y)

    colors = {
        3: ["white", "gray", "orange", "green"],
        4: ["gray", "white", "gray", "gray",
            "orange", "orange", "green", "green"],
        5: ["gray", "gray", "red", "green",
            "gray", "gray", "gray", "gray",
            "red", "gray", "red", "orange",
            "green", "gray", "orange", "green"]
    }

    if y in colors:
        color_index = int((x - 2 ** 4) / (2 ** (5 - y)))
        color = colors[y][color_index]
    else:
        color = "white"

    return """
        <td style="border: 1px solid black" colspan={colspan}
        align="center" valign=middle bgcolor="{color}">{value}</td>
    """.format(colspan=colspan, color=color, value=value)


def make_html_row(x):
    keys = [key for key, _ in x.iteritems()]
    titles = [key[2] for key, _ in x.iteritems()]
    values = [value for _, value in x.iteritems()]

    chunked_titles = make_chunks(titles, 2)
    common_prefixes = list(map(os.path.commonprefix, chunked_titles))

    shorter_titles = [title.replace(prefix, "").replace("_", " ")
                      for list_titles, prefix in zip(chunked_titles, common_prefixes)
                      for title in list_titles]

    row_content = "".join([write_value_cell(key, value) for key, value in zip(keys, shorter_titles)])
    html = "<tr>" + row_content + "</tr>"

    row_content = "".join([write_value_cell(key, value) for key, value in x.sort_index().iteritems()])
    html += "<tr>" + row_content + "</tr>"

    return html


def get_html(df, test):
    results = pd.DataFrame(dict(reducelist(test, df)), index=[0]).unstack()

    last_row_titles = ["", "", "CDCTF may help", "Criteria?", "", "", "", "", "VXDTF may help", "",
                       "hard cases", "CKF may help", "Criteria?", "", "CKF may help", "Merging"]

    html = "<table>"
    html += "".join(results.groupby(level=0).apply(make_html_row))
    html += "<tr>" + ("".join(["<td>" + value + "</td>" for value in last_row_titles])) + "</tr>"
    html += "</table>"

    return html


if __name__ == '__main__':
    # These are the categories to be tested successively
    test = [
        ("all", None),
        ("has_vxd", lambda x: (x.n_svd_hits >= 2)),
        ("vxd_was_found", lambda x: x["vxd_was_found"] == 1),
        ("has_cdc", lambda x: x.n_cdc_hits >= 3),
        ("cdc_was_found", lambda x: x["cdc_was_found"] == 1),
    ]

    df = read_root("../matching_validation.root")
    html = get_html(df, test)

    tfile = TFile(VALIDATION_OUTPUT_FILE, "RECREATE")
    html_content = TNamed("Tracking Table Validation", html)
    html_content.Write()
    tfile.Close()
