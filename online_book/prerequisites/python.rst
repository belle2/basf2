.. _onlinebook_python:

Python
======

.. sidebar:: Overview
    :class: overview

    **External Training**: about 7 hours.

    **Teaching**: 10 min

    **Exercises**: 10 min

    **Prerequisites**:

      * None

    **Questions**:

      * What are the key concepts of python?
      * How can I process tabular data?
      * How can I plot data?


    **Objectives**:

      * Get familiar with python
      * Understand how to manipulate and plot data with `root_pandas`

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
<https://swcarpentry.github.io/python-novice-inflammation/>`_   introduction. We
would like you to go there and go through the introduction and then come back
here when you are done.

.. image:: swcarpentry_logo-blue.svg
    :target: https://swcarpentry.github.io/python-novice-inflammation/
    :alt: Programming with Python




Practising Python
=================

Welcome back! Now we're going to test you on your new-found knowledge
in Python. We will work in a jupyter notebook, to allow you to
practice using them. The main difference between using a jupyter
notebook (.ipynb) and a python file (.py) is that jupyter notebooks
are interactive and allow you to see what your code does each step of
the way. If you were to type all of the following code into a python
file and run it, you would acheive the same output (provided you save
something as output).


What are the key concepts of python?
------------------------------------

As you should be aware by now, the key concepts of python include:

  * importing libraries that you wish to use
  * importing and/or storing data in different ways i.e. arrays, lists
  * writing and using (sometimes pre-defined) functions
  * writing conditions: if statements, for loops etc.
  * understanding and using errors to debug

You should be aware that there are multiple ways of running
python. Either interactively from your terminal:

.. code-block:: bash

  python3

>>> import numpy
>>> print(numpy.pi)

As a script from your terminal:

.. code-block:: bash

  python3 my_script.py # where this file has python commands inside

Or within a python compiler and interpreter such as Visual Studio or XCode.

The official recommended version of python is python3. Python2 is no longer supported. To check which python version you have installed you can check in your terminal using 

.. code-block:: bash

  python3 --version 

OR you could perform this in a live python session, either in your terminal or in a jupyter notebook using:

.. code:: ipython3

  from platform import python_version
  print(python_version())


A section to really cover our bases
-----------------------------------

In the case that, throughout your external Python training, you did
not create a python file using bash commands:

.. admonition:: Exercise
  :class: exercise stacked

  Create a python file, import the python library 'NumPy' as the
  shortcut `np`, and print out the value of pi.

.. admonition:: Hint
  :class: xhint stacked toggle
  
  To create a file you'll need to use your bash skills. The internet is your friend.
     
.. admonition:: Hint
  :class: xhint stacked toggle

  The specific bash command you'll need to create a new file is `touch`.

.. admonition:: Hint
  :class: xhint stacked toggle

  Add the ``import`` command inside your python file using your favourite
  editor. Previous tutorials introduced the editor 'nano' to you.
  
.. admonition:: Solution
  :class: solution toggle

  .. code-block:: bash

    # create your .py file and name it as you wish.
    touch my_file.py
    # open your file to edit it.
    emacs -nw my_file.py # the argument `-nw` opens the editor in your terminal.

  Now add the python lines to your file.

  .. code-block:: python
  
    import numpy as np
    print(np.pi)
     
Congratulations! You've now created your first python file. Now, run it!

.. admonition:: Exercise
  :class: exercise stacked

  Run your new python file in your terminal.

.. admonition:: Solution
  :class: solution toggle

  .. code-block:: ipython3

    python3 my_file.py


Great! Well done! 😁 You can now create python scripts in your terminal!

From now on follow this textbook and practise your python skills in a jupyter
notebook. If you are unsure how to use juptyer notebooks, go back to
:ref:`onlinebook_ssh` to learn how to get them running.

Remember that everything you do in your jupyter notebook is an interactive version of your python script. 






  
Pandas Tutorial and Python Data Analysis
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This section aims to answer the question "How can I process tabular data?"


We will use the ``root_pandas`` package to read TTrees from ROOT files. 

Now, the previous sentence may have not been familiar to you at all. If so, read
on. If not, feel free to skip the next paragraph.

ROOT: a nano introduction
---------------------------

ROOT files, as you'll come to be familiar with, are the main way we store our
data at Belle II. Within these files are 'TTrees' known as 'trees', which are
analogous to a sub-folder. For example, you may store a tree full of :math:`B`
meson candidates. Within a tree you can have 'TBranches' known as
'branches'. Each branch could be one of the oodles of variables available for
the particle you've stored in your tree - for example, the :math:`B` meson's
invariant mass, it's daughter's momentum, it's great-great-granddaughter's
cluster energy etc. etc. etc.

More information:`CERN's ROOT <https://root.cern.ch/>`
For when you need help with your root file manipulation: `CERN's ROOT Forum <https://root-forum.cern.ch/>`

Jupyter Notebooks
-----------------

