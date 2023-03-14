snakemake -s snakefile --forceall --dag | dot -Tpdf > dag.pdf  # full DAG
snakemake -s snakefile --rulegraph | dot -Tpdf > dagsimple.pdf  # simplified DAG
