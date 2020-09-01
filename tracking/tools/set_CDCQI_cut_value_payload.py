#!/usr/bin/env python3

import sys
from ROOT import Belle2


if __name__ == "__main__":
    if len(sys.argv) < 4:
        raise ValueError("Please specify in the given order: the CDC cut value, experiment, run [exp. end], [run end].")

    # define parameter for the CDC QI
    cut = float(sys.argv[1])

    # and for the payload
    experiment = int(sys.argv[2])
    run = int(sys.argv[3])
    exp_end = -1
    run_end = -1
    if len(sys.argv) >= 5:
        exp_end = int(sys.argv[4])
    if len(sys.argv) >= 6:
        run_end = int(sys.argv[5])
    iov = Belle2.IntervalOfValidity(experiment, run, exp_end, run_end)

    # create cdc qi cut parameter payload instance
    cdcQIcut = Belle2.CDCQICutParameter()
    cdcQIcut.setCDCQIcutvalue(cut)

    # and save in localdb/, ready for upload
    Belle2.Database.Instance().storeData("CDCQICutParameter", cdcQIcut, iov)

    print("Successfully wrote CDC QI cut value of", cut, "into the CDB with iov (",
          experiment, ",", run, ",", exp_end, ",", run_end, ")")
