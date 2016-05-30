import time
import b2daq

# create empty object
obj = b2daq.DAQDBObject()

# define config name (should be unique in a table)
obj.setName("KONNO:001")

# add contents to objects
obj.addText("label", "konno")
cobj = b2daq.DAQDBObject()
cobj.addText("name", "tomoyuki")
obj.addObject("person", cobj)

# show contents in the object
obj.print()

# create new entry to database
obj.create("konno")
