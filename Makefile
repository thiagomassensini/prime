.PHONY: all clean view

# Default target
all: paper.pdf

# Build the PDF
paper.pdf: paper.tex references.bib
	pdflatex paper.tex
	bibtex paper
	pdflatex paper.tex
	pdflatex paper.tex

# Clean auxiliary files
clean:
	rm -f *.aux *.log *.bbl *.blg *.out *.toc *.lof *.lot

# Clean everything including PDF
distclean: clean
	rm -f paper.pdf

# View the PDF (requires a PDF viewer)
view: paper.pdf
	xdg-open paper.pdf 2>/dev/null || open paper.pdf 2>/dev/null || echo "Please open paper.pdf manually"
