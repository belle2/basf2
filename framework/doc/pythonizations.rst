Pythonizations
--------------
In many important tasks one is required to fill a PyStoreArray with instances of a certain class.
One example could be when someone wants to create background overlay files starting from PXD digits.
In order to do that, one should first create the object instance in Python and then fill the PyStoreArray with it.
This procedure, however, can be optimized to make it much faster.
The solution is to create an array-oriented interface capable of filling the PyStoreArray in compiled mode,
accessing the underlying TClonesArray, thus without having to first create the object instance in Python.
In order to use this function one has to call it as a member function of the PyStoreArray, as shown in the usage example below,
and pass one by one the members of the class in question specifying their names.

Instead of just filling a PyStoreArray we can also read from it using the same logic.
In order to read the content of a PyStoreArray, one has just to call the ``readArray`` as shown in the example below,
and it will return a dictionary where each key is one member of the class into consideration.
In order to read the content of a PyStoreArray we have to first pass to the functions some empty arrays and then fill them with the class members.
The filling of the empty arrays is done by the ``fillValues`` function, which has to be implemented in the header file of every class of objects which we want
to be able to read.
This function is already implemented for the ``PXDDigits`` and for the ``CDCHits``.

``fillValues`` implementation
+++++++++++++++++++++++++++++
The implementation of the ``fillValues`` function is pretty straightforward.
We just have to fill the empty arrays passed to the function with the class members.
Since every class has a different number and type of members, this has to be implemented
specifically for each class we want to be able to read from a PyStoreArray.
The implementation for the case of ``PXDDigits`` looks like this:

.. code-block:: c

    void fillValues(unsigned short* charge, unsigned short* uCellID, unsigned short* vCellID, unsigned short* sensorID)
    {
      *charge = m_charge;
      *uCellID = m_uCellID;
      *vCellID = m_vCellID;
      *sensorID = m_sensorID;
    }


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
