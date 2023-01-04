Logging
-------

The Logging system of the Belle II Software is rather flexible and allows
extensive configurations. In the most simple case a call to
`set_log_level <basf2.set_log_level>` is all that is needed to set the minimum severity of
messages to be printed. However in addition to this global log level one can
set the log level for specific packages and even for individual modules
separately. The existing log levels are defined as

.. autoclass:: basf2.LogLevel

.. autofunction:: basf2.set_log_level
.. autofunction:: basf2.set_debug_level
.. attribute:: basf2.logging

    An instance of the `LogPythonInterface <basf2.LogPythonInterface>` class for fine grained control
    over all settings of the logging system.

Creating Log Messages
+++++++++++++++++++++

Log messages can be created in a very similar way in python and C++. You can
call one of the logging functions like `B2INFO` and supply the message as
string, for example

.. code-block:: python

   B2INFO("This is a log message of severity INFO")

In Python you can supply multiple arguments which will all be converted to
string and concatenated to form the log message

.. code-block:: python

   for i in range(1,4):
       B2INFO("This is log message number ", i)

which will produce

.. code-block:: text

  [INFO] This is log message number 1
  [INFO] This is log message number 2
  [INFO] This is log message number 3

This works almost the same way in C++ except that you need the ``<<`` operator
to construct the log message from multiple parts

.. code-block:: c++

   for(int i=1; i<4; ++i) {
     B2INFO("This is log message " << i << " in C++");
   }

.. _logging_logvariables:

.. rubric:: Log Variables

.. versionadded:: release-03-00-00

However, the log system has an additional feature to include variable parts in
a fixed message to simplify grouping of similar log messages: If a log message only
differs by a number or detector name it is very hard to filter repeating
messages. So we have log message variables which can be used to specify varying
parts while having a fixed message.

In Python these can just be given as keyword arguments to the logging functions

.. code-block:: python

   B2INFO("This is a log message", number=3.14, text="some text")

In C++ this again almost works the same way but we need to specify the
variables a bit more explicitly.

.. code-block:: c++

   B2INFO("This is a log message" << LogVar("number", 3.14) << LogVar("text", "some text"));

In both cases the names of the variables can be chosen feely and the output
should be something like

.. code-block:: text

   [INFO] This is a log message
           number = 3.14
           text = some text

.. rubric:: Logging functions


To emit log messages from within Python we have these functions:

.. autofunction:: basf2.B2DEBUG
.. autofunction:: basf2.B2INFO
.. autofunction:: basf2.B2RESULT
.. autofunction:: basf2.B2WARNING
.. autofunction:: basf2.B2ERROR
.. autofunction:: basf2.B2FATAL

The same functions are available in C++ as macros once you included ``<framework/logging/Logger.h>``

The Logging Configuration Objects
+++++++++++++++++++++++++++++++++

The `logging <basf2.logging>` object provides a more fine grained control over the
settings of the logging system and should be used if more than just a global
log level should be changed


.. autoclass:: basf2.LogPythonInterface
   :members:

.. autoclass:: basf2.LogConfig
   :members:

.. autoclass:: basf2.LogInfo
