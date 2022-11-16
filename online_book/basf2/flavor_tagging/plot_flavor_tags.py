##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import matplotlib.pyplot as plt
import uproot


plt.style.use("belle2")

df = uproot.open("Bd2JpsiKS.root:tree").arrays(['qrMC', 'FBDT_qrCombined'], library='pd')

fig, ax = plt.subplots()
ax.hist(
    df.query("qrMC == -1.")["FBDT_qrCombined"],
    histtype="step",
    linewidth=1.5,
    label=r"True $\bar B^0$",
    bins=30,
)
ax.hist(
    df.query("qrMC == 1.")["FBDT_qrCombined"],
    histtype="step",
    linewidth=1.5,
    label=r"True $B^0$",
    bins=30,
)
ax.set_xlabel("FBDT_qrCombined")
ax.legend()
fig.savefig("flavor_tags.svg")

