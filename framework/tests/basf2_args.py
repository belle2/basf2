import os
import sys

ret = os.system('basf2 --info')
if ret != 0:
    sys.exit(1)

ret = os.system('basf2 --version')
if ret != 0:
    sys.exit(1)

ret = os.system('basf2 -m')
if ret != 0:
    sys.exit(1)

ret = os.system('basf2 -m RootOutput')
if ret != 0:
    sys.exit(1)
