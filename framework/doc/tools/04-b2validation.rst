Tools for Physics Validation of the Software
++++++++++++++++++++++++++++++++++++++++++++

``b2validation``: Run Physics Validation
----------------------------------------

This program is used to run the full physics validation suite

.. argparse::
    :module: validationfunctions
    :func: get_argument_parser
    :prog: b2validation
    :nodefault:
    :nogroupsections:
    :absolutecommand:

``b2validation-server``: Run Webserver Displaying the Phyiscs Validation Results
--------------------------------------------------------------------------------

With ``b2validation-server`` you can run a local webserver to display the
results of a physics validation performed with ``b2validation``.

.. argparse::
    :module: validationserver
    :func: get_argument_parser
    :prog: b2validation-server
    :nodefault:
    :nogroupsections:
    :absolutecommand:

``b2validation-bisect``: Find commits which introduced validation degradation
-----------------------------------------------------------------------------

.. argparse::
    :filename: validation/tools/b2validation-bisect
    :func: get_argument_parser
    :prog: b2validation-bisect
    :nodefault:
    :nogroupsections:
    :absolutecommand:
