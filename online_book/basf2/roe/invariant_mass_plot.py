#!/usr/bin/env python3

import matplotlib.pyplot as plt
from root_pandas import read_root

df = read_root("Bd2JpsiKS.root")

df.query("Jpsi_isSignal == 1").hist("Jpsi_M_uncorrected", label="w/o brems corr")
df.query("Jpsi_isSignal == 1").hist("Jpsi_M", label="with brems corr", alpha=0.7)
plt.yscale("log")  # set a logarithmic scale in the y-axis
plt.legend()  # show legend
