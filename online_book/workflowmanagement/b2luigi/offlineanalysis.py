# @cond
import b2luigi as luigi
from b2luigi.basf2_helper import Basf2nTupleMergeTask
from reconstruction import ReconstructionWrapper
import uproot
import matplotlib.pyplot as plt


class MergeFiles(Basf2nTupleMergeTask):
    batch_system = 'local'
    projectName = luigi.Parameter()

    def requires(self):
        skims = open(f"../{self.projectName}skims.dat", 'r')
        for skim in skims.read().splitlines():
            yield ReconstructionWrapper(skim=skim, projectName=self.projectName)


class Plot(luigi.Task):
    batch_system = 'local'

    def requires(self):
        data = {}
        data.update({"bmesons": MergeFiles(projectName="bmesons")})
        data.update({"qqcontinuum": MergeFiles(projectName="qqcontinuum")})
        return data

    def output(self):
        yield self.add_to_output("Mbc.jpg")
        yield self.add_to_output("deltaE.jpg")

    def run(self):
        treeName = 'BtoPiDtoKPiPi'
        some_variables = ["Mbc", "deltaE"]
        BBdata = self.requires()["bmesons"].get_output_file_name("reco.root")
        QQdata = self.requires()["qqcontinuum"].get_output_file_name("reco.root")
        BBtuple = uproot.open(f"{BBdata}:{treeName}")
        QQtuple = uproot.open(f"{QQdata}:{treeName}")

        for var in some_variables:
            plt.hist([QQtuple[var].array(), BBtuple[var].array()], label=["uudd Continuum", "mixed B mesons"], stacked=True)
            plt.legend(loc='best')
            plt.xlabel(f"{var} [GeV]")
            plt.savefig(self.get_output_file_name(f"{var}.jpg"), dpi=100)
            plt.close()
# @endcond
