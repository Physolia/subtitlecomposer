/*
    SPDX-FileCopyrightText: 2021 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "webvttoutputformat.h"

#include "core/richdocument.h"
#include "core/subtitleiterator.h"

#include "helpers/common.h"

using namespace SubtitleComposer;


WebVTTOutputFormat::WebVTTOutputFormat()
	: OutputFormat($("WebVTT"), QStringList($("vtt")))
{}

QString
WebVTTOutputFormat::dumpSubtitles(const Subtitle &subtitle, bool primary) const
{
	QString ret;

	ret += $("WEBVTT\n\n");

	for(SubtitleIterator it(subtitle); it.current(); ++it) {
		const SubtitleLine *line = it.current();

		const Time showTime = line->showTime();
		const Time hideTime = line->hideTime();
		ret += QString::asprintf("%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\n",
					showTime.hours(), showTime.minutes(), showTime.seconds(), showTime.millis(),
					hideTime.hours(), hideTime.minutes(), hideTime.seconds(), hideTime.millis());

		const SString text = (primary ? line->primaryDoc() : line->secondaryDoc())->toRichText();
		ret += text.richString()
				.replace(QLatin1String("&amp;"), QLatin1String("&"))
				.replace(QLatin1String("&lt;"), QLatin1String("<"))
				.replace(QLatin1String("&gt;"), QLatin1String(">"));

		ret += $("\n\n");
	}
	return ret;
}
