#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Test LHEReader and LHEInputModule

import os
import sys
import math
from ROOT import TFile
from pdg import add_particle
from basf2 import create_path, register_module, process, print_params, find_file
from modularAnalysis import fillParticleListsFromMC, printVariableValues, variablesToNtuple
from variables import variables as vm
from tempfile import TemporaryDirectory

# check that the file exists, if not: skip the test
inputfile = find_file('generators/tests/event_flipped.lhe')
if len(inputfile) == 0:
    sys.stderr.write(
        'TEST SKIPPED: input file ' +
        inputfile +
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
lhereader.param('wrongSignPz', True)  # Using flipped convention where e+ goes in +z
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

printVariableValues('A:gen', variables, path=testpath)
printVariableValues('gamma:gen', variables, path=testpath)

variablesToNtuple('A:gen', variables, 'darkphoton', 'test.root', path=testpath)
variablesToNtuple('gamma:gen', variables, 'gammas', 'test.root', path=testpath)

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

    # precision is low since MCParticle class is using floats to store particle momenta
    # when changed to double precision was much better
    eps = 2e-5

    # test momenta balance in CMS
    assert math.isclose(t1.pxcms, -t2.pxcms, rel_tol=eps), 'Momenta don\'t balance'
    assert math.isclose(t1.pycms, -t2.pycms, rel_tol=eps), 'Momenta don\'t balance'
    assert math.isclose(t1.pzcms, -t2.pzcms, rel_tol=eps), 'Momenta don\'t balance'

    # test that dark photon CM momentum is close to LHE input generators/tests/event.lhe
    assert math.isclose(t1.pxcms, +1.4572035746e-03, rel_tol=eps), 'CMS momenta are not as expected'
    assert math.isclose(t1.pycms, +2.5198484375e-03, rel_tol=eps), 'CMS momenta are not as expected'
    assert math.isclose(t1.pzcms, -3.8826254795e+00, rel_tol=eps), 'CMS momenta are not as expected'
    assert math.isclose(t1.pecms, 6.6973734293e+00, rel_tol=eps), 'CMS momenta are not as expected'

    # test that dark photon mass is as in LHE
    assert math.isclose(t1.M, 5.4571074540e+00, rel_tol=eps), 'Mass is not as expected'

    # test dark photon momentum in LAB
    assert math.isclose(t1.px, 0.24481175912036506892, rel_tol=eps), 'Boosted momenta are not as expected'
    assert math.isclose(t1.py, 0.002519848437500000083, rel_tol=eps), 'Boosted momenta are not as expected'
    assert math.isclose(t1.pz, -2.1368737390471825854, rel_tol=eps), 'Boosted momenta are not as expected'

    # test photon momentum in LAB
    assert math.isclose(t2.px, 0.21182522798639694117, rel_tol=eps), 'Boosted momenta are not as expected'
    assert math.isclose(t2.py, -0.002519848437500000083, rel_tol=eps), 'Boosted momenta are not as expected'
    assert math.isclose(t2.pz, 5.1364143846516343572, rel_tol=eps), 'Boosted momenta are not as expected'

    fi.Close()
