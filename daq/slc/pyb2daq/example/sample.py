from pyb2daq import *


def g(dbobj):
    print(dbobj.getName())
    dbobj.print()
    dbobj('input').print()
    print(dbobj('input').getFieldType('buf'))
