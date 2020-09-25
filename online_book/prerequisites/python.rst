.. _onlinebook_python:

Python
======

.. sidebar:: Overview
    :class: overview

    **External Training**: about 7 hours.

    **Teaching**: 5 min

    **Exercises**: 0 min

    **Prerequisites**:

    	* None

    **Questions**:

        * What are the key concepts of python?
        * How can I process tabular data?
        * How can I plot data?
        * How can I define functions in python?

    **Objectives**:

        * Get familiar with python

High Energy Physics (HEP) analyses are too complex to be done with pen, paper
and calculator. They usually are not even suited for spreadsheet programs like
Excel. There are multiple reasons for this. For one the data size is usually
larger than paper or spreadsheets can handle. But also the steps we take are
quite complex. You are of course welcome to try but we really don't recommend
it.

So what we need is something more powerful. For many years the HEP community
believed that only very fast and complex programming languages are powerful
enough to handle our data. So most students needed to start with C++ or even
Fortran.

And while there's nothing wrong with those languages once you mastered them, the
learning curve is very long and steep. Issues with the language have been known
to take a major fraction of students time with frustrating issues like:

* Why does it not compile?
* It crashes with an error called "segmentation violation", what's that?
* Somehow it used up all my memory and I didn't even load the data file yet.

And while it's true that once the program was finally done running the analysis
be very fast it is not necessarily efficient. Spending half a year for on a
program for it to finish in an hour instead of one month development and have it
finish in a day is maybe not the best use of students time.

So in recent years HEP has started moving to Python for analysis use: It is very
easy to learn and has very nice scientific libraries to do all kinds of things.
Some people still say python is way too slow and if you misuse it that is
certainly true. But if used correctly python is usually much easier to write and
can achieve comparable if not better speeds when compared to naive C++
implementations. Yes, if you are a master of C++ nothing can beat your execution
speed but the language is very hard to master. In contrast Python offers
sophisticated and optimized libraries for basically all relevant use cases.
Usually these include optimizations that would take years to implement in C++,
like GPU support.

Think about it: almost all of the billion dollar industry that is machine
learning is done in Python and they would not do that if it would not be
efficient.

Consequently in Belle II we make heavy use of Python which means you will need
to be familiar with it. By now you probably know what's coming next.

Luckily there is a very large amount of good python tutorials out there. We'll
stick with Software Carpentry and their `Programming with Python
<https://swcarpentry.github.io/python-novice-inflammation/>`_  introduction. We
would like you to go there and go through the introduction and then come back
here when you are done.

.. image:: swcarpentry_logo-blue.svg
    :target: https://swcarpentry.github.io/python-novice-inflammation/
    :alt: Programming with Python




Practising Python
=================

Welcome back! Now we're going to test you on your new-found knowledge in Python. We will work in a jupyter notebook, to allow you to practice using them. The main difference between using a jupyter notebook (.ipynb) and a python file (.py) is that jupyter notebooks are interactive and allow you to see what your code does each step of the way. If you were to type all of the following code into a python file and run it, you would acheive the same output (provided you save something as output).


What are the key concepts of python?
------------------------------------

As you should be aware by now, the key concepts of python include:

   * importing libraries that you wish to use
   * importing and/or storing data in different ways i.e. arrays, lists
   * writing and using (sometimes pre-defined) functions
   * writing conditions: if statements, for loops etc.
   * understanding and using errors to debug

You should be aware that there are multiple ways of running python. Either live from your terminal:
.. code-block:: bash
	   	python
	   	import numpy
	   	print(numpy.pi)
As a script from your terminal:
.. code-block:: bash
	   	python my_script.py #where this file has python commands inside
Or within a python compiler and interpreter such as Visual Studio or XCode.



A section to really cover our bases
-----------------------------------
In the case that, throughout your external Python training, you did not create a python file using bash commands:

.. admonition:: Exercise
   :class: exercise stacked

   Create a python file, import the python library 'NumPy' as `np`, and print out a phrase of your choice.

