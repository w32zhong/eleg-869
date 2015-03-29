thesis-main.pdf: thesis-main.tex *.tex
	pdflatex $<	
	bibtex thesis-main.aux
	pdflatex $<
	pdflatex $<

clean: 
	rm -f *.blg *.aux *.dvi *.bbl *.lof *.log *.toc *.out *.lot
	rm -f thesis-main.pdf
