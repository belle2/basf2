Inserting Figures
-----------------

While properly documenting the code itself is the first step to take, you may want to include figures that explain
overall concepts of a package, module, or class (see :numref:`framework_modpath_diagram`).
To do this, first simply place the image file you would like to display into your ``<package>/doc>`` directory.
You can then place the image (in this case ``cat.jpg``) into the documentation by using

.. code-block:: rst

  .. _cat_picture:

  .. figure:: cat.jpg
    :width: 40em
    :align: center

    Why is it always cats?

where we have also included the reference ``cat_picture`` to use later.
This markup would display the image as

.. _cat_picture:

.. figure:: cat.jpg
  :width: 40em
  :align: center

  Why is it always cats?

Just as with other components, we can reference the picture from the text by using ``:ref:`cat_picture```.
Which then appears as a reference using the caption text as :ref:`cat_picture`.
As before, we could use the ``:numref:`` syntax to get a numbered reference displayed as :numref:`cat_picture`.
