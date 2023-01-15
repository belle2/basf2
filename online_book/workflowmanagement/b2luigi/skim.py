# @cond
import b2luigi as luigi
from b2luigi.basf2_helper.tasks import Basf2PathTask

import os
import json
import basf2 as b2
import modularAnalysis as ma
import vertex as vx


class SkimTask(Basf2PathTask):
    batch_system = "gbasf2"
    gbasf2_project_name_prefix = luigi.Parameter(significant=False)
    gbasf2_input_dataset = luigi.Parameter(hashed=True)
#    gbasf2_release = "<release name>" #defaults to current basf2 release if not specified
    gbasf2_print_status_updates = True
    gbasf2_max_retries = 10
    gbasf2_download_dataset = True
    gbasf2_download_logs = False

    runningOnMC = luigi.BoolParameter()

    def output(self):
        yield self.add_to_output("skim.udst.root")  # udst.skimmed_udst_output will add ending .udst.root if different

    def create_path(self):
        mypath = b2.create_path()
        ma.inputMdstList(filelist=self.gbasf2_input_dataset, path=mypath, entrySequences=['0:10'])

        ma.fillParticleList(
            decayString='K+:my',
            cut="dr < 0.5 and abs(dz) < 3 and thetaInCDCAcceptance and kaonID > 0.01",
            path=mypath)
        ma.fillParticleList(decayString='pi+:my', cut="dr < 0.5 and abs(dz) < 3 and thetaInCDCAcceptance", path=mypath)

        ma.reconstructDecay(decayString="D-:K2Pi -> K+:my pi-:my pi-:my", cut="1.5 < M < 2.2", path=mypath)

        ma.reconstructDecay(decayString='B0:PiD-toK2Pi -> D-:K2Pi pi+:my', cut='5.0 < Mbc and abs(deltaE) < 1.0', path=mypath)
        vx.treeFit('B0:PiD-toK2Pi', 0, path=mypath, updateAllDaughters=False, ipConstraint=True, massConstraint=[411])
        ma.applyCuts('B0:PiD-toK2Pi', '5.2 < Mbc and abs(deltaE) < 0.5', path=mypath)

        import udst
        # dump in UDST format
        # basf2 currently does not support pickling paths with skimmed udst outputs
        udst.add_udst_output(path=mypath, filename="skim.udst.root", particleLists=['B0:PiD-toK2Pi'], mc=self.runningOnMC)

#        udst.add_skimmed_udst_output(mypath, skimDecayMode="BtoPiD", skimParticleLists=['B0:PiD-toK2Pi'], mc=self.runningOnMC,
#                                     outputFile="skim.udst.root"  # WARNING: here do not use self.get_output_file_name
#                                    )
        return mypath


class BatchesToTextFile(luigi.Task):
    batch_system = 'local'
    skim = luigi.Parameter(hashed=True)
    projectName = luigi.Parameter()
    runningOnMC = luigi.BoolParameter()
    NumBatches = 3

    def requires(self):
        yield SkimTask(
                runningOnMC=self.runningOnMC,
                gbasf2_project_name_prefix=self.projectName,
                gbasf2_input_dataset=self.skim
            )

    def get_batch_file_names(self, key="skim.udst.root"):
        inputdir = self._transform_input(self.input(), key)[0]
        skimfiles = [f"{inputdir}/{file}" for file in os.listdir(inputdir)]

        binwidth = int(len(skimfiles)/self.NumBatches)

        batches = {}
        for batch in range(self.NumBatches):
            if(batch == self.NumBatches - 1):
                batches.update({f"batch{batch}.json": list(skimfiles[binwidth*batch:])})
            else:
                batches.update({f"batch{batch}.json": list(skimfiles[binwidth*batch:binwidth*(batch+1)])})
        return batches

    def output(self):
        for batch in range(self.NumBatches):
            yield self.add_to_output(f"batch{batch}.json")

    def run(self):

        for key, file_list in self.get_batch_file_names().items():
            if hasattr(self, "keys") and key not in self.keys:
                continue

            with open(self.get_output_file_name(key), "w+") as f:
                f.write(json.dumps(file_list))
# @endcond
