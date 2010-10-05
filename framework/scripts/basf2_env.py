import os
from kbasf2 import *

#-----------------------------------------------
#             Set basf2 information
#-----------------------------------------------
basf2label     = "BASF2 (Belle Analysis Framework 2)"
basf2version   = "0.0.1"
basf2copyright = "Copyright(C) 2010 - Belle II Collaboration"
basf2authors   = ["Andreas Moll","Martin Heck","Nobu Katayama",
                  "Ryosuke Itoh","Thomas Kuhr","Kolja Prothmann",
                  "Martin Ritter","Zbynek Drasal"]


#-----------------------------------------------
#               Prepare basf2
#-----------------------------------------------

#Check for environment variables set by the belle 2 release script
envarLocalDir = os.environ.get("BELLE2_LOCAL_DIR", None)
if envarLocalDir is None:
  print """The environment variable BELLE2_LOCAL_DIR is not set. Please execute the 'setuprel' script first."""
  Exit(1)

envarSubDir = os.environ.get("BELLE2_SUBDIR", None)
if envarSubDir is None:
  print """The environment variable BELLE2_SUBDIR is not set. Please execute the 'setuprel' script first."""
  Exit(1)

#Get the architecture of the computer
unamelist = os.uname()
archstring = unamelist[0]+"_"+unamelist[4]

#Set basf2 directories
basf2dir     = envarLocalDir                            #basf2 directory
basf2libdir  = os.path.join(basf2dir,'modules',envarSubDir) #basf2 lib directory
basf2datadir = os.path.join(basf2dir,'data')            #basf2 data directory


#-----------------------------------------------
#       Create default framework object
#-----------------------------------------------

#Create default framework
fw = Framework()

#Add standard Linux shared file extension
fw.add_lib_extension(".so")

#Load all basf2 modules which are in the lib directory of the framework
fw.load_module_libs(basf2libdir)

#-----------------------------------------------
#             Print output
#-----------------------------------------------
print ">>> basf2 Python environment set"
print ">>> Framework object created: fw"

