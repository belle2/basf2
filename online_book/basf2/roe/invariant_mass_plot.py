#!/usr/bin/env python3

import matplotlib as mpl
import matplotlib.pyplot as plt
from root_pandas import read_root

# Only include this line if you're running from ipython an a remote server
mpl.use("Agg")

plt.style.use("belle2")  # use the official Belle II plotting style

# Make sure that the .root file is in the same directory to find it
df = read_root("Bd2JpsiKS.root")

# Let's only consider signal J/Psi
df_signal_only = df.query("Jpsi_isSignal == 1")

fig, ax = plt.subplots()

ax.hist(df_signal_only["Jpsi_M_uncorrected"], label="w/o brems corr", alpha=0.5)
ax.hist(df_signal_only["Jpsi_M"], label="with brems corr", alpha=0.5)

ax.set_yscale("log")  # set a logarithmic scale in the y-axis
ax.set_xlabel("Invariant mass of the J/Psi")
ax.set_ylabel("Events")
ax.legend()  # show legend

plt.savefig("brems_corr_invariant_mass.png")
