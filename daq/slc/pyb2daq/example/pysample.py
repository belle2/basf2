from pyb2daq import *


def init():
    callback = NSMCallback()
    callback.add('val', 0)
    callback.log(LogPriority.DEBUG, "KONNO!")


def timeout():
    callback = NSMCallback()
    callback.set('val', 100)
    callback.log(LogPriority.ERROR, "TOMOYUKI!")
