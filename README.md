This is an edition of Eliezer Yudkowsky's fanfiction [_Harry Potter and the Methods of Rationality_](http://www.hpmor.com) as PDF files suitable for printing with a print-on-demand service.

The PDFs are generated using LuaLaTeX. The code was forked from [xenohedron/hpmor-xetex](https://github.com/xenohedron/hpmor-xetex) and incorporates most of the changes listed there.

# Distinctive features
This is an edition in six books, as suggseted by the author. The book size is 13.5 x 20.5 cm, which means around 5.3 x 8.1 inches (compared to 6 x 9 inches for the hpmor.com and xenohedron editions). The books come with beautiful matching covers (see below).

## Content
* The _Daystar's Remix of Rationality_ is NOT used: the original text is used for all chapters.
* The omake files are included (books 1 and 3), with fancy logos for all the alternate stories. Also included are the introduction (book 1), the teaser (book 2), a note from the author (book 2), and the "final exam" section (book 6).
* Britpicks in the first chapters were undone, for consistency (same as xenohedron's edition).
* The numbering of the chapters is continuous, so that book 2 starts at chapter 22 and so on. This was done to keep the original chapter numbering, which is nice since a lot of discussions online refer to the chapters by their number.
* The typo fixes from xenohedron are included. Some more were fixed (see commit log).

## Typography and LaTeX
* The main text is typeset in URW Garamond 10.5 point.
* Parseltongue is typeset using a different typeface (Alegreya Sans Light Italic, same as the hpmor.com edition).
* The front matter pages have been redesigned.
* A simpler style is used for the the headers and footers (compared to the hpmor.com and xenohedron editions).
* The text is typeset on a grid, so that the lines on one side of a page are aligned with the lines on the other side (this looks better especially if one can see through the paper).
* Pages are filled to the bottom, i.e. LaTeX's `\raggedbottom` is not used. Ragged bottom means that more space is occasionally left at the bottom of pages to avoid [widows and orphans](https://en.wikipedia.org/wiki/Widows_and_orphans). I prefer consistent spacing, so I instead made an effort to avoid widows, and to some extent orphans, by hand-tuning the layout of the paragraphs.
* Several typographic fixes have been made, e.g. for even spacing around section breaks, consistent use of small caps for Quotes Quill text (chapter 13) and sensible choice of font for double quotes around Parseltongue text.
* The LaTeX code has been somewhat simplified and commented.
* The code makes sure that the total number of pages is a multiple of 4, with the colophon landing on the last page.

# Covers
Lily Yao Lu has created cover images that match the book sizes. Since the files are big, they are stored in Google Drive folders that you can access by clicking the thumbnails below:

[![Book1](covers/thumbnail10-Book1.png?raw=true)](https://drive.google.com/drive/folders/0B4xgL7G7xKPJZ0J5WDdfbkR0N0E?resourcekey=0-ROE1CtqAD4Gyi4n20j9R2w&usp=sharing)
[![Book2](covers/thumbnail10-Book2.png?raw=true)](https://drive.google.com/drive/folders/0B4xgL7G7xKPJQVYxR1dBVV9FeTA?resourcekey=0-ojMkihdnNxqx2z8QJc3yNA&usp=sharing)

[![Book3](covers/thumbnail10-Book3.png?raw=true)](https://drive.google.com/drive/folders/0B4xgL7G7xKPJU2dQc1FJR1V4X3M?resourcekey=0-jwacaaFg_vD_a5J85yMXJw&usp=sharing)
[![Book4](covers/thumbnail10-Book4.png?raw=true)](https://drive.google.com/drive/folders/0B4xgL7G7xKPJZnlCNjB4dUxXd0E?resourcekey=0-FjeAMrcB4xl_rWUa2I2uCw&usp=sharing)

[![Book5](covers/thumbnail10-Book5.png?raw=true)](https://drive.google.com/drive/folders/0B4xgL7G7xKPJakN2ODN3N0U5Yzg?resourcekey=0-RMIk_sPLoiTETML7zmCHrQ&usp=sharing)
[![Book6](covers/thumbnail10-Book6.png?raw=true)](https://drive.google.com/drive/folders/0B4xgL7G7xKPJMXdSUzVrN3JiSWc?resourcekey=0-7si7NAsPBe4dWthHQzumVg&usp=sharing)

# Printing books
I printed mine using [epubli](https://www.epubli.de/). The book size matches their "notebook" format. 

For some reason, epubli rejected the PDF of the third book (`hpmor-3.pdf`), so I [reprocessed](#building-the-pdfs) the file into `hpmor-3-fixed.pdf` and that one worked.

# Building the PDFs
All the necessary files (fonts, etc.) are included in the repository. The code compiles with LuaLaTex. XeLaTeX should also work but there might be formatting issues such as bad letter and word spacing. I used LuaLaTex 0.95.0 from TeX Live 2016.

If you want to make your own edition, you should start from the `master` branch. If you want to reproduce my PDFs or fine-tune them, you should start with the `epubli-notebook` branch: this branch adds some commits for hand-tuning the layout of paragraphs, which only make sense once all the typesetting parameters are fixed (page size, margins, font size, etc.).

Before building the PDFs, you may want to set appropriate values for `\CoverAuthor` and `\PdfSourceUrl` in `hp-header.tex` (otherwise you will get warnings in the compilation logs). These values are used in the colophon, on the last page of each book.

You can build the 6 books by running `make` twice (the second run is required to build the table of contents). Or you can build a single book with e.g. `make hpmor-1.pdf`. This is the same as running `lualatex hpmor-1`, except that it parses the compilation output and shows every underfull/overfull box message in red.

For some reason, [epubli](https://www.epubli.de/) rejects the PDF of book 3. To solve this I convert it to PostScript and back to PDF:

```
pdftops hpmor-3.pdf hpmor-3.ps
ps2pdf hpmor-3.ps hpmor-3-fixed.pdf
```

## Hand-tuning the paragraphs
There is some Lua code to help hand-tuning the layout of paragraphs. You can activate it by uncommenting the lines `\input{hand-tuning}` and `\ShowLoosenessHelp` in `hp-header.tex`. This will change the color of the last line in each paragraph that can be expanded or shrunk by one line (using the `\looseness` command). The color code is as follows:

|Color|Meaning|
|-----|-------|
|Blue|Expandable|
|Red|Shrinkable|
|Magenta|Expandable and shrinkable|
|Dull cyan|Expandable with emergency stretch|
|Dull pink|Expandable with emergency stretch and shrinkable|

Note that `\ShowLoosenessHelp` will cause a significant slowdown of the compilation.
