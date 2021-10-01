#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Test LHEReader and LHEInputModule

import os
import math
from ROOT import TFile
from pdg import add_particle
from basf2 import create_path, register_module, process, print_params, find_file
from modularAnalysis import fillParticleListsFromMC
from modularAnalysis import variablesToNtuple as v2nt
from variables import variables as vm
from beamparameters import add_beamparameters
from tempfile import TemporaryDirectory

# check that the file exists, if not: skip the test
inputfile = find_file('generators/tests/event.lhe')
if len(inputfile) == 0:
    sys.stderr.write(
        'TEST SKIPPED: input file ' +
        filepath +
        ' not found.')
    sys.exit(-1)


# add the A' particle to the pdg database so we can specify it later
add_particle('A', 9000008, 5.5, 0.1329, 0, 0)

# configure the LHE reade
lhereader = register_module('LHEInput')
lhereader.param('makeMaster', True)
lhereader.param('runNum', 1337)
lhereader.param('expNum', 0)
lhereader.param('inputFileList', [inputfile])
lhereader.param('useWeights', False)
lhereader.param('nInitialParticles', 2)
lhereader.param('nVirtualParticles', 0)
lhereader.param('wrongSignPz', True)  # because Belle II convention is different to LEP etc
print_params(lhereader)

# prepare the path
testpath = create_path()
testpath.add_module('Progress')
testpath.add_module(lhereader)
testpath.add_module('BoostMCParticles')
fillParticleListsFromMC([('gamma:gen', ''), ('A:gen', '')], path=testpath)

# dump information from basf2
vm.addAlias('pxcms', 'useCMSFrame(px)')
vm.addAlias('pycms', 'useCMSFrame(py)')
vm.addAlias('pzcms', 'useCMSFrame(pz)')
vm.addAlias('pecms', 'useCMSFrame(E)')
variables = ['M', 'px', 'py', 'pz', 'E',
             'pxcms', 'pycms', 'pzcms', 'pecms']
v2nt('A:gen', variables, 'darkphoton', 'test.root', path=testpath)
v2nt('gamma:gen', variables, 'gammas', 'test.root', path=testpath)

# temporary directory to keep cwd clean
with TemporaryDirectory() as tmp:

    # process the basf2 path
    os.chdir(tmp)
    process(testpath)

    # open output and check the momenta are what is expected
    fi = TFile('test.root')
    t1 = fi.Get('darkphoton')
    t2 = fi.Get('gammas')

    assert t1.GetEntries() == 1, 'Output contains %i entries' % t1.GetEntries()
    assert t2.GetEntries() == 1, 'Output contains %i entries' % t1.GetEntries()

    t1.GetEntry(0)
    t2.GetEntry(0)

    assert t1.__run__ == 1337, 'Run number not set correctly'
    assert t1.__experiment__ == 0, 'Experiment number not set correctly'
    assert t2.M == 0, 'Photon is not as expected'

    # a float is only 7 decimal digits of precision, we might get improvements
    # to the variable manager precision later
    eps = 1e-5
    assert math.isclose(t1.M, 5.4571074540e+00, rel_tol=eps), 'Mass is not as expected'

    assert math.isclose(t1.pxcms, -t2.pxcms, rel_tol=eps), 'Momenta don\'t balance'
    assert math.isclose(t1.pycms, -t2.pycms, rel_tol=eps), 'Momenta don\'t balance'
    assert math.isclose(t1.pzcms, -t2.pzcms, rel_tol=eps), 'Momenta don\'t balance'

    assert math.isclose(t1.pxcms, -0.023859674786793544, rel_tol=eps), 'CMS momenta are not as expected'
    assert math.isclose(t1.pycms, 0.0025198485236614943, rel_tol=eps), 'CMS momenta are not as expected'
    assert math.isclose(t1.pzcms, -3.882552444880825, rel_tol=eps), 'CMS momenta are not as expected'
    assert math.isclose(t1.pecms, 6.697373505125157, rel_tol=eps), 'CMS momenta are not as expected'

    assert t1.px == 0.24189484119415283, 'Boosted momenta are not as expected'
    assert t1.py == 0.0025198485236614943, 'Boosted momenta are not as expected'
    assert t1.pz == -2.136873722076416, 'Boosted momenta are not as expected'

    assert t2.px == 0.21474215388298035, 'Boosted momenta are not as expected'
    assert t2.py == -0.0025198485236614943, 'Boosted momenta are not as expected'
    assert t2.pz == 5.136414527893066, 'Boosted momenta are not as expected'

    fi.Close()
