import os
import sys

ret = os.system('basf2 -m')

# exit(ret) doesn't work?
if ret != 0:
    sys.exit(1)
