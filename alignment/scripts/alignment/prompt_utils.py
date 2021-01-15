def fix_mille_paths_for_algo(algo):
    """
    Add pre.algorithm to fix mille file paths

    Attach a pre-algorithm which loops over collector files
    and fixes the paths to mille binary files (in case they have
    been moved since creation - otherwise the effect is null)

    If previous pre_algorithm exists, it is run after the fix
    is applied.

    Parameters
    ----------
    algo : caf.framework.Algorithm
        The algorithm to which to attach pre_algorithm
    """

    prev_prealgo = algo.pre_algorithm

    def fixMillePaths(algorithm, iteration):
        import ROOT
        import os

        print("Now fixing .mille binary paths in CollectorOutput.root files")

        for path in algorithm.getInputFileNames():
            file = ROOT.TFile(path, "UPDATE")
            dirname = os.path.dirname(path)

            runRange = file.Get("MillepedeCollector/RunRange")
            runSet = [(-1, -1)] if runRange.getGranularity() == "all" else [(e, r) for e, r in runRange.getExpRunSet()]

            for exp, run in runSet:
                milleData = file.Get(f"MillepedeCollector/mille/mille_{exp}.{run}/mille_1")

                fixed_milleFiles = [os.path.join(dirname, os.path.basename(milleFile)) for milleFile in milleData.getFiles()]

                milleData.clear()
                for f in fixed_milleFiles:
                    milleData.addFile(f)

                file.cd(f"MillepedeCollector/mille/mille_{exp}.{run}/")
                milleData.Write("", ROOT.TObject.kOverwrite)
            file.Write()
            file.Close()

        # Run the previously set pre-algorithm if any
        if prev_prealgo is not None:
            prev_prealgo(algorithm, iteration)

    algo.pre_algorithm = fixMillePaths
