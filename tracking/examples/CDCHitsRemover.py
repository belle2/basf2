import basf2

basf2.conditions.expert_settings(stalled_timeout=0, connection_timeout=600)
basf2.conditions.disable_globaltag_replay()
basf2.conditions.prepend_globaltag("online")
basf2.conditions.prepend_globaltag("data_proc16")

dir = '/group/belle2/dataprod/BGOverlay/BGOrd/rel8/BGOExp24rel8/release-08-00-04/e0024/4S/r02280/beambg/sub00/'
fName = 'beambg_000001_prod00040540_task232279000001.root'
path = basf2.create_path()
path.add_module("RootInput", inputFileNames=[dir+fName])
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
