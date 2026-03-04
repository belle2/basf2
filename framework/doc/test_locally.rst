How to test locally
-------------------

In order to locally test your code, you must first fully build basf2. This will
also ensure that nothing in basf2 breaks due to changes in the documentation. 

To both compile basf2 and specify where the documentation output will be saved, you
can use the following command:

.. code:: bash

    scons --sphinx html

The output will be produced in ``$BELLE2_LOCAL_DIR/build/html`` and you can
navigate it with your favorite browser to check if the output is what you
expect.

If you only made changes to the ``analysis`` or ``online_book`` packages, you can also
add the ``--light`` option to your command to greatly speed up the compilation time. 

Once basf2 has been compiled, you can build the documentation any time using

.. code:: bash

    b2code-sphinx-build

If you're only interested in sphinx warnings, use

.. code:: bash

    b2code-sphinx-warnings

.. important:: 
    It is recommended to re-compile basf2 every time changes are made to the documentation
    to ensure that nothing in basf2 has been broken. 

If you are working on a remote server with port ``8XXX`` forwarded to your local
machine (as when running a Jupyter notebook), you can also start a tiny web server on
the remote machine and access it from your local machine to view the rendered
documentation:

.. code:: bash

    # log in to remote with port forwarding
    # (8XXX is a unique number of your choice)
    ssh login.cc.kek.jp -L 8XXX:localhost:8XXX
    # set up basf2
    cd ${BELLE2_LOCAL_DIR}/build/html
    python3 -m http.server 8XXX

Now navigate to ``http://0.0.0.0:8XXX`` on your local machine to see the rendered
web pages.
