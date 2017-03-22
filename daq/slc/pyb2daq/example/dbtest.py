from b2daq import *

obj = DAQDBObject("daq", "STORE_CDC@RC:local:cdc:")

# show config name
print(obj.getName())

# show field names in the object
print(obj.getFieldNames())

# show full pathes of variables in the object
print(obj.getNameList(True))

# show all varialble in the object
# obj.print()

# find child object
record = obj.getObject("record")
print("%d" % record.getInt("nproc"))
print("%s" % record.getText("runtype"))
