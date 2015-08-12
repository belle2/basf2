import os
import sys
from basf2 import *

if len(sys.argv) != 3:
    sys.exit('Must provide enough arguments: [r/w] [global tag] (test global tag on pnnl = testAerogel)'
             )

use_central_database()
set_global_tag(sys.argv[2])

# register modules
# set parameters in modules
eventinfosetter = register_module("EventInfoSetter")
param_eventinfosetter = {'expList': [1],
                         'runList': [1],
                         'evtNumList': [1]}
eventinfosetter.param(param_eventinfosetter)

eventinfoprinter = register_module("EventInfoPrinter")

# set parameter 'wr' to read ('r') or write ('r') mode
arichdb = register_module("ARICHDatabase")
arichdb.param('wr', sys.argv[1])

# conditions database
conditionsdb = register_module("Conditions")
param_conditionsdb = {'experimentName': '0',
                      'runName': '0',
                      'globalTag': sys.argv[2]
                      }
conditionsdb.param(param_conditionsdb)

# load parameters from xml files - available at https://belle2.cc.kek.jp/browse/viewvc.cgi/svn/groups/arich/database/data/aerogel
paramloader = register_module('Gearbox')
xmlfile = 'file://%s/arich/dbdata/AerogelData.xml' % (os.getcwd())
paramloader.param('fileName', xmlfile)


# create path and add modules
main = create_path()
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(conditionsdb)
main.add_module(paramloader)
main.add_module(arichdb)

# process
process(main)
