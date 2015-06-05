import ROOT


def initialize_results():
    # Create a dict with the arrays for the results
    column_names = ["finding_efficiency", "hit_efficiency", "clone_rate", "fake_rate"]
    results = {column: [] for column in column_names}
    return results


def extract_information_from_file(file_name, results):
    # Read in the given root file and export the information to the results
    root_file = ROOT.TFile(file_name)
    if not root_file.IsOpen():
        return None

    keys = root_file.GetListOfKeys()
    for graph_or_table in keys:
        name = graph_or_table.GetName()
        # As the naming convention is that simple in the validation output, this single line should do the job
        root_obj = root_file.Get(name)

        if "overview_figures_of_merit" in str(name):
            root_obj.GetEntry(0)
            for branch in root_obj.GetListOfBranches():
                branch_name = branch.GetName()
                if branch_name in results:
                    results[branch_name].append(100.0 * float(getattr(root_obj, branch.GetName())))

        # also support histograms
        if root_obj.IsA().GetName() == "TH1F":
            if root_obj.GetName() in results:
                # compute the y average across bins
                # useful for efficiency over <X> plots
                nbinsx = root_obj.GetNbinsX()
                sum = 0.0
                sumZeroSupressed = 0.0
                countZeroSupressed = 0
                for i in range(nbinsx):
                    v = root_obj.GetBinContent(i + 1)
                    sum = sum + v  # from first bin, ignored underflow (i=0) and overflow (i=nbinsx+1) bins
                    if v > 0.0:
                        sumZeroSupressed = sumZeroSupressed + v
                        countZeroSupressed = countZeroSupressed + 1
                meanY = sum / nbinsx
                meanYzeroSupressed = sum / countZeroSupressed

                results[root_obj.GetName()] = (root_obj.GetMean(), meanY, meanYzeroSupressed)

    root_file.Close()

    return results
