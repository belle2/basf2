"""LCG extractor IAction implementation.

The LCGExtractor class provides extra LCG-backend-specific attributes in addition to the generic extractor implementation.
 
"""

from GangaRobot.Framework import Utility
from GangaRobot.Lib.Base.BaseExtractor import BaseExtractor
from GangaRobot.Lib.Core.CoreExtractor import CoreExtractor
from Ganga.GPI import *

class LCGExtractor(BaseExtractor):
    
    def __init__(self):
        self.chain = [CoreExtractor(), self]

    def handlejobnode(self, jobnode, job):
        jobnode.addnode('backend-reason', job.backend.reason)
        jobnode.addnode('backend-exitcode', job.backend.exitcode)
        jobnode.addnode('backend-exitcode_lcg', job.backend.exitcode_lcg)


