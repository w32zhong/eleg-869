.PHONY: clean

all: $(addsuffix .eps, \
leafrootpath \
leafroottree \
)
	
%.eps: %.dot clean
	# neato -Teps $*.dot -o $@
	dot -Teps $*.dot -o $@

clean:
	rm -f *.eps
