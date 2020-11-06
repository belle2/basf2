.. _b2conditionsdb:

``b2conditionsdb``: Conditions DB interface
+++++++++++++++++++++++++++++++++++++++++++

.. argparse::
    :module: conditions_db.cli_main
    :func: get_argument_parser
    :prog: b2conditionsdb
    :nodefault:
    :nogroupsections:
    :absolutecommand:

.. _b2conditionsdb-extract:

``b2conditionsdb-extract``: Extract payload for a run range and save as TTree
-----------------------------------------------------------------------------

.. versionadded:: release-03-00-00

.. argparse::
    :filename: framework/tools/b2conditionsdb-extract
    :func: get_argument_parser
    :prog: b2conditionsdb-extract
    :nodefaultconst:
    :nogroupsections:

.. _b2conditionsdb-recommend:

``b2conditionsdb-recommend``: Recommend a global tag to analyse a given file
----------------------------------------------------------------------------

.. versionadded:: release-03-00-00

.. argparse::
    :filename: framework/tools/b2conditionsdb-recommend
    :func: create_argumentparser
    :prog: b2conditionsdb-recommend
    :nodefaultconst:
    :nogroupsections:


.. _b2conditionsdb-request:

``b2conditionsdb-request``: Request inclusion of payloads in official global tag
--------------------------------------------------------------------------------

.. versionadded:: release-03-00-00

.. argparse::
    :filename: framework/tools/b2conditionsdb-request
    :func: get_argument_parser
    :prog: b2conditionsdb-request
    :nodefaultconst:
    :nogroupsections:
