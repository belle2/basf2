from org.csstudio.opibuilder.scriptUtil import PVUtil

cprs = [2061]
for cpr in cprs:
    hslb = [0, 1, 2, 3]
    for hs in hslb:
        PVUtil.writePV("nsm://set:CPR%d:hslb[%d]:boot" % (cpr, hs), "on")
        print("nsm://set:CPR%d:hslb[%d]:boot on" % (cpr, hs))
