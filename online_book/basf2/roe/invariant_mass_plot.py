#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import matplotlib as mpl
import matplotlib.pyplot as plt
import uproot

# Only include this line if you're running from ipython an a remote server
mpl.use("Agg")

plt.style.use("belle2")  # use the official Belle II plotting style

# Declare list of variables
var_list = ['Jpsi_isSignal', 'Jpsi_M_uncorrected', 'Jpsi_M']

# Make sure that the .root file is in the same directory to find it
df = uproot.open("Bd2JpsiKS.root:tree").arrays(var_list, library='pd')

# Let's only consider signal J/Psi
df_signal_only = df.query("Jpsi_isSignal == 1")

fig, ax = plt.subplots()

ax.hist(df_signal_only["Jpsi_M_uncorrected"], label="w/o brems corr", alpha=0.5, range=(1.7, 3.2))
ax.hist(df_signal_only["Jpsi_M"], label="with brems corr", alpha=0.5, range=(1.7, 3.2))

ax.set_yscale("log")  # set a logarithmic scale in the y-axis
ax.set_xlabel("Invariant mass of the J/Psi [GeV]")
ax.set_xlim(1.5, 3.5)
ax.set_ylabel("Events")
ax.legend()  # show legend

plt.savefig("brems_corr_invariant_mass.png")
