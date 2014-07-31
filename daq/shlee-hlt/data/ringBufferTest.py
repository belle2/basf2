#!/usr/bin/env python
# -*- coding: utf-8 -*-
import os
from basf2 import *

# Register modules
input = fw.register_module('RingBufferTest')

main = fw.create_path()

main.add_module(input)

fw.process(main)
