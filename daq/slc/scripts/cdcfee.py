from b2daq import *
from cdcfeehandler import *

callback = NSMCallback()


def set_non(hslb, val):
    return True


def init(hslb, db):
    print('initing')
    vname = ('cdc[%d].' % hslb.get_finid())
    callback.addFloat(vname + 'temp', 0)
    callback.addText(vname + 'date', cdc_date_get, set_non, hslb)
    callback.addInt(vname + 'firmver', cdc_firmver_get, set_non, hslb)
    callback.addText(vname + 'format', cdc_format_get, set_non, hslb)
    callback.addInt(vname + 'window', cdc_window_get, set_non, hslb)
    callback.addInt(vname + 'delay', cdc_delay_get, set_non, hslb)
    callback.addInt(vname + 'dacth', cdc_dacth_get, set_non, hslb)
    callback.addInt(vname + 'tdcth', cdc_tdcth_get, set_non, hslb)
    callback.addInt(vname + 'dacctl', cdc_dacctl_get, set_non, hslb)
    for i in range(0, 48):
        callback.addInt(vname + ('pedestal[%d]' % i),
                        cdc_pedestal_get, set_non,
                        {'hslb': hslb, 'index': i})


def boot(hslb, db):
    print('booting')


def load(hslb, db):
    print("loading slot-%d" % hslb.get_finid())
    # setting CDC control (data format, window and delay)
    val = int(0)
    mode = db.getText("mode")
    if mode == "suppress":
        callback.log(LogPriority.DEBUG, "suppress mode")
        val = 2 << 24
    elif mode == "raw":
        callback.log(LogPriority.DEBUG, "raw mode")
        val = 1 << 24
    elif mode == "raw-suppress":
        callback.log(LogPriority.DEBUG, "raw-suppress mode")
        val = 3 << 24
    else:
        callback.log(LogPriority.DEBUG, "no mode selected")
        val = 0
    val |= (db.getObject("window").getInt("val") & 0xFF) << 8
    val |= (db.getObject("delay").getInt("val") & 0xFF)
    hslb.writefee32(0x0012, val)
    # setting ADC threshold
    val = db.getObject("adcth").getInt("val") & 0xFFFF
    hslb.writefee32(0x0013, val)
    # setting DAC control
    val = db.getObject("tdcth").getInt("val") & 0x7FFF
    hslb.writefee32(0x0015, val)


def monitor(hslb):
    hslb.writefee32(0X0016, 0x00480F00)
    # time.sleep(0.5)
    hslb.writefee32(0X0016, 0x00400400)
    # time.sleep(0.5)
    hslb.writefee32(0X0016, 0x004120F0)
    # time.sleep(0.5)
    # temperature
    hslb.writefee32(0X0016, 0x80000000)
    # time.sleep(0.5)
    v0 = hslb.readfee32(0X0016) & 0xffff
    temp = (v0 / 64.) * (503.975 / 1024.) - 273.15
    vname = "cdc[%d]." % hslb.get_finid()
    callback.set(vname + "temp", temp)
    if temp > 70.:
        callback.log(LogPriority.WARNING,
                     "slot-%d Too high : %3.3f" % (slot, temp))
