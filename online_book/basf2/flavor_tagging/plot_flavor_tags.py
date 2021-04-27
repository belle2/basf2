import matplotlib.pyplot as plt
import root_pandas


plt.style.use("belle2")

df = root_pandas.read_root("Bd2JpsiKS.root")

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
