#!/usr/bin/env python
# -*- coding: utf-8 -*-

import basf2 as b2


def add_klm_trigger(path):
    """
    add the gdl module to path
    """
    trgklm = b2.register_module('KLMTrigger')
    path.add_module(trgklm)