Jupyter Notebooks are interactive notebooks that allow one to visualise code,
data and outputs in a linear and clear way. When you run a notebook you have an
operating system called a kernel that runs the code .   Each block in a jupyter
notebook is a 'cell'. These cells can be run using the kernel by clicking the
run button or by pressing `Shift + Enter`.   When you run a cell, the kernel will
process and store any variables or dataframes you define.   If your kernel
crashes, you will have to restart it.

.. admonition:: Exercise
  :class: exercise stacked

  Examine the `Cell` and `Kernel` drop down menus to see what options you have available.




Importing ROOT files and manipulating with Python
-------------------------------------------------

In this section we will learn how to import a ROOT file as a Pandas DataFrame
using the ``root_pandas`` library.


Pandas provides high-performance, easy-to-use data structures and data analysis
tools for Python, see `here
<https://pandas.pydata.org/docs/user_guide/10min.html>`.


``root_pandas`` needs ROOT to be installed but there is an alternative called
``uproot`` which can load root files into pandas dataframes without requiring
ROOT.


.. admonition:: Exercise
  :class: exercise stacked

  Start a jupyter notebook and import `root_pandas`, as well as other libraries you think you'll need.

.. admonition:: Hint
  :class: xhint stacked toggle

  You can find some help here :ref:`onlinebook_ssh`.

.. admonition:: Solution
  :class: solution toggle

  .. code:: ipython3

    import root_pandas


You can load in an example dataframe using the `read_root` function from the `root_pandas` package. 

.. code:: ipython3

  # Loading an example data frame
  df = root_pandas.read_root("https://desycloud.desy.de/index.php/s/R8iModtQsa4WwYx/download?path=%2F&files=pandas_tutorial_ntuple.root")

This code imports the `pandas_tutorial_ntuple.root` root file as a dataframe `df`. 



Investigating your DataFrame
----------------------------

In jupyter notebooks you can display a DataFrame by calling it in a cell. You
can see the output, where in our case each row corresponds to one candidate:

.. code:: ipython3

  df

Note that for the dataframe to be displayed a cell with multiple lines of 
code, one must call it at the end of the cell.


You can see a preview of the dataframe by only showing the ``head`` of the
dataframe. Try using ``tail`` for the opposite effect. (Optional: You can
specify the number of rows shown in the brackets).

.. code:: ipython3

  df.head(5)

Each DataFrame has an index (which is in our case the number of the candidates)
and a set of columns:

.. code:: ipython3

  len(df.columns)

You can access the full data stored in the DataFrame with the ``values`` object,
which is a large 2D numpy matrix

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

Selecting columns, rows or subsets of DataFrames works in similar manner as
python built in objects or numpy arrays.

Getting
^^^^^^^

Selecting a column can be performed by ``df['column_name']`` or
``df.column_name``. The result will be a pandas Series, a 1D vector. The
difference between the two options is that using ``df.column`` allows for
auto-completion.

.. code:: ipython3

  df['B0_M'].describe()
  # or
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

Similarly to arrays in python, one can select rows via ``df[i:j]``. And single
rows can be returned via ``df.iloc[i]``.

.. code:: ipython3

  df[2:10]


Vectorized Operations
---------------------

This is one of the most powerful features of pandas and numpy. Operations on a
Series or DataFrame are performed element-wise.

.. code:: ipython3

  df.B0_mbc - df.B0_M

.. code:: ipython3

  # Awful non-physical example of vectorized operations

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

Sometimes we want to change the type of a column. For example if we look at all
the different values in the ``B0_isSignal`` column by using

.. code:: ipython3

  df['B0_isSignal'].unique()

we see that there are only two values. So it might make more sense to interpet
this as a boolean value:

.. code:: ipython3

  df['B0_isSignal'] = df['B0_isSignal'].astype(bool)
  df.B0_isSignal.value_counts()


.. admonition:: Exercise
  :class: exercise stacked

  Create two DataFrames, one for Signal and one for Background only
  containing ``B0_mbc``, ``B0_M`` and ``B0_deltae`` columns. Split between
  signal and background using the ``B0_isSignal`` column.







Grouped Operations
------------------

One of the most powerful features of pandas is the ``groupby`` operation.  This is beyond the scope of the tutorial, but the user should be aware of it's existence ready for later analysis. ``groupby`` allows the user to group all rows in a dateframe by selected variables.

.. code:: ipython3

  df.groupby('B0_isSignal').describe()









A short introduction to plotting in python
==========================================

In this section we will answer 'How can I plot data?' and demonstrate the
``matplotlib`` package used to plot in python.

.. code:: ipython3

  import matplotlib.pyplot as plt
  %matplotlib inline


In previous example workshops the simple decay mode :math:`B^0\to \phi K_S^0`,
where :math:`\phi \to K^+ K^-` and :math:`K_S^0 \to \pi^+ \pi^-` was
reconstructed. Now we will use these candidates to plot example
distributions. This time we use the ``root_pandas`` package to read the data

.. code:: ipython3

  # Loading an example data frame
  import root_pandas

.. code:: ipython3

  df = root_pandas.read_root("https://desycloud.desy.de/index.php/s/R8iModtQsa4WwYx/download?path=%2F&files=pandas_tutorial_ntuple.root").astype(float)
  df.B0_isSignal = df.B0_isSignal.astype(bool)
  df.describe()

