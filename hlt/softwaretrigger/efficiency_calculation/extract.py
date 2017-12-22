import os
from glob import glob

import ROOT
import pandas as pd


def extract_efficiencies(channels, storage_location):
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


def extract_file_sizes(channels, storage_location):
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
            result["root_total_size"] = tree.GetZipBytes() / root_events
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
