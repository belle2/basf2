.. _maweappers:

ModularAnalysis
===============
.. automodule:: modularAnalysis
   :members:

   .. data:: modularAnalysis.analysis_main

      The 'path' argument of all modularAnalysis convenience functions has a default value ``analysis_main``.
      This `basf2.Path` object is created implictly for you. 
      Unfortunately, this is rather error-prone so it's now deprecated.

      .. warning:: 
                using ``analysis_main`` is deprecated, please create your own path in your scripts

      If your code used to look something like:
     
      .. code-block:: python
      
         from basf2 import *
         from stdCharged import stdPi
         from modularAnalysis import doSomething
         stdPi()
         doSomething()
         process(analysis_main)
      
      please replace it with your own path:

      .. code-block:: python
      
         import basf2
         from stdCharged import stdPi
         from modularAnalysis import doSomething
         mypath = basf2.Path() # create your own path (call it what you like)
         stdPi(path=mypath)
         doSomething(path=mypath)
         basf2.process(mypath)

      
      In case you want to make absolutely sure you're *not* using this default `Path`
      object, you can do:

      .. code-block:: python

          import analysisPath
          analysisPath.analysis_main = None

      *before* importing any standard lists or modularAnalysis. 
      This will force you to always specify your own path.
