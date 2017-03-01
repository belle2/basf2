#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2
from cdctrigger import add_cdc_trigger


def add_trigger(path, components=None):
    """
    This function adds the trigger modules for specified components to path.
    """
    if components is None or 'CDC' in components:
        add_cdc_trigger(path)
