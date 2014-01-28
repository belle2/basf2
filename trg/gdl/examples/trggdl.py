#!/user/bin/env python
#
# 2014/01/28 : YI : 1st version

from basf2 import *

#...suppress messages and warnings during processing...
set_log_level(LogLevel.ERROR)
set_random_seed(0)

#...Register modules...
eventinfosetter = register_module('EventInfoSetter')
gdltrg          = register_module('TRGGDL')

#...EventInfoSetter...
eventinfosetter.param({'evtNumList': [5], 'runList': [1]})

#...EventInfoSetter...
gdltrg.param('DebugLevel', 1)
gdltrg.param('ConfigFile',
             os.path.join(os.environ['BELLE2_LOCAL_DIR'],
                          "trg/gdl/data/ftd/0.01/ftd_0.01"))

#...Create paths...
main = create_path()

#...Add modules to paths...
main.add_module(eventinfosetter)
main.add_module(gdltrg)

#...Process events...
process(main)

#...Print call statistics...
print statistics
