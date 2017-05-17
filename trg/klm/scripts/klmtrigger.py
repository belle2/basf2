#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *


def add_klm_trigger(path):
    """
    add the gdl module to path
    """
    trgklm = register_module('KLMTrigger')
    path.add_module(trgklm)
