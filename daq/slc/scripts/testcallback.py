from b2daq import *


class TestNSMCallback(NSMCallback):

    def __init__(self):
        super(TestNSMCallback, self).__init__()

    def init(self):
        self.addInt("ival", int(0))
        self.addFloat("fval", float(0))
        self.addText("tval", "konno")
        print("Init done")

    def timeout(self):
        self.setInt("ival", 100)
        print("Timeout...")


if __name__ == '__main__':
    callback = TestNSMCallback()
    callback.run("store01")
