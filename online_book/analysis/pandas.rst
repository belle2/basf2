Pandas
======

.. sidebar:: Overview
    :class: overview

    **Length**: 10-20 min


If you are searching for alternatives to ROOT or starting off in Belle II and don't want to touch
C++, you've come to the right page. 

While you can find detailed documentation from `pandas <https://pandas.pydata.org/docs/>`_ themselves 
and an in-depth introduction in :ref:`pandatutorial_prereq`, we will try to write this assuming 
you skipped it and are just familiar with ROOT (and basic python syntax). 

This page might also be a useful page for a quick reference on how to exploit pandas. 

Importing ROOT files
---------------------------------
Please refer to `uproot <https://uproot.readthedocs.io/en/stable/index.html>` documentation for details. 

To provide quick code snippets, 

.. code:: python 

    import uproot

    file = uproot.open("path/to/dataset.root")

Assuming your ROOT file has different TTrees and you just want to access one of them, 

.. code:: python 

    tree = uproot.open("path/to/dataset.root:myTree")

    # if you need to check what branches are in your tree

    print(tree.keys())


    # if you want to use all the variables in your ROOT file

    df_full = tree.arrays(library='pd')

    # if you know you're only using a smaller subset of variables 
    
    variable_list = ["var1", "var2", "var3"]
    df = tree.arrays(variable_list, library='pd')


DataFrames 
---------------------------------
Using uproot, we've taken our ROOT file and extracted relevant variables/branches into a useful table of 
values, where the rows are an event and columns are the branches/variables. 

You can get a quick sense of what is in your dataframe via the following examples: 

.. code:: python 

    # method 1
    df.head(5) # see first five events/rows in data frame

    # method 2
    df.columns() # should be the same as variable_list or tree.keys()

    # method 3
    df.describe() # built-in tool to describe dataframe



Functionality for data analysis
-------------------------------

Think of an DataFrame as a table that you can use to compute new
columns from existing ones and filter based on various conditions.

To illustrate some preliminary examples, we will demonstrate using the following toy example: 

.. code:: python

    import pandas as pd
    import numpy as np 

    n_events = 1_000
    var1 = np.linspace(0,1, num = n_events)
    var2 = np.random.uniform(-1,1, size = n_events)
    label = np.random.randint(5, size = n_events)

    # df used in examples below 
    df = pd.DataFrame({'var1':var1, 'var2': var2,"label": label})

Method 1: Boolean indexing  
Simplest method when filtering rows based on conditions applied to individual columns. 

.. code:: python

    # keeping events with var1 > 0.5
    mask1 = df["var1"] > 0.5

    # cut on var2
    mask2 = abs(df["var2"]) < 0.1

    # select on even labels only 
    mask3 = df["label"] % 2 == 0

    # (mask1 and mask2) or (mask3)
    df_filtered = df[(mask1 & mask2) | mask3]


Method 2: Using df.loc[] accessor 
For when you need to filter both rows and columns simutaneously. 

.. code:: python

    # along with the masks we apply above, we will also only select on the variable columns
    df_filtered = df.loc[ ((df['var1'] > 0.5) & (abs(df["var2"]) > 0.1) ) | (df["label"] % 2 == 0), ["var1", "var2", "var3"]  ]

Method 3: Using df.query 
For those who like SQL-like syntax, use df.query. 

.. code:: python

    df_filtered = df.query("var1 > 0.5 and abs(var2) > 0.1 and label%2 == 0")

Method 4: Using df.isin
When you're trying to select specific values in a given column, df.isin is useful. 

.. code:: python

    # selecting only specific labels, then selecting the rest 
    signal = df[ df["label"].isin([2,4]) ]
    background = df[~df["label"].isin(signal) ].dropna() #simply reversing the boolean masks



Inspection
----------

DataFrames offer easily accessible methods to track down what actually
happened in a computation.

If you want to track how your cuts are impacting your dataframes, you can simply
look at the length of the dataframes before and after the filter. 

Additionally, you can also plot your dataframes quite easily for inspection. 
See the code snippet below: 

