/*
    SPDX-FileCopyrightText: 2021 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RICHCSS_H
#define RICHCSS_H

#include <QObject>
#include <QMap>

namespace SubtitleComposer {

class RichCSS : public QObject
{
	Q_OBJECT

public:
	RichCSS(QObject *parent=nullptr);
	virtual ~RichCSS();

	static inline QString parseCssSelector(const QString &selector) { return parseCssSelector(QStringRef(&selector)); }
	static QString parseCssSelector(QStringRef selector);

	static inline QMap<QByteArray, QString> parseCssRules(const QString &rules) { return parseCssRules(QStringRef(&rules)); }
	static QMap<QByteArray, QString> parseCssRules(QStringRef rules);

	void parse(const QStringRef &css);
	void parseCssBlock(QStringRef selector, QStringRef rules);
};
}

#endif // RICHCSS_H