.. admonition:: Hint
   :class: xhint stacked toggle
	
   To create a file you'll need to use your bash skills. The internet is your friend.
	   
.. admonition:: Hint
   :class: xhint stacked toggle

   The specific bash command you'll need to create a new file is `touch`.

.. admonition:: Hint
   :class: xhint stacked toggle

   Add the `import` command inside your python file using your favourite editor. My personal preference is an editor called emacs. The internet is a magical place on which to learn the main keyboard commands for your chosen editor.
   
.. admonition:: Solution
   :class: solution toggle

   .. code-block:: bash

      #create your .py file and name it as you wish.
      touch my_file.py
      #open your file to edit it.
      emacs -nw my_file.py #the argument `-nw` opens the editor in your terminal.

   .. code-block:: python
      #now add the lines to your file.
      import numpy
      print("Hello world!")
      
Congratulations! You've now created your first python file. Now, run it!

.. admonition:: Exercise
   :class: exercise stacked

   Run your new python file in your terminal.

.. admonition:: Hint
   :class: xhint stacked toggle

   You'll need to get python to run your file.

.. admonition:: Solution
   :class: solution toggle

	.. code-block:: ipython3
   		python my_file.py #perhaps you'll want to use `python3` instead depending on which python you have installed. 

	.. code-block:: bash
		#To check which python version you have installed you can check in your terminal using 
	   	python #or python3
		from platform import python_version
		print(python_version())

Great! Well done! 😁 You can now create python scripts in your terminal!

From now on follow this textbook and practise your python skills in a jupyter notebook. If you are unsure how to use juptyer notebooks, go back to :ref:`onlinebook_ssh` to learn how to get them running.

Remember that everything you do in your jupyter notebook is an interactive version of your python script. 






   
Pandas Tutorial and Python Data Analysis
==========================================

This section aims to answer the question "How can I process tabular data?"


We will use the ``root_pandas`` package to read TTrees from ROOT files. 

Now, the previous sentence may have not been familiar at all to you. If so, read on. If not, feel free to skip the next paragraph.

ROOT: a nano introduction
---------------------------
ROOT files, as you'll come to be familiar with, are the main way we store our data at Belle II. Within these files are 'TTrees' known as 'trees', which are analogous to a sub-folder. For example, you may store a tree full of :math: `B` meson candidates. Within a tree you can have 'TBranches' known as 'branches'. Each branch could be one of the oodles of variables available for the particle you've stored in your tree - for example, the :math:`B` meson's invariant mass, it's daughter's momentum, it's great-great-granddaughter's cluster energy etc. etc. etc.

More information:`CERN's ROOT <https://root.cern.ch/>`
For when you need help with your root file manipulation: `CERN's ROOT Forum <https://root-forum.cern.ch/>`


Importing ROOT files and manipulating with Python
-------------------------------------------------
In this section we will learn how to import a ROOT file as a Pandas DataFrame using the ``root_pandas`` library.


``root_pandas`` needs ROOT to be installed but there is an alternative called ``uproot`` which can root files into pandas dataframes without requiring ROOT

.. code:: ipython3

	import root_pandas

Example data
------------

In previous example notebooks the simple decay mode :math:`B^0\to \phi K_S^0`, where :math:`\phi \to K^+ K^-` and :math:`K_S^0 \to \pi^+ \pi^-` was reconstructed. Now we will use these candidates to plot example distributions. This time we use the ``root_pandas`` package to read the data.

.. code:: ipython3

	# Loading an example data frame
	df = root_pandas.read_root("https://desycloud.desy.de/index.php/s/R8iModtQsa4WwYx/download?path=%2F&files=pandas_tutorial_ntuple.root")

If your files are quite large you may start to find your jupyter notebook kernel crashing - there are a few ways in which we can mitigate
  this.

 	-  ‘Chunk’ your data
  	-  Only import the columns (variables) that you will use/need.
	-  Add any cuts you can

