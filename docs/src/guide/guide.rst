**************************************
User Guide
**************************************

.. toctree::
   :hidden:
   :maxdepth: 2

Tool Flow for Documentation
===========================

An open-source tool flow consists of Doxygen, DoxyRest and Sphinx is used,
as illustrated in the figure below.

.. image:: /images/doc_flow.png
   :alt: doc tool flow
   :width: 100%
   :align: center

In this flow, the API documentation is mainly extraced from source code,
while the user guide and another materials are written in reStructuredText.



HTML Doc Compilation
====================

The compilation flow is organized by a Makefile, and basic use can be as simple as:

.. code:: bash

        source /path/to/faas_tools_kit/setup.sh
        export HTML_DEST_DIR=/path/to/html_folder
        cd doc
        make
        make install

make install can be replaced by make all when everything needs to be rebuilt

.. code:: bash

        make all

Targets of this Makefile are:

:xml:           call Doxygen to extract specially comment.
:rst:           call DoxyRest to generated reStructuredText from XML.
:html:          call Sphinx to build HTML doc web site.
:install:       copies the web site to directory specified by ``HTML_DEST_DIR`` env var.
:clean:         clean the stamps to force rerun.
:ultraclean:    clean the stamps and generated files.



Customization
=============

Doxygen Cutomization
--------------------

The configuration file for Doxyrest is ``doc/Doxyfile``. Update the ``INPUT`` list to include
all files that you need to extract doxygen comment from, the result will be XML files in ``xml``
directory, which is later translated to reStructuredText files in ``rst`` directory.

DoxyRest Customization
----------------------

This tool is basically a LUA template engine, extracting info from XML and export through LUA
templates in ``doxyrest_frame`` directory.

doxyrest_frame/c_util.lua
~~~~~~~~~~~~~~~~~~~~~~~~~

The ``simplifyFilePath`` function should be updated to truncate the absolute file path relative
to the project path.

Sphinx Customization
--------------------

Customization through the ``conf.py`` file.

Mathjax extension
-----------------

Mathematical symbols like :math:`\alpha`, :math:`\sigma_{1}`, :math:`\langle`, :math:`\rangle`, etc, can be added by using Mathjax extension

For example :

.. math::
   :label: eq_a

   \begin{align}
   \underline{x}=[  x_{1}, ...,  x_{n}]^{T}
   \end{align}

But before using it, make sure that the following 3 sentences have been added in the extensions of conf.py:

.. code::

    'sphinx.ext.autodoc',
    'sphinx.ext.intersphinx',
    'sphinx.ext.mathjax',

And also add the following sentences at the end of ``conf.py``:

.. code::

    intersphinx_mapping = {'https://docs.python.org/': None}


Finally, you can refer like :eq:`eq_a`.

If you want to document Python modules, refer to `this page <https://pythonhosted.org/an_example_pypi_project/sphinx.html#auto-directives>`_.

FAQ
====

Where can I host my website?
    github/gitenterprise can automatically host static HTML pages in ``gh-pages`` branch.
    Just remember to create a ``.nojekyll`` file in root HTML directory.
