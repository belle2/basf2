# @cond
import uproot
import matplotlib.pyplot as plt

output_file = {"Mbc": snakemake.output.mbc_plot, "deltaE": snakemake.output.deltaE_plot}
BBdata = snakemake.input.data_BB
QQdata = snakemake.input.data_QQ

treeName = 'BtoPiDtoKPiPi'
some_variables = ["Mbc", "deltaE"]
BBtuple = uproot.open(f"{BBdata}:{treeName}")
QQtuple = uproot.open(f"{QQdata}:{treeName}")

for var in some_variables:
    plt.hist([QQtuple[var].array(), BBtuple[var].array()], label=["uudd Continuum", "mixed B mesons"], stacked=True)
    plt.legend(loc='best')
    plt.xlabel(f"{var} [GeV]")
    plt.savefig(output_file[var], dpi=100)
    plt.close()
# @endcond
