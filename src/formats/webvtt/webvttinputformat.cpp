/*
    SPDX-FileCopyrightText: 2021 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "webvttinputformat.h"

#include "core/richdocument.h"
#include "helpers/common.h"

#include <QRegularExpression>


using namespace SubtitleComposer;

WebVTTInputFormat::WebVTTInputFormat()
	: InputFormat($("WebVTT"), QStringList($("vtt")))
{
}

int
skipTextLine(const QString &str, int off)
{
	for(; off < str.length(); off++) {
		if(str.at(off) == QChar::LineFeed)
			return off + 1;
	}
	return str.length();
}

int
skipTextBlock(const QString &str, int off)
{
	for(bool prevLF = false; off < str.length(); off++) {
		const bool curLF = str.at(off) == QChar::LineFeed;
		if(prevLF && curLF)
			return off + 1;
		prevLF = curLF;
	}
	return str.length();
}


bool
WebVTTInputFormat::parseSubtitles(Subtitle &subtitle, const QString &data) const
{
	if(!data.startsWith($("WEBVTT")))
		return false;

	int off = skipTextBlock(data, 6);
	// TODO: save optional header somewhere

	staticRE$(reTime, "(?:([0-9]{2,}):)?([0-5][0-9]):([0-5][0-9])\\.([0-9]{3}) --> (?:([0-9]{2,}):)?([0-5][0-9]):([0-5][0-9])\\.([0-9]{3})\\b([^\\n]*)", REu);

	while(off < data.length()) {
		if(data.midRef(off, 5) == $("STYLE")) {
			// NOTE: styles can't appear after first cue/line
			// TODO: support styles
			off = skipTextBlock(data, off + 5);
			continue;
		}
		if(data.midRef(off, 4) == $("NOTE")) {
			// TODO: save notes somewhere
			off = skipTextBlock(data, off + 4);
			continue;
		}
		int end = skipTextLine(data, off);
		QStringRef cueId = data.midRef(off, end - off).trimmed();
		QStringRef cueTime;
		const bool hasCue = !cueId.contains($("-->"));
		off = end;
		if(hasCue) {
			end = skipTextLine(data, off);
			cueTime = data.midRef(off, end - off).trimmed();
			off = end;
		} else {
			cueTime = cueId;
			cueId.clear();
		}
		QRegularExpressionMatch m = reTime.match(cueTime);
		if(!m.isValid()) {
			qWarning() << "Invalid WEBVTT subtitle";
			return false;
		}

		const Time showTime(m.capturedRef(1).toInt(), m.capturedRef(2).toInt(), m.capturedRef(3).toInt(), m.capturedRef(4).toInt());
		const Time hideTime(m.capturedRef(5).toInt(), m.capturedRef(6).toInt(), m.capturedRef(7).toInt(), m.capturedRef(8).toInt());
		const QStringRef cueStyle = m.capturedRef(9);
		(void)cueStyle; // TODO: support styles

		end = skipTextBlock(data, off);
		const QStringRef cueText = data.midRef(off, end - off).trimmed();
		off = end;

		SString stext;
		stext.setRichString(cueText); // TODO: handle voice/class tags

		SubtitleLine *line = new SubtitleLine(showTime, hideTime);
		line->primaryDoc()->setRichText(stext, true);
		subtitle.insertLine(line);
	}

	return true;
}
