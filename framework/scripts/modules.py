from basf2 import *

avModList = fw.available_modules()
print "The following modules were found (%s):" %(len(avModList))

for item in avModList:
  print ""
  print "==================="
  print '%-20s %s' % (item.type(), item.description())
  print "-------------------"

  paramList = item.available_params()
  for paramItem in paramList:
    defaultStr = ", ".join(['%s' % defaultItem for defaultItem in paramItem.default])

    forceString = ""
    if paramItem.forceInSteering: forceString = "*"

    print '%-1s %-20s %-14s %-30s %s' % (forceString, paramItem.name, paramItem.type, defaultStr, paramItem.description)

