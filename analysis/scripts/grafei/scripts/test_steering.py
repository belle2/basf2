import basf2 as b2
import modularAnalysis as ma


path = b2.create_path()
ma.inputMdst(filename="", path=path)

b2.conditions.prepend_globaltag(ma.getAnalysisGlobaltag())

ma.fillParticleList(
    "e+:test",
    "dr<2 and abs(dz)<2",
    writeOut=True,
    path=path,
)

b2.process(path)
