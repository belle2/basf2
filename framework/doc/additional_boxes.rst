Additional boxes for the online lessons
---------------------------------------

.. sidebar:: Overview
    :class: overview

    **Teaching**: 10 min

    **Exercises**: 5 min

    **Prerequisites**: None

    **Questions**:

    * What is a particle list?
    * What are final state particles?
    * How can I specify decays?

    **Objectives**:

    * Reconstruct particles

.. code:: rst

    .. sidebar:: Overview
        :class: overview

        **Teaching**: 10 min

        **Exercises**: 5 min

        **Prerequisites**: None

        **Questions**:

            * What is a particle list?
            * What are final state particles?
            * How can I specify decays?

        **Objectives**:

            * Reconstruct particles


No hands-on training without some nice exercises:

.. code:: rst

    .. admonition:: Question
       :class: exercise stacked

       What's the object-oriented way to get rich?

    .. admonition:: Hint
       :class: toggle xhint stacked

       Think about relationships between classes!

    .. admonition:: Solution
       :class: toggle solution

       Inheritance.

.. admonition:: Question
   :class: exercise stacked

   What's the object-oriented way to get rich?

.. admonition:: Hint
   :class: toggle xhint stacked

   Think about relationships between classes!

.. admonition:: Solution
   :class: toggle solution

   Inheritance.

**Notes**:

* You can also use ``.. admonition:: Exercise`` for an exercise rather than a question
  (in general the content after ``admonition::`` will always be the title).
* The ``stacked`` class removes the space after the question block, so that the solution
  block is directly joined. If you want to write some text after your question, simply
  remove this class.
* Note that the class for the hint box is ``xhint`` (short for exercise-hint),
  not ``hint`` (the latter is already in use for "normal" hint boxes)

.. code:: rst

    .. admonition:: Key points
        :class: key-points

        * There are 10 kinds of people in this world:
          Those who understand binary, those who don't,
          and those who weren't expecting a base 3 joke.

.. admonition:: Key points
    :class: key-points

    * There are 10 kinds of people in this world:
      Those who understand binary, those who don't,
      and those who weren't expecting a base 3 joke.