Pandas built in histogram function
----------------------------------

There exists, if you prefer, a built in histogram function for Pandas. The
following cells show how to implement it.

.. code:: ipython3

  df.B0_mbc.hist(range=(5.2, 5.3), bins=100)

.. code:: ipython3

  df.B0_mbc.hist(range=(5.2, 5.3), bins=100, by=df.B0_isSignal)

.. code:: ipython3

  df[df.B0_isSignal == True].B0_mbc.hist(range=(5.2, 5.3), bins=100)
  df[df.B0_isSignal == False].B0_mbc.hist(range=(5.2, 5.3), bins=100, alpha=.5)

Using Matplotlib
----------------

Matplotlib however is a much more developed plotting tool that functions well
with juptyer notebooks, so this is what this tutorial will focus on. You can
compare the differences between the syntax below.

.. code:: ipython3

  h = plt.hist(df.B0_mbc, bins=100, range=(5.2, 5.3))

.. code:: ipython3

  h = plt.hist(df.B0_mbc[df.B0_isSignal], bins=100, range=(5.2, 5.3))
  h = plt.hist(df.B0_mbc[~df.B0_isSignal], bins=100, range=(5.2, 5.3))

Making your plots pretty
------------------------

Let’s face it, physicists aren’t well known for their amazing graphical
representations, but here’s our chance to shine! We can implement matplotlib
functions to make our plots GREAT. You can even choose a colourblind friendly
colour scheme!

You can have subplots:

.. code:: ipython3

  fig, axes = plt.subplots(figsize=(10,6))

  h = axes.hist(df.B0_mbc[df.B0_isSignal == 1], bins=100, range=(5.2, 5.3), 
    histtype='stepfilled', lw=1, label="Signal", edgecolor='black')
  h = axes.hist(df.B0_mbc[df.B0_isSignal == 0], bins=100, range=(5.2, 5.3), 
    histtype='step', lw=2, label="Background")
  axes.legend(loc="best")
  axes.set_xlabel(r"$M_{\mathrm{bc}}$", fontsize=18)
  axes.grid()
  axes.set_xlim(5.2, 5.3)
  fig.tight_layout()

The implementation of 2D histograms are often very useful and are easily done:

.. code:: ipython3

  plt.figure(figsize=(15,10))
  cut = 'B0_mbc>5.2 and B0_phi_M<1.1'
  h = plt.hist2d(df.query(cut).B0_mbc, df.query(cut).B0_phi_M, bins=100)
  plt.xlabel(r"$M_{BC}$")
  plt.ylabel(r"$M(\phi)$")
  plt.savefig("2dplot.pdf")

Matplotlib now understands data frames so in almost all cases you can just name
the columns and supply the dataframe as ``data=`` argument

.. code:: ipython3

  fig, axes = plt.subplots(1,2, figsize=(20,10))
  axes[0].hist2d("B0_mbc", "B0_deltae", range=[(5.26,5.29), (-0.1,0.1)], df[df.B0_isSignal], bins=50);
  axes[1].hist2d("B0_mbc", "B0_deltae", range=[(5.26,5.29), (-0.1,0.1)], data=df[~df.B0_isSignal], bins=50, cmap="magma");



.. admonition:: Exercise
  :class: exercise stacked

  Write a function to automatically plot a column in the DataFrame for
  signal and background. Loop over all columns and produce all plots.


Dealing with large files in a jupyter notebook
----------------------------------------------

If your files are quite large you may start to find your jupyter notebook kernel
crashing - there are a few ways in which we can mitigate this.

  -   ‘Chunk’ your data
  -   Only import the columns (variables) that you will use/need.
  -   Add any cuts you can

To import the file using chunking there are some slight differences in the code:
``df_chunk = root_pandas.read_root([filePath/fileName],'treeName',
columns = Y4S_columns, chunksize=100)``. Here I have defined which columns I wish
to be included in the following string:

.. code:: ipython3

  Y4S_columns = ['B0_mbc', 'B0_M', 'B0_deltae', 'B0_isSignal']

.. admonition:: Exercise
  :class: exercise stacked

  Load your dataframe as chunks.

.. admonition:: Solution
  :class: solution toggle  

  .. code:: ipython3

    df_chunk=root_pandas.read_root(["https://desycloud.desy.de/index.php/s/R8iModtQsa4WwYx/download?path=%2F&files=pandas_tutorial_ntuple.root"], 'Y4S', columns=Y4S_columns, chunksize=100000)

Now the data is loaded as chunks, we 'loop' over or run through all the chunks
and piece them together. This is the point at which we can add our cuts to
reduce the loaded, chunked file more.

.. code:: ipython3

  cut="(B0_mbc>5.2)" # Define our cut

  df_list = []
  for chunk in df_chunk:
    chunk = chunk.query(cut) # Implement our cut!
    df_list.append(chunk)
    df = pd.concat(df_list) # Concatenate our chunks into a dataframe!






.. topic:: Authors of this lesson

  Martin Ritter (Intro)

  Hannah Wakeling (Exercises)
