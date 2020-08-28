#!/usr/bin/env python3

import sys
from ROOT import Belle2

if __name__ == "__main__":
    if len(sys.argv) < 2:
        raise ValueError("Please specify the CDC cut value.")

    # define parameter for the CDC QI
    cut_value_CDC_QI = float(sys.argv[1])
    print(" A CDC QI cut value of", cut_value_CDC_QI, "will be written into the CDB.")

    # and for the payload
    iov = Belle2.IntervalOfValidity.always()

    # and save in localdb/, ready for upload
    Belle2.Database.Instance().storeData("CDCQICutParameter", cut_value_CDC_QI, iov)
