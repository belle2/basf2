import time
import b2daq

obj = b2daq.DAQDBObject("daq", "STORE_CDC@RC:local:cdc:")
record = obj.getObject("record")

print(obj.getName())
print(obj.getFieldNames())
print(record.getFieldNames())
print(obj.getNameList(True))
obj.print()
print("%d" % record.getInt("nproc"))
print("%s" % record.getText("runtype"))
