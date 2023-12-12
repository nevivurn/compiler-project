REPORTS := \
	1.Lexical.Analysis.pdf \
	2.Syntax.Analysis.pdf \
	3.Semantic.Analysis.pdf \
	4.Intermediate.Code.Generation.pdf \
	5.Code.Generation.pdf \
	6.SnuPLC.pdf

.PHONY: all
all: $(REPORTS)

.PHONY: clean
clean:
	rm -f $(REPORTS)
	latexmk -C

%.pdf: %.md
	pandoc $^ -o $@

.PHONY: 6.SnuPLC.pdf
6.SnuPLC.pdf: 6.SnuPLC.tex
	latexmk -pdf -pdflatex="pdflatex -interaction=nonstopmode" -use-make $^
