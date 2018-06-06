import basf2
from ROOT import Belle2
Belle2.Environment.Instance().setDryRun(True)

basf2.set_random_seed("something boring")
path = basf2.create_path()


def add_inout(path, basename):
    path.add_module("RootInput", inputFileName="in-" + basename + ".root")
    path.add_module("RootOutput", outputFileName="out-" + basename + ".root")


path.add_module("RootInput", inputFileNames=["in-%d.root" % d for d in range(10)])
add_inout(path, "normal")

path1 = basf2.create_path()
add_inout(path1, "sub_path")
path.add_path(path1)


path2 = basf2.create_path()
add_inout(path2, "conditional")
m = path.add_module("EventInfoSetter")
m.if_true(path2)

path3 = basf2.create_path()
add_inout(path3, "for_each")
path.for_each("MCParticles", "MCParticle", path3)

basf2.process(path)
