# Include this only if running in a Jupyter notebook
# %matplotlib inline

import matplotlib.pyplot as plt
from root_pandas import read_root

df = read_root("ContinuumSuppression.root")

fig, ax = plt.subplots()

signal_df = df.query("(isContinuumEvent == 0.0)")
continuum_df = df.query("(isContinuumEvent == 1.0)")

hist_kwargs = dict(bins=30, range=(0, 1), histtype="step")
ax.hist(signal_df["R2"], label="Not Continuum", **hist_kwargs)
ax.hist(continuum_df["R2"], label="Continuum", **hist_kwargs)
ax.set_xlabel("R2")
ax.set_ylabel("Total number of candidates")
ax.legend()
fig.savefig("R2.pdf")
