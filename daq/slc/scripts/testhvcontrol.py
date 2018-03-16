from b2daq import *
import random


class TestHVControlCallback(HVControlCallback):

    def __init__(self):
        super(TestHVControlCallback, self).__init__()
        self.m_sw = [[[0 for i3 in range(10)]for i2 in range(10)]for i1 in range(100)]
        self.m_vup = [[[0 for i3 in range(10)]for i2 in range(10)]for i1 in range(100)]
        self.m_vdown = [[[0 for i3 in range(10)]for i2 in range(10)]for i1 in range(100)]
        self.m_vdemand = [[[0 for i3 in range(10)]for i2 in range(10)]for i1 in range(100)]
        self.m_vlim = [[[0 for i3 in range(10)]for i2 in range(10)]for i1 in range(100)]
        self.m_clim = [[[0 for i3 in range(10)]for i2 in range(10)]for i1 in range(100)]

    def update(self):
        # performed at the interval of hv.interval
        return

    def initialize(self, db):
        return

    def setSwitch(self, crate, slot, channel, switchon):
        # set channel switch (true:ON, false:OFF)
        print("setswitch called : crate = %d, slot = %d, channel = %d, switch: %s"
              % (crate, slot, channel, ("ON" if switchon else "OFF")))
        self.m_sw[crate][slot][channel] = switchon

    def setRampUpSpeed(self, crate, slot, channel, voltage):
        # set voltage ramp up speed with unit of [V]
        print("setrampup called : crate = %d, slot = %d, channel = %d, voltage: %f"
              % (crate, slot, channel, voltage))
        self.m_vup[crate][slot][channel] = voltage

    def setRampDownSpeed(self, crate, slot, channel, voltage):
        # set voltage ramp down speed with unit of [V]
        print("setrampdown called : crate = %d, slot = %d, channel = %d, voltage: %f"
              % (crate, slot, channel, voltage))
        self.m_vdown[crate][slot][channel] = voltage

    def setVoltageDemand(self, crate, slot, channel, voltage):
        # set demand voltage with unit of [V]
        print("setvoltagedemand called : crate = %d, slot = %d, channel = %d, voltage: %f"
              % (crate, slot, channel, voltage))
        self.m_vdemand[crate][slot][channel] = voltage

    def setVoltageLimit(self, crate, slot, channel, voltage):
        # set voltage limit with unit of [V]
        print("setvoltagelimit called : crate = %d, slot = %d, channel = %d, voltage: %f"
              % (crate, slot, channel, voltage))
        self.m_vlim[crate][slot][channel] = voltage

    def setCurrentLimit(self, crate, slot, channel, current):
        # set current limit with unit of [uA]
        print("setcurrentlimit called : crate = %d, slot = %d, channel = %d, voltage: %f"
              % (crate, slot, channel, current))
        self.m_clim[crate][slot][channel] = current

    def getRampUpSpeed(self, crate, slot, channel):
        # return voltage ramp up speed with unit of [V]
        return self.m_vup[crate][slot][channel]

    def getRampDownSpeed(self, crate, slot, channel):
        # return voltage ramp down speed with unit of [V]
        return self.m_vdown[crate][slot][channel]

    def getVoltageDemand(self, crate, slot, channel):
        # return demand voltage with unit of [V]
        return self.m_vdemand[crate][slot][channel]

    def getVoltageLimit(self, crate, slot, channel):
        # return voltage limit with unit of [V]
        return self.m_vlim[crate][slot][channel]

    def getCurrentLimit(self, crate, slot, channel):
        # return current limit with unit of [uA]
        return self.m_clim[crate][slot][channel]

    def getVoltageMonitor(self, crate, slot, channel):
        # return monitored voltage with unit of [V]
        return self.m_vdemand[crate][slot][channel] + random.random() % 10 if self.m_sw[crate][slot][channel] else 0

    def getCurrentMonitor(self, crate, slot, channel):
        # return monitored current with unit of [uA]
        return 10 + random.random() % 10 if self.m_sw[crate][slot][channel] else 0

    def getSwitch(self, crate, slot, channel):
        # return status of channel swicth (true:ON, false:OFF)
        return self.m_sw[crate][slot][channel]

    def getState(self, crate, slot, channel):
        #
        #  return channel status:
        #  OFF: power off,
        #  ON: power on without error
        #  OCP: trip due to over current
        #  OVP: trip due to over voltage
        #  ERR: another error
        #  RAMPUP: ramping up voltage
        #  RAMPDOWN: ramping down voltage
        #
        return HVState.ON if self.m_sw[crate][slot][channel] else HVState.OFF


if __name__ == '__main__':
    callback = TestHVControlCallback()
    callback.run("konno")
