.. _maweappers:

ModularAnalysis
===============
.. automodule:: modularAnalysis
   :members:

   .. data:: modularAnalysis.analysis_main

      The 'path' argument of all functions has a default value 'analysis_main',
      which is created implictly. In case you don't want to use this default
      `Path` object, you can do

          >>> import analysisPath
          >>> analysisPath.analysis_main = None

      *before* importing modularAnalysis. This will force you to always specify
      your own path.
