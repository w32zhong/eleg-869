.PHONY: clean

all: clean \
$(addsuffix .eps, \
leafrootpath \
leafroottree \
not-necessary \
not-necessary2 \
lpd \
lpd2 \
) \
$(addsuffix .pdf, \
)
	
%.eps: %.dot 
	# neato -Teps $*.dot -o $@
	dot -Gconcentrate=true -Teps $*.dot -o $@

# %.pdf: %.tex.dot
# 	dot2tex -tmath $^ > $*.tex
# 	pdflatex $*.tex $@

clean:
	rm -f *.eps
	rm -f *.pdf