To import the file it is similiar to before: ``df_chunk = root_pandas.read_root([filePath/fileName],'treeName', columns=Y4S_columns, chunksize=100000)`` where I have defined which columns I wish to be included in the following string:

.. code:: ipython3

	Y4S_columns = ['B0_mbc', 'B0_M', 'B0_deltae', 'B0_isSignal']

	.. code:: ipython3

		df_chunk=root_pandas.read_root(["https://desycloud.desy.de/index.php/s/R8iModtQsa4WwYx/download?path=%2F&files=pandas_tutorial_ntuple.root"], 'Y4S', columns=Y4Sbefore_columns, chunksize=1000000)

Now the data is loaded as chunks, we run through all the chunks and piece them together. This is the point at which we can add our cuts to reduce the loaded file more.

.. code:: ipython3

	cut="(B0_mbc>5.2)" #Define our cut

	df_list = []
	for chunk in df_chunk:
		chunk = chunk.query(cut) #Implement our cut!
		df_list.append(chunk)
        df = pd.concat(df_list) #Concatenate our chunks into a dataframe!

In jupyter notebooks you can display a DataFrame by calling it in a cell.

You can see the output, where each row corresponds to one candidate in our case:

.. code:: ipython3

	df

	You can take a preview of the dataframe by only showing the ``head`` of the dataframe. Try using ``tail`` for the opposite effect. (Optional: You can specify the number of rows shown in the brackets).

.. code:: ipython3

	df.head()

Each DataFrame has an index (which is in our case the number of the candidates) and a set of columns:

.. code:: ipython3

	len(df.columns)

You can access the full data stored in the DataFrame with the ``values`` object, which is a large 2D numpy matrix

.. code:: ipython3

	df.values

You can display the values of the DataFrame also sorted by a specific column:

.. code:: ipython3

	df.sort_values(by='B0_M').head()

A useful feature to quickly summarize your data is to use the descibe function:

.. code:: ipython3

	df.describe()

Selecting parts of your DataFrame
---------------------------------

Selecting columns, rows or subsets of DataFrames works in similar manner as python built in objects or numpy arrays.

Getting
-------

Selecting a colums can be performed by ``df['column_name']`` or ``df.column_name``. The result will be a pandas Series, a 1D vector.

.. code:: ipython3

	df['B0_M']

using ``df.column`` allows for completion

.. code:: ipython3

	df.B0_M.describe()

Selecting multiple columns
^^^^^^^^^^^^^^^^^^^^^^^^^^

Multiple columns can be selected by passing an array of columns:

.. code:: ipython3

	df[['B0_mbc', 'B0_M', 'B0_deltae', 'B0_isSignal']].describe()

We can assign the subset to a variable

.. code:: ipython3

	subset = df[['B0_mbc', 'B0_M', 'B0_deltae', 'B0_isSignal']]
	subset.columns

Selecting Rows
^^^^^^^^^^^^^^

Similarly to arrays in python, one can select rows via ``df[i:j]``. And single rows can be returned via ``df.iloc[i]``.

.. code:: ipython3

	df[2:10]

Vectorized Operations
---------------------

This is one of the most powerful features of pandas and numpy. Operations on a Series or DataFrame are performed element-wise.

.. code:: ipython3

	df.B0_mbc - df.B0_M

.. code:: ipython3

	# Stupid example of vectorized operations

	import numpy as np

	x = (df.B0_deltae * df.B0_et)**2 /(np.sin(df.B0_cc2)+np.sqrt(df.B0_cc5))

	2*x - 2

Adding Columns
--------------

You can easily add or remove columns in the following way:

.. code:: ipython3

	# Adding a columns:

	df['fancy_new_column'] = (df.B0_deltae * df.B0_et)**2 /(np.sin(df.B0_cc2)+np.sqrt(df.B0_cc5) + 0.1)
	df['delta_M_mbc'] = df.B0_M - df.B0_mbc

.. code:: ipython3

	df.delta_M_mbc.describe()

.. code:: ipython3

	df['fancy_new_column']

