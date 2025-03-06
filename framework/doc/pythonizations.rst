Pythonizations
--------------
In many important tasks one is required to fill a PyStoreArray with instances of a certain class.
One example could be when someone wants to create background overlay files starting from PXD digits.
In order to do that, one should first create the object instance in Python and then fill the PyStoreArray with it.
This procedure, however, can be optimized to make it much faster.
The solution is to create an array-oriented interface capable of passing the objects to the PyStoreArray
without having to first create the object instance in Python.
The idea is to write a C++ function which does the filling of the PyStoreArray accessing directly the underlying
ROOT TClonesArray, and then create a python wrapper which allows a cleaner way for the user to call the function.
Other then filling the PyStoreArray we can also read from it, and the idea is the same in both cases.

Usage example
+++++++++++++
The usage is pretty straightforward. Assume we want to fill a PyStoreArray with PXDDigits,
then we just instantiate the PyStoreArray and call the ``fillArray`` function passing all the 
members of the PXDDigit class, as in the following example:

.. code-block:: python

    store = ROOT.Belle2.PyStoreArray("PXDDigits")
    store.registerInDataStore()
    store.fillArray(sensorID=sids, uCellID=ucells, vCellID=vcells, charge=charge)

The ``readArray`` function usage is very similar. It will return a dictionary with all the 
members of the class:

.. code-block:: python

    d = store.readArray()
