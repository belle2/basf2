#!/usr/bin/env python
# -*- coding: utf-8 -*-

#
# Author : Yanliang Han
# Date:    2012-11-19
# Email:   hanyl@ihep.ac.cn
#

"""
Generate proxy for gbasf2 use
You can use "gb2_proxy_init -h" to get help
"""

import os
import sys
if sys.argv[1:]:
    args = ' "%s"' % '" "'.join(sys.argv[1:])
else:
    args = ''
sys.exit(os.system('dirac-proxy-init -M %s' % args) / 256)
