.PHONY: clean

all: clean \
$(addsuffix .eps, \
leafrootpath \
leafroottree \
not-necessary \
not-necessary2 \
lpd \
lpd2 \
) exp-q.tmp exp-d.tmp
	
%.eps: %.dot 
	# neato -Teps $*.dot -o $@
	dot -Gconcentrate=true -Teps $*.dot -o $@

# %.pdf: %.tex.dot
# 	dot2tex -tmath $^ > $*.tex
# 	pdflatex $*.tex $@

%.tmp: %.dot
	dot2tex -tmath --figpreamble="\Huge" $< > $@
	pdflatex $@

clean:
	rm -f *.aux
	rm -f *.log
	rm -f *.tmp
	rm -f *.eps
	rm -f *.pdf
