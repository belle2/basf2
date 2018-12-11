#!/usr/bin/env python3
# -*- coding: utf-8 -*-


from basf2 import *
from basf2 import use_local_database, LogLevel, reset_database, use_database_chain
from modularAnalysis import *
import b2biiConversion
from b2biiConversion import convertBelleMdstToBelleIIMdst, setupB2BIIDatabase

# Personally I always override the Magnetic field to 1.5T, which was the default in Belle,
# the magnetic field in BASF2 for Belle is also 1.5T but only in a finite region around the detector,
# this leads to (very) small deviation, hence I do this override.
import ROOT
from ROOT import Belle2
# ROOT.Belle2.BFieldManager.getInstance().setConstantOverride(0, 0, 1.5 * ROOT.Belle2.Unit.T)

# You have to use the decay-channel configuration for Belle (e.g. PID variables are different)
# AND you have to set b2bii in the FeiConfiguration, because the loading of the FSP particles is different
import fei

os.environ['BELLE_POSTGRES_SERVER'] = 'can01'
os.environ['USE_GRAND_REPROCESS_DATA'] = '1'
os.environ['PGUSER'] = 'g0db'
os.environ['BELLE2_FILECATALOG'] = 'NONE'

# setupB2BIIDatabase(True)

reset_database()
use_database_chain()

use_local_database(
    os.path.join('/home/belle/pablog/work/belle2/basf2/analysis/examples/5SFEI/db/', 'database.txt'),
    '/home/belle/pablog/work/belle2/basf2/analysis/examples/5SFEI/db/',
    False,
    LogLevel.WARNING)

# particles = fei.get_default_channels(convertedFromBelle=True, chargedB=False, neutralB=False)
particles = fei.get_default_channels(convertedFromBelle=True)
configuration = fei.config.FeiConfiguration(prefix='FEI_Belle1_Generic_2017_1', b2bii=True, training=True, monitor=True)
feistate = fei.get_path(particles, configuration)

path = create_path()

if feistate.stage <= 0:
    b2biiConversion.convertBelleMdstToBelleIIMdst(
        "/group/belle/bdata_b/mcprod/dat/e000043/evtgen/bsbs/00/all/0127/" +
        "5S_onresonance/10/evtgen-bsbs-00-all-e000043r001016-b20090127_0910.mdst",
        # "/group/belle/bdata_b/mcprod/dat/e000065/evtgen/mixed/00/all/0127/on_resonance/00/evtgen-mixed-00-all-e000065r000084-b20090127_0910.mdst",
        applyHadronBJSkim=True,
        useBelleDBServer=os.environ['BELLE_POSTGRES_SERVER'], path=path)
else:
    inputMdstList('Belle', [], path)

path.add_path(feistate.path)
path.add_module('RootOutput')

print(path)
process(path)


# from basf2 import *
# from modularAnalysis import *
#
# import b2biiConversion
# import ROOT
# from ROOT import Belle2
# ROOT.Belle2.BFieldManager.getInstance().setConstantOverride(0, 0, 1.5 * ROOT.Belle2.Unit.T)
#
# import fei
# particles = fei.get_default_channels(convertedFromBelle=True)
# configuration = fei.config.FeiConfiguration(prefix='FEI_TEST', b2bii=True, training=True)
# feistate = fei.get_path(particles, configuration)
#
#
# path = create_path()
# if feistate.stage <= 0:
#     b2biiConversion.convertBelleMdstToBelleIIMdst("/group/belle/bdata_b/mcprod/dat/e000043/evtgen/bsbs/00/all/0127/5S_onresonance/10",
# applyHadronBJSkim=True, path=path)
# else:
#     inputMdstList('Belle', [], path)
#
# path.add_path(feistate.path)
# path.add_module('RootOutput')
#
# print(path)
# process(path)
