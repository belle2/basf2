from b2daq import *
import random


class TestRCCallback(RCCallback):

    def __init__(self):
        super(TestRCCallback, self).__init__()

    def initialize(self, db):
        db.print()
        print("init done")

    def boot(self, db):
        db.print()
        print("boot done")

    def load(self, db):
        db.print()
        print("load done")

    def start(self, expno, runno):
        print("start expno=%d runno=%d" % (expno, runno))

    def stop(self):
        print("stop")

    def abort(self):
        print("abort")

    def recover(self, db):
        print("recover")

    def pause(self):
        print("pause")
        return True

    def resume(self, subno):
        print("resume")
        return True

    def monitor(self):
        return


if __name__ == '__main__':
    callback = TestRCCallback()
    callback.run("runcontrol")
