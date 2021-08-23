#!/usr/bin/env python
# -*- coding: utf-8 -*-

# SPDX-FileCopyrightText: 2018 Safa AlFulaij <safa1996alfulaij@gmail.com>
#
# SPDX-License-Identifier: GPL-2.0-or-later

import ranges
import subtitle

s = subtitle.instance()

# First add a “working” line
s.insertNewLine(s.linesCount(), False)

# Keep track. 2 lines = line 0 line 1
tempLine = s.line(s.linesCount()-1)

# All the lines except the last one
for line_index in range(0, s.linesCount()-1):
	line = s.line( line_index )

	# To store the modefied sublines
	combinedArray = []

	# each line in the subtitle line
	for eachSubLine in line.richPrimaryText().split("\n"):
		tempLine.setRichPrimaryText(eachSubLine)
		if tempLine.isRightToLeft():
			combinedArray.append(u"\u202B%s"%eachSubLine.decode('utf8')) # Add RLE for RTL lines
		else:
			combinedArray.append(u"\u202A%s"%eachSubLine.decode('utf8')) # Add LRE for LTR lines

	line.setRichPrimaryText("\n".join(combinedArray))

# Delete the temp line
s.removeLine(s.linesCount()-1)