Modifying Columns
-----------------

Sometimes we want to change the type of a column. For example if we look at all the different values in the ``B0_isSignal`` column

.. code:: ipython3

	df['B0_isSignal'].unique()

We see that there are only two values. So it might make more sense to interpet this as a boolean value

.. code:: ipython3

	df['B0_isSignal'] = df['B0_isSignal'].astype(bool)
	df.B0_isSignal.value_counts()

Boolean Indexing
----------------

Boolean indexing will probably be the most used method to select parts of a dataframe. A boolean mask can be passed to the DataFrame like ``df[[True, False, False, ...]]``, where the result will only return rows with ``True``.

.. code:: ipython3

	# lets create a boolean mask:


We can now select the DataFrame by this mask

.. code:: ipython3

	df[ df.B0_deltae.abs() < 0.1 ]

	# or

	sel = df.B0_deltae.abs() < 0.1

	subset[sel].head(10)

You can use python boolean arithmetic to make more complex selections. However, we have an equivalent but much faster selection than looping over the frame

In the following cell we: \* create the three masks, \* then applied element wise logical operation

.. code:: ipython3

	sel = ((df.B0_deltae < 0) & (df.B0_mbc > 5)) | (df.B0_isSignal == 1 )
	sel

Beware: These are logical operators on arrays of truth values.

Creating subsets of DataFrames
------------------------------

Many times it will be practical to create subsets of DataFrames to easily store selections.

.. code:: ipython3

	df2 = subset[sel]

.. code:: ipython3

	df2.describe()

.. code:: ipython3

	(len(df) - len(df2) )/ len(df)

Side note
~~~~~~~~~

When creating subsets of a DataFrame, there will \ *not*\  be a hard-copy of the data in memory, the new DataFrame will only be a mask to the original one. This causes problems if you add new columns into a subset of a DataFrame, i.e.  ``df2["new"] = 42``. Only for special cases it is advised to create a hard copy of data in memory by calling ``df.copy()``.


.. admonition:: Exercise
   :class: exercise stacked

	Create two DataFrames, one for Signal and one for Background only containing ``B0_mbc``, ``B0_M`` and ``B0_deltae`` columns. Split between signal and background using the ``B0_isSignal`` column.


Statistical Operations
----------------------

There are built-in operations on pandas objects that allow us to access statistical operations easily

.. code:: ipython3

	df.min()

.. code:: ipython3

	df.max() - df.median()

Binned Statistics
-----------------

.. code:: ipython3

	df.B0_isSignal.value_counts()



.. admonition:: Exercise
   :class: exercise stacked

	Calculate the average number of candidates per event. (Use the mean and standard deviation.)


	   


Grouped Operations
------------------

One of the most powerful features of pandas is the ``groupby`` operation.

.. code:: ipython3

	subset.groupby('B0_isSignal').describe().T

	# What does the  .T do?

.. code:: ipython3

	group = df.groupby('B0_isSignal')
	group.size()

Group by muptible columns
-------------------------

.. code:: ipython3

	group = df.groupby(['exp_no', 'run_no', 'evt_no'])

.. code:: ipython3

	group.size()

This is an excellent tool to monitor statistical quantities across categories, in our physics case for instance runs and experiments
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. admonition:: Exercise
   :class: exercise stacked

	Do a best candidate selection on min(abs(B0_deltae))

.. code:: ipython3

	# create groupby

.. code:: ipython3

	# use group.column.rank(...)














A short introduction to plotting in python
==========================================

In this short tutorial we want to demonstrate the ``matplotlib`` package
used to plot in python.

.. code:: ipython3

    import matplotlib.pyplot as plt
    %matplotlib inline

Example data
------------

In previous example notebooks the simple decay mode
:math:`B^0\to \phi K_S^0`, where :math:`\phi \to K^+ K^-` and
:math:`K_S^0 \to \pi^+ \pi^-` was reconstructed. Now we will use these
candidates to plot example distributions. This time we use the
``root_pandas`` package to read the data

