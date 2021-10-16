.. _onlinebook_python_module:

A simple python module
=======================

This lesson will give you an idea about the structure and use of basf2 modules.
Most of the modules in our software are implemented in C++ and are made
available for analysis by `modularAnalysis`. This package consists of python
wrapper functions around the C++ modules in order to use them in the python
steering file. You have already learned about this in
:ref:`onlinebook_basf2_introduction`

C++ is very strong and fast, but usually much more complicated to read and write
than Python. For this reason the basf2 framework provides the possibility to
write modules also in Python. This can be very helpful if you want to
investigate or test something.

To put your hands on this, simply copy the code into a python file and run it
with ``basf2 my_python_module.py``. It is nothing more than a steering file with
your own class.

Minimal example
---------------

Let's begin with the following minimal example for a new python module. It is
the "Hello World" of basf2 modules. The magic happens in the class
``MinModule(basf2.Module)``. In this basic example, the class only consists of
one member function ``event`` that is called once for each event. We use the
logging function ``basf2.B2INFO()`` to print our message. To execute the model,
we need to create a path and generate dummy events. Then, our module is added to
the path and we run the path.

.. literalinclude:: steering_files/081_module.py
   :linenos:

You can see that implementing a minimal python module just takes 5 lines of code
(3 without documentation) so it's very nice for fast and quick prototyping.

.. note::
    - Python modules have to be implemented or imported in the steering file
    - Python modules are usually much slower then C++ modules but for many small
      tasks this does not make a significant difference.
    - These *hacky* modules will not appear in module list (``basf2 -m``)
    - Python modules can only be used in analysis code or private scripts.
      Only C++ modules can be added to the offical reconstruction code that is
      run for HLT or for calibration.

Detailed usage
--------------

Let's extend the minimal example class above to
show all methods which can be implemented by a Python module. As you have seen
above all the member functions are optional.

.. literalinclude:: steering_files/083_module.py
   :linenos:

Accessing Datastore Objects
---------------------------

Datastore objects can be accessed via the
`PyStoreArray <https://software.belle2.org/development/classBelle2_1_1PyStoreArray.html>`__
class and the
`PyStoreObj <https://software.belle2.org/development/classBelle2_1_1PyStoreObj.html>`__ classes.
Let's create a small module which will print the event number and information on
MCParticles, namely the PDG code. To have tracks available, we will use the
`ParticleGun` module, which generates very simple events.



.. admonition:: Exercise
   :class: exercise stacked

   Write a Python module that prints the number of charged particles per event
   and the total charge. Note: per default, the `ParticleGun` generates only one
   track per event, but you can adjust this.

.. admonition:: Hint
   :class: toggle xhint stacked

   You can find information on the Particle class in
   `doxygen <https://software.belle2.org/development/classBelle2_1_1MCParticle.html>`__.
   The `ParticleGun` has the option `nTracks`.

.. admonition:: Solution
   :class: toggle solution

   .. literalinclude:: steering_files/087_module.py
    :linenos:

.. note::

    For PyStoreObj you can access most members of the underlying class
    directly, like ``eventinfo.getEvent()`` above. However if you want to get the
    object directly or want to access a member which also exists by the same name in
    PyStoreObj you can use the obj() member to get a reference to the underlying
    object itself: ``eventinfo.obj().getEvent()``


More advanced examples
~~~~~~~~~~~~~~~~~~~~~~

- :file:`framework/examples/cdcplotmodule.py` - A full example that uses
  matplotlib to plot CDCSimHits
- :file:`framework/examples/interactive_python.py` drops into an interactive
  (i)python shell inside the event() function, allowing exploration of
  available objects and data structures
- :file:`reconstruction/examples/plot_LL_diff.py` - Gets PID log-likelihoods,
  uses relations to get corresponding MC truth and fills ROOT histograms
  accordingly

.. include:: ../lesson_footer.rstinclude

.. topic:: Author of this lesson

   Pascal Schmolz
