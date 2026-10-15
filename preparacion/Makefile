FILENAME = preparcial_ipc

all: $(FILENAME).pdf

$(FILENAME).pdf: $(FILENAME).tex ../assets/logo_unimagdalena.png
	pdflatex -interaction=nonstopmode -halt-on-error $(FILENAME).tex
	pdflatex -interaction=nonstopmode -halt-on-error $(FILENAME).tex
	rm -f *.aux *.log *.out

clean:
	rm -f *.pdf *.aux *.log *.out

.PHONY: all clean