.. code:: ipython3

    # Loading an example data frame
    import root_pandas

.. code:: ipython3

    df = root_pandas.read_root("https://desycloud.desy.de/index.php/s/R8iModtQsa4WwYx/download?path=%2F&files=pandas_tutorial_ntuple.root").astype(float)
    df.B0_isSignal = df.B0_isSignal.astype(bool)
    df.describe()

Pandas built in histogram function
----------------------------------

There exists, if you prefer, a built in histogram function for Pandas.
The following cells show how to implement it.

.. code:: ipython3

    df.B0_mbc.hist(range=(5.2, 5.3), bins=100)

.. code:: ipython3

    df.B0_mbc.hist(range=(5.2, 5.3), bins=100, by=df.B0_isSignal)

.. code:: ipython3

    df[df.B0_isSignal==True].B0_mbc.hist(range=(5.2, 5.3), bins=100)
    df[df.B0_isSignal==False].B0_mbc.hist(range=(5.2, 5.3), bins=100, alpha=.5)

Using Matplotlib
----------------

Matplotlib however is a much more developed plotting tool that functions
well with juptyer notebooks, so this is what this tutorial will focus
on. You can compare the differences between the syntax below.

.. code:: ipython3

    h = plt.hist(df.B0_mbc, bins=100, range=(5.2, 5.3))

.. code:: ipython3

    h = plt.hist(df.B0_mbc[df.B0_isSignal], bins=100, range=(5.2, 5.3))
    h = plt.hist(df.B0_mbc[~df.B0_isSignal], bins=100, range=(5.2, 5.3))

Making your plots pretty
~~~~~~~~~~~~~~~~~~~~~~~~

Let’s face it, physicists aren’t well known for their amazing graphical
representations, but here’s our chance to shine! We can implement
matplotlib functions to make our plots GREAT. You can even choose a
colourblind friendly colour scheme!

You can have subplots:

.. code:: ipython3

    fig, axes = plt.subplots(figsize=(10,6))
    
    h = axes.hist(df.B0_mbc[df.B0_isSignal==1], 
                 bins=100, range=(5.2, 5.3), 
                 histtype='stepfilled', lw=1,
                 label="Signal", edgecolor='black')
    h = axes.hist(df.B0_mbc[df.B0_isSignal==0], 
                 bins=100, range=(5.2, 5.3), 
                 histtype='step', lw=2,
                 label="Background")
    axes.legend(loc="best")
    axes.set_xlabel(r"$M_{\mathrm{bc}}$", fontsize=18)
    axes.grid()
    axes.set_xlim(5.2, 5.3)
    fig.tight_layout()

The implementation of 2D histrograms are often very useful and are
easily done:

.. code:: ipython3

    plt.figure(figsize=(15,10))
    cut ='B0_mbc>5.2 and B0_phi_M<1.1'
    h = plt.hist2d(df.query(cut).B0_mbc, df.query(cut).B0_phi_M, bins=100)
    plt.xlabel(r"$M_{BC}$")
    plt.ylabel(r"$M(\phi)$")
    plt.savefig("2dplot.pdf")

Matplotlib now understands data frames so in almost all cases you can
just name the columns and supply the dataframe as ``data=`` argument

.. code:: ipython3

    fig, axes = plt.subplots(1,2, figsize=(20,10))
    axes[0].hist2d("B0_mbc", "B0_deltae", range=[(5.26,5.29), (-0.1,0.1)], data=df[df.B0_isSignal], bins=50);
    axes[1].hist2d("B0_mbc", "B0_deltae", range=[(5.26,5.29), (-0.1,0.1)], data=df[~df.B0_isSignal], bins=50, cmap="magma");



Task:
-----

Write a function to automatically plot a column in the DataFrame for
signal and background. Loop over all columns and produce all plots.







How can I define functions in python?
-------------------------------------



.. topic:: Authors of this lesson

     Martin Ritter (Intro) & Hannah Wakeling (Exercises)
B
