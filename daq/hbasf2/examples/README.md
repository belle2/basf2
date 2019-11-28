# Running the ZMQ-HLT Framework

The sub-folders in this example directory include different possibilities to run and test/debug the ZMQ-HLT framework apart from the unittests run on every commit.
Each folder contains a `README.md` on how to run the specific setup itself.

Currently, these tests are implemented

* `local_test`: start the ZMQ data transportation applications locally on this machine without the need for `nsmd2` or the `daq_slc` package. Useful for quick local tests of the data transportation itself. Together with the unittests, this can be used while developing new features. Runs without additional dependencies using python.
* `full_local_test`: uses a docker setup to spin up containerized machines mimicking the real HLT setup. Starts an `nsmd2` network as well as the slowcontrol apps, so the `daq_slc` package is needed as a dependency. Can be used to test the interplay of the slowcontrol apps with the data transportation as well as for first speed tests and typical workflows (SALS, SS, etc.). Storage and Event Builders are mocked.
* `hlt_test`: only runs on the real HLT hardware, but is in principle equivalent to the `full_local_test`. Storage and Event Builders are still mocked.