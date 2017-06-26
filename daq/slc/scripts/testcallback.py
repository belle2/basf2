from b2daq import *
import threading


class TestNSMCallback(NSMCallback):

    def __init__(self):
        super(TestNSMCallback, self).__init__()
        self.available = False

    def init(self):
        self.addInt("ival", int(0))
        self.addFloat("fval", float(0))
        self.addText("tval", "konno")
        self.available = True
        print("Init done")

    def timeout(self):
        self.setInt("ival", 100)
#       print("Timeout... %f"%float(val))


def th_func(callback):
    callback.run("store01")

if __name__ == '__main__':
    callback = TestNSMCallback()
    args = [callback]
    threading.Thread(target=th_func, args=args).start()
    print("debug-1")
    while True:
        print("debug-2")
        if callback.available:
            val = callback.getText("ARICHLV", "crate[1].slot[1].channel[7].switch")
            print("get crate[1].slot[1].channel[7].switch = %s" % val)
            print("set crate[1].slot[1].channel[7].switch = ON")
            callback.setText("ARICHLV", "crate[1].slot[1].channel[7].switch", "ON")
            break
