import os
import shutil
import subprocess
from io import StringIO
from glob import glob

import ROOT
import pandas as pd

latex_template = """\\documentclass[a0paper,11pt,landscape]{article}
\\usepackage[landscape,margin=1.4cm]{geometry}
\\usepackage{siunitx}
\\usepackage{booktabs}
\\title{\\textbf{%%%title%%%}}
\\author{Belle II Trigger Efficiency Calculation Tool}
\\date{}
\\begin{document}
\\maketitle
\\section{%%%section_name%%%}
%%%efficiency_table%%%
\\end{document}
"""


def render_to_latex(df, eff_name, filename):
    """
    Store a pandas data frame df to a latex file compile ot pdf if
    pdflatex is available on the machine
    """
    tex_filename = filename + ".tex"
    output_string = ""

    output = StringIO()
    df.to_latex(output, multicolumn=True, column_format="l" + "p{3cm}" * len(df.columns))
    output_string = output.getvalue()

    latex_string = latex_template.replace("%%%efficiency_table%%%", output_string)
    latex_string = latex_string.replace("%%%section_name%%%", eff_name)
    latex_string = latex_string.replace("%%%title%%%", eff_name)

    # do some espacing for characters troublesome to Latex
    latex_string = latex_string.replace("<", "$<$")
    latex_string = latex_string.replace(">", "$>$")
    latex_string = latex_string.replace("software\_trigger\_cut\&", "software\_trigger\_cut\& ")

    with open(tex_filename, "w") as file:
        file.write(latex_string)

    if shutil.which("pdflatex"):
        subprocess.check_call(["pdflatex", tex_filename])


def extract_efficiencies(channels, storage_location):
    """
    Compute the software trigger efficiencies from channels
    """
    efficiency_list = []

    for channel in channels:
        result_list = []
        channel_path = os.path.join(storage_location, channel)
        analysed_path = os.path.join(channel_path, "analysed")

        for filename in glob(os.path.join(analysed_path, "*_results.pkl")):
            result_list += pd.read_pickle(filename)

        if len(result_list) == 0:
            continue

        results = pd.DataFrame(result_list)

        efficiencies = (results[["final_decision", "software_trigger_cut&calib&total_result",
                                 "software_trigger_cut&fast_reco&total_result",
                                 "software_trigger_cut&hlt&total_result"]] == 1).mean()
        # efficiencies = results[[col for col in results.columns if "fast_reco" in col]].mean()
        efficiencies.index = ["final decision", "calibration", "fast reco", "hlt"]
        efficiencies = pd.concat([efficiencies, (results[[col for col in results.columns if "fast_reco" in col]] == 1).mean()])
        efficiencies.name = channel
        efficiency_list.append(efficiencies)

    all_efficiencies = pd.DataFrame(efficiency_list)
    all_efficiencies.to_pickle("all_efficiencies.pkl")

    print("\n### Final Efficiencies per channel ###\n")
    print(all_efficiencies)

    render_to_latex(all_efficiencies, "All Efficiencies", "all_efficiencies")

    return all_efficiencies


def extract_l1_efficiencies(channels, storage_location):
    """
    Compute the level1 efficiencies from channels
    """
    efficiency_list = []

    for channel in channels:
        result_list = []
        channel_path = os.path.join(storage_location, channel)
        analysed_path = os.path.join(channel_path, "analysed")

        for filename in glob(os.path.join(analysed_path, "*_l1_results.pkl")):
            result_list += pd.read_pickle(filename)

        if len(result_list) == 0:
            continue

        results = pd.DataFrame(result_list)

        efficiencies = (results == 1).mean()
        efficiencies.name = channel
        efficiency_list.append(efficiencies)

    all_efficiencies = pd.DataFrame(efficiency_list)
    all_efficiencies.to_pickle("all_l1_efficiencies.pkl")

    print("\n### Final L1 Efficiencies per channel ###\n")
    print(all_efficiencies)

    render_to_latex(all_efficiencies, "L1 Efficiencies", "l1_efficiencies")

    return all_efficiencies


def extract_file_sizes(channels, storage_location):
    """
    Compute the file size for each channel
    """
    # Is this really needed?
    ROOT.gErrorIgnoreLevel = ROOT.kWarning + 1

    filesize_list = []

    for channel in channels:
        result_list = []
        channel_path = os.path.join(storage_location, channel)
        reconstructed_path = os.path.join(channel_path, "reconstructed")

        for filename in glob(os.path.join(reconstructed_path, "*_raw.root")):
            result = {}

            root_file = ROOT.TFile(filename)
            tree = root_file.Get('tree')
            if not tree:
                continue
            root_events = tree.GetEntriesFast() * 1.0
            for branch in tree.GetListOfBranches():
                result[branch.GetName()] = branch.GetZipBytes('*') / root_events
                result[branch.GetName() + " unzipped"] = branch.GetTotalSize() / root_events
            result["root_total_size"] = tree.GetZipBytes() / root_events
            result["root_total_size_unzipped"] = tree.GetTotBytes() / root_events
            result["total_size_on_disk"] = os.path.getsize(filename) / root_events
            result_list.append(result)

        results = pd.DataFrame(result_list)

        filesizes = results.mean()
        filesizes.name = channel
        filesize_list.append(filesizes)

    all_filesizes = pd.DataFrame(filesize_list)
    all_filesizes.to_pickle("all_filesizes.pkl")

    print("\n### Final File Sizes per channel in bytes ###\n")
    print(all_filesizes)

    # add some units
    all_filesizes = all_filesizes.divide(1000)
    all_filesizes.columns = [cname + " [kB]" for cname in all_filesizes.columns]

    render_to_latex(all_filesizes, "File Sizes", "all_filesizes")

    return all_filesizes
