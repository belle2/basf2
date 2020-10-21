import matplotlib.pyplot as plt
from root_pandas import read_root

plt.style.use("belle2")
df = read_root("Bd2JpsiKS.root")

m_bins = 50  # number of bins for the histograms of both plots

# Z position

fig, ax = plt.subplots(figsize=(8, 6))
m_range = [-0.1, 0.1]
ax.set_xlim(left=-0.1, right=0.15)
ax.hist(df["Jpsi_dz"], bins=m_bins, range=m_range, label=r"$J/\psi$ vertex")
ax.hist(
    df["Jpsi_mcDecayVertexZ"],
    histtype="step",
    lw=2,
    color="black",
    linestyle="--",
    bins=m_bins,
    range=m_range,
    label=r"$J/\psi$ vertex(MC)",
)
ax.set_xlabel("dz[cm]")
ax.set_ylabel("Events")
ax.legend()
fig.savefig("vertex_jpsi_dz.svg")

# P-value

fig, ax = plt.subplots(figsize=(8, 6))
m_range = [0, 1]
ax.set_xlim(left=-0.05, right=1.05)
ax.hist(
    df["Jpsi_chiProb"],
    bins=m_bins,
    range=m_range,
    label=r"$J/\psi$ vertex",
)
ax.set_yscale("log")  # set a logarithmic scale in the y-axis
ax.set_xlabel("p-value")
ax.set_ylabel("Events")
ax.legend()
fig.savefig("vertex_pValue.svg")
