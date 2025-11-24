import basf2
'''
Example script which takes background overlay file as an input, applies CDC hit filters, and writes the result out.
'''

#  path to a background file
NAME = ''
path = basf2.create_path()
path.add_module("RootInput", inputFileNames=[NAME])
path.add_module("Progress")
path.add_module('Geometry', useDB=True)
path.add_module("TFCDC_WireHitPreparer", useBadWires=True, filter="cuts_from_DB", filterParameters={})
path.add_module("CDCHitsRemover", inputWireHits="CDCWireHitVector")
br = [
    'ARICHDigits',
    'CDCHits',
    'ECLWaveforms',
    'EventLevelTriggerInfo',
    'EventMetaData',
    'KLMDigits',
    'SVDShaperDigits',
    'TOPDigits',
    'TRGECLBGTCHits']
path.add_module(
    "RootOutput", outputFileName="beambg_filtered.root", branchNames=br, keepParents=True
)
basf2.print_path(path)
basf2.process(path)
