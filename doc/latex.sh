#!/bin/bash
echo "Building csound-ac.pdf reference manual..."
doxygen
cd latex
pdflatex --jobname=csound-ac refman.tex
pdflatex --jobname=csound-ac refman.tex
# mv -f refman.pdf ../../csound-ac.pdf
