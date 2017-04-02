This is a LuaLaTeX version of Eliezer Yudkowsky's epic fanfiction:

*_Harry Potter and the Methods of Rationality_*

The code was forked from <http://github.com/xenohedron/hpmor-xetex>.  It should
still compile with XeLaTex but there might be formatting issues.

Features:
* Includes an edited version of _Daystar's Remix of Rationality_
    - Improves the flow and tone of chapters 1-4.
* Formatted in the style of the American editions of canon Harry Potter
    - Text set in 11-point URW Garamond No. 8, chapter headings set in Lumos
    - Large initial capitals at the start of each chapter with lettrine
* Spellcheck and standardization
    - Fixed some typos
    - Removed early Britpicks for consistent usage throughout
    - Removed Omake files and epigraphs
    - Standardization of formats for dates and times
    - Consistent capitalization of various words
    - Minor improvements to italics, lists, notes, and headlines
* LaTeX formatting code restructured, pruned, and somewhat commented
    - Individual .tex files for each chapter
    - Formatting and macros in common header
    - Batch file to compile in full
* Outputs six PDF files for printing the story in six volumes
    - Page size 9 in. by 6 in.
    - Bookmarks and unobtrusive links with hyperref
    - Automatic chapter numbering restarts in each subbook with ToC
* Careful typography
    - Microtype protrusion
    - Adjusted line spacing
    - Nonbreaking spaces and hyphenation
    - Em dash and ellipsis
    - Wider kerning in Lumos
    - Manual linebreaks in long chapter titles
    - Fancy headers, footers, and section breaks
