How to test locally
-------------------

You can test locally your changes in the Sphinx documentation by compiling your
code with the following command:

.. code:: bash

    scons --sphinx html

The output will be produced in ``$BELLE2_LOCAL_DIR/build/html`` and you can
navigate it with your favorite browser to check if the output is what you
expect.

Alternatively you can also build the sphinx documentation without building any
code with

.. code:: bash

    b2code-sphinx-build

which will be slightly faster. If you're only interested in sphinx warnings,
use

.. code:: bash

    b2code-sphinx-warnings

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
