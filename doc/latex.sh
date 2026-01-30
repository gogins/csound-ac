#!/bin/bash
echo "Building csound-ac.pdf reference manual..."
doxygen
cd latex
rm -f csound-ac.aux csound-ac.toc csound-ac.out csound-ac.lof csound-ac.lot csound-ac.bbl csound-ac.blg csound-ac.run.xml csound-ac.bcf csound-ac.fls csound-ac.fdb_latexmk csound-ac.log   
pdflatex -jobname=csound-ac refman.tex
pdflatex -jobname=csound-ac refman.tex