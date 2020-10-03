import matplotlib as mpl
import matplotlib.pyplot as plt
from root_pandas import read_root

plt.style.use("belle2")
df = root_pandas.read_root("Bd2JpsiKS.root")

m_bins = 50  # number of bins for the histograms of both plots

# Z position

plt.figure(figsize=(8, 6))
m_range = [-0.1, 0.1]
plt.xlim(left=-0.1, right=0.15)
plt.hist(df["Jpsi_dz"], bins=m_bins, range=m_range, label=r"$J/\psi$ vertex")
plt.hist(
    df["Jpsi_mcDecayVertexZ"],
    histtype="step",
    lw=2,
    color="black",
    linestyle="--",
    bins=m_bins,
    range=m_range,
    label=r"$J/\psi$ vertex(MC)",
)
plt.xlabel("dz[cm]")
plt.ylabel("Events")
plt.legend()
plt.savefig("vertex/jpsi_dz.svg")

# P-value

plt.figure(figsize=(8, 6))
m_range = [0, 1]
plt.xlim(left=-0.05, right=1.05)
plt.hist(
    df_signal_only["Jpsi_chiProb"],
    bins=m_bins,
    range=m_range,
    label=r"$J/\psi$ vertex",
)
plt.yscale("log")  # set a logarithmic scale in the y-axis
plt.xlabel("p-value")
plt.ylabel("Events")
plt.legend()
plt.savefig("vertex/pValue.svg")
