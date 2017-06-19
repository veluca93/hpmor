all: $(shell echo hpmor-{1..6}.pdf)

%.pdf: FORCE
	lualatex $* | grep --color '\(Over\|Under\)full \\[hv]box\|$$'

.PHONY: FORCE all
