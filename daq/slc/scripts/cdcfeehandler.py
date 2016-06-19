from b2daq import *

callback = NSMCallback()


def cdc_date_get(hslb):
    ival = hslb.readfee32(0x0010)
    year = (ival >> 24) & 0xFF
    month = (ival >> 16) & 0xFF
    day = (ival >> 8) & 0xFF
    revision = ival & 0xFF
    val = "synthesized date = %d/%02d/%02d (revision=%d)" % (year, month, day, revision)
    callback.log(LogPriority.DEBUG, val)
    return val


def cdc_firmver_get(hslb):
    val = hslb.readfee32(0x0011) & 0xFF
    return val


def cdc_format_get(hslb):
    ival = (hslb.readfee32(0x0012) >> 24) & 0x3
    val = "unknown"
    if ival == 0x1:
        val = "raw"
    elif ival == 0x2:
        val = "suppress"
    elif ival == 0x3:
        val = "raw-suppress"
    return val


def cdc_window_get(hslb):
    val = (hslb.readfee32(0x0012) >> 8) & 0xFF
    return val


def cdc_delay_get(hslb):
    val = hslb.readfee32(0x0012) & 0xFF
    return val


def cdc_dacth_get(hslb):
    val = hslb.readfee32(0x0013) & 0xFFFF
    return val


def cdc_tdcth_get(hslb):
    val = hslb.readfee32(0x0015) & 0xFFFF
    return val


def cdc_dacctl_get(hslb):
    val = hslb.readfee32(0x0015) & (0xFFFF >> 6)
    return val


def cdc_pedestal_get(dict):
    index = dict['index']
    i = index / 2
    val = dict['hslb'].readfee32(0x0020 + int(i))
    if index % 2 == 0:
        val = (val & 0xFFF >> 6)
    else:
        val = (val >> 16) & (0xFFFF >> 6)
    return val
