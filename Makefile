all: $(shell echo hpmor-{1..3}.pdf cover-{1..3}.pdf)

%.pdf: %.tex FORCE
	latexmk -lualatex --shell-escape $< | grep --color '\(Over\|Under\)full \\[hv]box\|$$'

.PHONY: FORCE all

clean:
	latexmk -C
