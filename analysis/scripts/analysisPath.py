# !/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import create_path
analysis_main = create_path()
analysis_main._deprecation_warning = """\
Use of analysis_main is deprecated, please update your scripts.
You should now create a basf2.Path, and add modules there. If you are
using modularAnalysis convenience functions, you should pass it into them.

If your code looked something like:

   from basf2 import *
   from modularAnalysis import doSomething
   doSomething()
   process(analysis_main)

please replace it with your own path:

   import basf2
   from modularAnalysis import doSomething
   mypath = basf2.Path() # create your own path (call it what you like)
   doSomething(path=mypath)
   basf2.process(mypath)

For more hints, check the documentation
  https://software.belle2.org/framework/doc/index-03-framework.html#modules-and-paths
or ask a question at
  https://questions.belle2.org

              """
