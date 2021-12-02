/*
    SPDX-FileCopyrightText: 2021 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "webvttinputformat.h"

#include "core/richdocument.h"
#include "core/subtitle.h"
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
	int end;
	const QStringRef hdr = data.midRef(6, off).trimmed();
	if(!hdr.isEmpty())
		subtitle.meta("comment.intro.0", hdr.toString());

	QVector<QStringRef> notes;
	int noteId = 0;
	staticRE$(reTime, "(?:([0-9]{2,}):)?([0-5][0-9]):([0-5][0-9])\\.([0-9]{3}) --> (?:([0-9]{2,}):)?([0-5][0-9]):([0-5][0-9])\\.([0-9]{3})\\b([^\\n]*)", REu);

	while(off < data.length()) {
		if(data.midRef(off, 5) == $("STYLE")) {
			if(!notes.isEmpty()) { // store note before style
				for(const QStringRef &note: notes)
					subtitle.meta(QByteArray("comment.top.") + QByteArray::number(noteId++), note.toString());
				notes.clear();
			}
			// NOTE: styles can't appear after first cue/line, even if we're not forbidding it
			// TODO: support styles
			end = skipTextBlock(data, off + 5);
			off = end;
			continue;
		}
		if(data.midRef(off, 4) == $("NOTE")) {
			end = skipTextBlock(data, off += 4);
			notes.push_back(data.midRef(off, end).trimmed());
			off = end;
			continue;
		}
		end = skipTextLine(data, off);
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

		if(!notes.isEmpty()) {
			QString comment;
			for(const QStringRef &note: notes)
				comment.append(note);
			notes.clear();
			line->meta("comment", comment);
		}
		subtitle.insertLine(line);
	}

	if(!notes.isEmpty()) {
		noteId = 0;
		for(const QStringRef &note: notes)
			subtitle.meta(QByteArray("comment.bottom.") + QByteArray::number(noteId++), note.toString());
		notes.clear();
	}

	return true;
}
