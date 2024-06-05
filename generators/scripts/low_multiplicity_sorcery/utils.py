##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
from .constants import EventCode, EventCode_CrossSection, EventCodes

from basf2 import B2FATAL, Module
from generators import add_aafh_generator, add_koralw_generator, add_phokhara_generator, add_treps_generator


def add_low_multiplicity_generator(path, event_code):
    """
    Add dedicated generator based on EventCode.
    Module name is also changed to avoid conflicts

    Parameters:
        path (basf2.Path): path where generators should be added
        event_code (EventCode): EventCode integer to identify a channel
    """

    if event_code not in EventCodes:
        B2FATAL(f'The event code {event_code} is unknown.')
    if event_code is EventCode.eetautau:
        add_aafh_generator(path, 'e+e-tau+tau-', enableTauDecays=False)
        rename_module_in_path(path, 'AafhInput', 'AAFH_eetautau')
    elif event_code is EventCode.mumumumu:
        add_aafh_generator(path, 'mu+mu-mu+mu-')
        rename_module_in_path(path, 'AafhInput', 'AAFH_mumumumu')
    elif event_code is EventCode.mumutautau:
        add_aafh_generator(path, 'mu+mu-tau+tau-', enableTauDecays=False)
        rename_module_in_path(path, 'AafhInput', 'AAFH_mumutautau')
    elif event_code is EventCode.tautautautau:
        add_koralw_generator(path, 'tau+tau-tau+tau-', enableTauDecays=False)
        rename_module_in_path(path, 'KoralWInput', 'KoralW_tautautautau')
    elif event_code is EventCode.eepipi:
        add_treps_generator(path, 'e+e-pi+pi-')
        rename_module_in_path(path, 'TrepsInput', 'TREPS_eepipi')
    elif event_code is EventCode.eeKK:
        add_treps_generator(path, 'e+e-K+K-')
        rename_module_in_path(path, 'TrepsInput', 'TREPS_eeKK')
    elif event_code is EventCode.eepp:
        add_treps_generator(path, 'e+e-ppbar')
        rename_module_in_path(path, 'TrepsInput', 'TREPS_eepp')
    elif event_code is EventCode.pipiISR:
        add_phokhara_generator(path, 'pi+pi-')
    elif event_code is EventCode.pipipi0ISR:
        add_phokhara_generator(path, 'pi+pi-pi0')
    elif event_code is EventCode.pipipipiISR:
        add_phokhara_generator(path, 'pi+pi-pi+pi-')
    elif event_code is EventCode.pipipi0pi0ISR:
        add_phokhara_generator(path, 'pi+pi-pi0pi0')
    elif event_code is EventCode.pipietaISR:
        add_phokhara_generator(path, 'pi+pi-eta')
    elif event_code is EventCode.KKISR:
        add_phokhara_generator(path, 'K+K-')
    elif event_code is EventCode.K0K0barISR:
        add_phokhara_generator(path, 'K0K0bar')
    elif event_code is EventCode.ppbarISR:
        add_phokhara_generator(path, 'ppbar')


def get_event_ranges(events, event_codes=None):
    """
    Get event range for channels to be generated. The number of events of each channel is determined by:
    channel cross-section / total cross-section of all channels assigned to the same basf2 job.

    Parameters:
        events (int): Total number of events
        event_codes (Iterable): tuple or list of EventCode.
    Return:
        a dictionary with EvenCode as the key and tuple of event range as value.
    """
    total_cross_section = sum(EventCode_CrossSection[e] for e in event_codes)
    lower_range_delimiter = 0
    return_dict = {}
    for e in event_codes:
        weight = EventCode_CrossSection[e] / total_cross_section
        # Check if this is the last event code: in case, close the range and return.
        if e == event_codes[len(event_codes) - 1]:
            return_dict[e] = (lower_range_delimiter, events + 1)
            return return_dict
        events_in_range = round(events * weight)
        return_dict[e] = (lower_range_delimiter, lower_range_delimiter + events_in_range)
        lower_range_delimiter += events_in_range


def rename_module_in_path(path, module_type, new_module_name):
    """
    Helper function to rename a module named with its type.
    Parameters:
        path (basf2.Path): path where a module to be renamed
        module_type (str): default module name which is also module.type()
        new_module_name (str): new module name
    """
    for module in path.modules():
        if module.type() == module_type:
            module.set_name(new_module_name)


class EventRangePathSplitter(Module):
    """
    Module to set ``eventExtraInfo(EventCode)`` according to event range
    of each channel.

    Parameters:
        event_codes (Iterable): tuple or list of EventCode.
    """

    def __init__(self, event_codes):
        """
        Initialize the class
        Parameters:
            event_codes (Iterable): tuple or list of EventCode.
        """
        import ROOT  # noqa
        super().__init__()
        #: tuple or list of event codes
        self.event_codes = event_codes
        #: PyStoreObj off EventExtraInfo
        self.event_extra_info = ROOT.Belle2.PyStoreObj('EventExtraInfo')
        #: Dictionary to get number of events for each event code
        self.event_ranges = None
        #: Event number
        self.event_number = -1

    def initialize(self):
        """
        Initialize module before any events are processed
        """
        import ROOT  # noqa
        self.event_ranges = get_event_ranges(ROOT.Belle2.Environment.Instance().getNumberOfEvents(), self.event_codes)
        self.event_extra_info.registerInDataStore()

    def beginRun(self):
        """Begin run method of the module"""
        self.event_number = -1

    def event(self):
        """Event method of the module"""
        self.event_number += 1
        if not self.event_extra_info.isValid():
            self.event_extra_info.create()
        elif self.event_extra_info.hasExtraInfo('EventCode'):
            B2FATAL('The EventExtraInfo object has already an EventCode field registered.')
        for event_code, event_range in self.event_ranges.items():
            if (event_range[0] <= self.event_number < event_range[1]):
                self.event_extra_info.setExtraInfo('EventCode', event_code.value)
                self.return_value(event_code.value)
                return
        # This should never happen.
        self.return_value(0)


class ExtraInfoPathSplitter(Module):
    """
    Helper module to return True if the value from ``eventExtraInfo(EventCode)``
    is included in the event_codes parameter.

    Parameters:
        event_codes (Iterable): tuple or list of EventCode.
    """

    def __init__(self, event_codes):
        """
        Initialize the class
        Parameters:
            event_codes (Iterable): tuple or list of EventCode.
        """
        import ROOT  # noqa
        super().__init__()
        #: list of event codes
        self.event_codes = [e.value for e in event_codes]
        #: PyStoreObj for EventExtraInfo to save event codes
        self.event_extra_info = ROOT.Belle2.PyStoreObj('EventExtraInfo')

    def initialize(self):
        """
        Initialize module before any events are processed
        """
        self.event_extra_info.isRequired()

    def event(self):
        """Event method of the module"""
        if not self.event_extra_info.hasExtraInfo('EventCode'):
            B2FATAL('The EventExtraInfo object has no EventCode field registered.')
        if self.event_extra_info.getExtraInfo('EventCode') in self.event_codes:
            self.return_value(1)
        else:
            self.return_value(0)
