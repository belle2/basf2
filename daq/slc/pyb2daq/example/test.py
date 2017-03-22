import time
import pyb2daq
hslb = pyb2daq.HSLB()
hslb.open(0)
# time.sleep(100)
hslb.test()
hslb.close()