.. code:: python

    import plothist # library developed by Belle II collaborators for beautiful plots 
    import pandas as pd 
    import numpy as np 

    # toy example
    n_events = 1729
    var = np.random.uniform(-1,1, size = n_events)
    label = np.random.randint(2, size = n_events)

    # df used in examples below 
    df = pd.DataFrame({'var': var,"label": label})

    # plot entire histogram
    df["var"].hist(bins = 27)

    # plot signal and background separately  
    df.groupby('label').hist(column = "var",sharex=True)



Slightly advanced topics 
-------------------------

**Resampling**:

For some nontrivial tasks (ex. Machine Learning, toy studies), you will need to be able to 
select on a subset of events at random (ex. bootstrapping, creating training data, etc.). 
While you are free to create custom solutions, you can also use the following built-in funciton. 

.. code:: python

    # Sample 42 random rows with replacement
    df_num = df.sample(n=42, replace=True)

    # Sample 42% of your data w/o replacement
    df_per = df.frac(frac = 0.42, replace=False)

    # if you need to determine what wasn't sampled
    ## method 1
    df_num_remain = df.loc[df.index.difference(df_num.index)] 

    ## method 2
    df_per_remain = df[~df.isin(df_per)]

For those explicitly doing machine learning, you can also *import sklearn* and use 
*sklearn.model_selection.train_test_split* to get testing/training data with the labels separated. 

**Concatinating**:

For many analyses, you will probably be working with collections (see :ref:`GBASF2_Collections`), where you
might have a ROOT file corresponding to each collection. Naturally, you might want a way to combine dataframes 
with the same columns/keys. In this scenario, simply do 

.. code:: python

    # if you have a persistent list and want to combine them all at once. 
    total_df = pd.concat([df1, df2, df3], ignore_index = True)

The *ignore_index* option is helpful when the index value for each dataframe is meaningless. 
If you've encoded some useful information in the dataframe's index, proceed with caution. 

If, for some reason, you care about the index, you would have to be a bit more careful and merge the dataframes. 
For details, see `pandas.merge doc <https://pandas.pydata.org/docs/reference/api/pandas.merge.html>`_.


**Adding Variables**:

Let's take an example where you have a dataframe with the three momenta (:math:`p_x, p_y, p_z`) for two different electrons (e1, e2)
and you forgot to include the energy and momentum magnitude. You can simply take your existing dataframe and add them. 

.. code:: python

    # existing df with only the three-momenta and E of your electrons
    # incase you want to run it, we'll make it standalone
    # working with [p] = GeV/c
    n = 100
    df = pd.DataFrame({'event': np.linspace(0,n,n).astype(int),'e1_px':np.random.uniform(-2,2, n), 
                        'e1_py': np.random.uniform(-2,2, n), 'e1_pz': np.random.uniform(-2,2, n), 
                        'e2_px': np.random.uniform(-2,2, n), 'e2_py':np.random.uniform(-2,2, n), 
                        'e2_pz':np.random.uniform(-2,2, n)}
                    )
    m_electron = 0.000511 #GeV

    # A silly example where you forgot p and E

    # Example 1: hand-code variables and put it in
    df["e1_p"] = np.sqrt(df["e1_px"]*df["e1_px"] + df["e1_py"]**2 + np.power(df["e1_pz"],2))

    # Example 2: use a function and map values in
    df = df.assign(e1_E = df["e1_p"]*df["e1_p"] + m_electron**2)

    # Example 3: Using df.assign to do multiple columns simutaneously
    new_columns_for_e2 = {
        'e2_p': np.sqrt(df["e2_px"]**2* + df["e2_py"]**2 + df["e2_pz"]**2 ),
        'e2_E': np.sqrt(df["e2_px"]**2* + df["e2_py"]**2 + df["e2_pz"]**2 - m_electron**2) 
    }
    df = df.assign(**new_columns_for_e2)

There are a `couple other methods <https://www.geeksforgeeks.org/adding-new-column-to-existing-dataframe-in-pandas/>`_ 
but we've tried to outline a few of our favorites methods. 


.. rubric:: Author(s) of this topic

Tommy Lam  
