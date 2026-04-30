import basf2 as b2
import reconstruction as re
import softwaretrigger.path_utils as hlt_path
import softwaretrigger.test_support as hlt_test


class CheckCDCHits(b2.Module):

    def event(self):
        from ROOT import Belle2
        hits = Belle2.PyStoreArray('CDCHits')
        if hits.getEntries() > 0:
            b2.B2FATAL('There is a problem here.')


b2.set_random_seed('eodb')

main = b2.Path()

main.add_module('EventInfoSetter', evtNumList=5)

main.add_module(hlt_test.InflateCDCHits())

hlt_path.add_prefilter_module(main, event_abort=re.default_event_abort)

main.add_module(CheckCDCHits())

b2.process(main, calculateStatistics=True)
