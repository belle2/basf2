import b2luigi as luigi
from b2luigi.basf2_helper import Basf2PathTask, Basf2nTupleMergeTask

import json
import basf2 as b2
import modularAnalysis as ma
import vertex as vx

from skim import BatchesToTextFile, NumBatches


class ReconstructBatch(Basf2PathTask):
    queue = "s"
    looper = luigi.IntParameter()
    projectName = luigi.Parameter()
    skim = luigi.Parameter(hashed=True)
    runningOnMC = luigi.BoolParameter()

    def requires(self):
        yield BatchesToTextFile(
                runningOnMC=self.runningOnMC, projectName=self.projectName, skim=self.skim
            )

    def output(self):
        yield self.add_to_output("reco.root")

    def create_path(self):

        mypath = b2.create_path()
        with open(self.get_input_file_names()[f"batch{self.looper}.json"][0]) as f:
            inputMdstList = json.load(f)

        ma.inputMdstList(environmentType="default", filelist=inputMdstList, path=mypath)

        ma.fillParticleList(
            decayString='K+:1%',
            cut="dr < 0.5 and abs(dz) < 3 and thetaInCDCAcceptance and kaonID > 0.01",
            path=mypath)
        ma.fillParticleList(decayString='pi+:my', cut="dr < 0.5 and abs(dz) < 3 and thetaInCDCAcceptance", path=mypath)

        ma.reconstructDecay(decayString="D-:K2Pi -> K+:1% pi-:my pi-:my", cut="1.844 < M < 1.894", path=mypath)

        ma.reconstructDecay(decayString='B0:PiD-toK2Pi -> D-:K2Pi pi+:my', cut='5.0 < Mbc and abs(deltaE) < 1.0', path=mypath)
        vx.treeFit('B0:PiD-toK2Pi', -1, path=mypath, updateAllDaughters=False, ipConstraint=True, massConstraint=[-411, 411])
        ma.applyCuts('B0:PiD-toK2Pi', "1.845<daughter(0, M)<1.895", path=mypath)

        if(self.runningOnMC):
            ma.matchMCTruth(list_name='B0:PiD-toK2Pi', path=mypath)

        some_variables = ['Mbc', 'deltaE']
        ma.variablesToNtuple(decayString='B0:PiD-toK2Pi', variables=some_variables,
                             filename=self.get_output_file_name("reco.root"),  path=mypath, treename='BtoPiDtoKPiPi')
        return mypath


class ReconstructionWrapper(Basf2nTupleMergeTask):
    batch_system = 'local'
    projectName = luigi.Parameter()
    skim = luigi.Parameter(hashed=True)

    def requires(self):
        for looper in range(NumBatches):
            yield self.clone(ReconstructBatch, runningOnMC=True,
                             looper=looper, projectName=self.projectName, skim=self.skim)
