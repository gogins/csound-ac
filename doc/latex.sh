#!/bin/bash
echo "Building csound-ac.pdf reference manual..."
doxygen
cd latex
pdflatex refman.tex
pdflatex refman.tex
mv -f refman.pdf ../../csound-ac.pdf
