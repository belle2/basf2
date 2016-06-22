from org.csstudio.opibuilder.scriptUtil import PVUtil

# cprs = [2046, 2047, 2050, 2051, 2052]
# for cpr in cprs:
for cpr in range(2046, 2061):
    hslb = [0, 1, 2, 3]
    for hs in hslb:
        PVUtil.writePV("nsm://set:CPR%d:cdc[%d]:mode" % (cpr, hs), "suppress")
        print("nsm://set:CPR%d:cdc[%d]:mode suppress" % (cpr, hs))
