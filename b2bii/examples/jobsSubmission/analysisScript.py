#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# This is the main file for the analysis script
# It only contains:
#   some arguments sorting that will make it work with the job submission script
#   the relevant code for B2BII + FEI
# so it needs to be completed to make it work

# G. Caria

# import modules here...

# ------- Arguments sorting
if len(sys.argv) != 8:
    sys.exit('Must provide all 7 parameters !')

expNo = sys.argv[1]
minRunNo = sys.argv[2]
maxRunNo = sys.argv[3]
eventType = sys.argv[4]
dataType = sys.argv[5]
belleLevel = sys.argv[6]
streamNo = sys.argv[7]

# ------- B2BII
mc_beamparams = ''

os.environ['BELLE_POSTGRES_SERVER'] = 'can01'
os.environ['USE_GRAND_REPROCESS_DATA'] = '1'

isMC = True
b2biiConversion.setupB2BIIDatabase(True, False)

url = getBelleUrl(expNo, minRunNo, maxRunNo,
                  eventType, dataType, belleLevel, streamNo)

# setup database
use_central_database('production', LogLevel.WARNING, 'feiFiles/fei_database')
use_local_database('')
use_local_database(filename=os.path.join(mc_beamparams, 'B2BII_MC_database/dbcache.txt'),
                   directory=os.path.join(mc_beamparams, 'B2BII_MC_database'),
                   readonly=False, loglevel=LogLevel.INFO)

for exp in range(100):
    set_experiment_name(exp, "BELLE_exp%d" % exp)

b2biiConversion.convertBelleMdstToBelleIIMdst(url,
                                              applyHadronBJSkim=True)

# Gearbox needs to be loaded
loadGearbox()

#  ------- FEI
empty_path = create_path()
skimfilter = register_module('VariableToReturnValue')
skimfilter.param('variable', 'nCleanedTracks(dr < 2 and abs(dz) < 4)')
skimfilter.if_value('>12', empty_path, AfterConditionPath.END)
analysis_main.add_module(skimfilter)

fei_path = get_path_from_file('')
analysis_main.add_path(fei_path)

# ------- Output file
outDir = ''

if not os.path.exists(outDir):
    os.makedirs(outDir)

filenameEnd = str(expNo) + '_' + str(minRunNo) + '_' + str(maxRunNo) + '_' +\
    str(streamNo) + '_' + eventType + '_' + dataType + '.root'

outputFileName = outDir + '/output_' + filenameEnd

ntupleFile(outputFileName)

# Rest of analysis script goes here...
