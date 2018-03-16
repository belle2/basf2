import os
import sys
import inspect
globaldir = os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))
coredir = os.path.dirname(os.path.dirname(globaldir))

root_storage = coredir + "/Rec/"
