#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *


def add_gdl_trigger(path):
    """
    add the gdl module to path
    """
    trggdl = register_module('TRGGDL')
    path.add_module(trggdl)
