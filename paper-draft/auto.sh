#!/bin/bash
pdflatex sigproc-sp.tex
bibtex *.aux
pdflatex sigproc-sp.tex
pdflatex sigproc-sp.tex
