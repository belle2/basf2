import ROOT


def initialize_results():
    # Create a dict with the arrays for the results
    column_names = ["finding_efficiency", "hit_efficiency", "clone_rate", "fake_rate"]
    results = {column: [] for column in column_names}
    return results


def extract_information_from_file(file_name, results):
    # Read in the given root file and export the information to the results
    root_file = ROOT.TFile(file_name)

    keys = root_file.GetListOfKeys()

    for graph_or_table in keys:
        name = graph_or_table.GetName()
        # As the naming convention is that simple in the validation output, this single line should do the job
        if "overview_figures_of_merit" in str(name):
            overview_table = root_file.Get(name)
            overview_table.GetEntry(0)
            for branch in overview_table.GetListOfBranches():
                branch_name = branch.GetName()
                if branch_name in results:
                    results[branch_name].append(100.0 * float(getattr(overview_table, branch.GetName())))

    return results
