for_each
===========

Introduction
------------


Normally, a module's `event()` method is called once per event while any repeated processing is performed inside this call by e.g. looping over an array of input data. 
For more complex tasks, such such as modifying the behaviour of a chain of modules according to each entry in an array, this is no longer sufficient, or would lead to 
overly convoluted implementations. 
To remedy this, the framework supports sub-dividing events for a certain module chain by using `path.for_each(loopObjectName, arrayName, path)` in the steering file. 
This has the effect of calling the `event()` methods of modules in path for each entry in the array identified by arrayName. For each run through path, an object stored under the name loopObjectName will contain the array entry for this iteration. Modules can modify stored data to share it with other modules, but objects/arrays of event durability are reset after each iteration through the path and thus are not visible to following modules.

This feature is used by both the `FlavorTagger` and `FullEventInterpretation` algorithms.

For the full documentation, please take a look at the code:

.. autofunction:: basf2.Path.for_each

Usage
-----------
The following example shows the structure for one of the main use cases: building tag-side B0 candidates to match existing signal candidates.

.. code-block:: python

    import basf2 
    import modularAnalisys as ma

    path = basf2.create_path()  

    # create signal B0 candidates...  
     
    ma.buildRestOfEvent('B0:signal', path=path)  
      
    roe_path = basf2.create_path()  

    # add reconstruction of tag side to roe_path ...        
    path.for_each('RestOfEvent', 'RestOfEvents', roe_path)  

Inside `roe_path`, you can use the `isInRestOfEvent` variable to select final state particles that are part of the current loop iteration's `RestOfEvent` object.
