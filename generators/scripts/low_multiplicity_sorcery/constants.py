##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import enum


class EventCode(enum.IntEnum):
    """
    Event code number for channel identification. The value should be
    read from mDST with variable ``eventExtraInfo(EventCode)``.
    """
    #: eetautau
    eetautau = 1
    #: mumu
    mumumumu = 2
    #: mumutautau
    mumutautau = 3
    #: tautautautau
    tautautautau = 4
    #: eeKK
    eeKK = 5
    #: eepp
    eepp = 6
    #: eepipi
    eepipi = 7
    #: pi+pi-pi0ISR
    pipipi0ISR = 8
    #: K+K-ISR
    KKISR = 9
    #: K0K0barISR
    K0K0barISR = 10
    #: pi+pi-ISR
    pipiISR = 11
    #: pi+pi-pi+pi-ISR
    pipipipiISR = 12
    #: pi+pi-pi0pi0ISR
    pipipi0pi0ISR = 13
    #: etapi+pi-ISR
    etapipiISR = 14
    #: ppbarISR
    ppbarISR = 15


EventCode_CrossSection = {
    # The cross sections here are in nb.
    EventCode.eetautau: 0.01836,
    EventCode.mumumumu: 0.0003512,
    EventCode.mumutautau: 0.0001441,
    EventCode.tautautautau: 0.0000002114,
    EventCode.eeKK: 0.0798,
    EventCode.eepp: 0.0117,
    EventCode.eepipi: 1.895,
    EventCode.pipipi0ISR: 0.02637,
    EventCode.pipipipiISR: 0.02586,
    EventCode.pipipi0pi0ISR: 0.03922,
    EventCode.etapipiISR: 0.00264,
    EventCode.KKISR: 0.01631,
    EventCode.K0K0barISR: 0.008951,
    EventCode.ppbarISR: 0.0006176,
    EventCode.pipiISR: 0.1710,
}


__eventCodes = [e for e in (EventCode)]
EventCodes = tuple(__eventCodes)

EventCodes_hhISR = (
    EventCode.pipipi0ISR,
    EventCode.pipipipiISR,
    EventCode.pipipi0pi0ISR,
    EventCode.etapipiISR,
    EventCode.KKISR,
    EventCode.K0K0barISR,
    EventCode.ppbarISR,
    EventCode.pipiISR)

__eventCodes_llXX = [e for e in EventCodes if e not in EventCodes_hhISR]
EventCodes_llXX = tuple(__eventCodes_llXX)

EventCodes_taus = (EventCode.eetautau, EventCode.mumutautau, EventCode.tautautautau)
