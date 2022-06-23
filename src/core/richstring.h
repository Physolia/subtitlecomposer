/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RICHSTRING_H
#define RICHSTRING_H

#include <cstring>

#include <QColor>
#include <QDataStream>
#include <QDebug>
#include <QList>
#include <QString>

QT_FORWARD_DECLARE_CLASS(QRegExp)
QT_FORWARD_DECLARE_CLASS(QRegularExpression)

namespace SubtitleComposer {
class RichString;
class RichStringStyle;
}

QDataStream & operator<<(QDataStream &stream, const SubtitleComposer::RichString &string);
QDataStream & operator>>(QDataStream &stream, SubtitleComposer::RichString &string);

namespace SubtitleComposer {
class RichStringList : public QList<RichString>
{
public:
	RichStringList();
	RichStringList(const RichString &str);
	RichStringList(const RichStringList &list);
	RichStringList(const QList<RichString> &list);
	RichStringList(const QStringList &list);
	RichStringList(const QList<QString> &list);

	RichString join(const RichString &sep) const;

	RichStringList& operator=(const RichStringList& other) = default;
};

class RichString : public QString
{
public:
	typedef enum {
		Bold = 0x1,
		Italic = 0x2,
		Underline = 0x4,
		StrikeThrough = 0x8,
		Color = 0x10,
		AllStyles = Bold | Italic | Underline | StrikeThrough | Color
	} StyleFlag;

	RichString(const QString &string = QString(), quint8 styleFlags = 0, QRgb styleColor = 0);
	RichString(const RichString &richstring);
	RichString & operator=(const RichString &richstring);

	~RichString();

	inline QString string() const { return *this; }
	void setString(const QString &string, quint8 styleFlags = 0, QRgb styleColor = 0);

	QString richString() const;
	RichString & setRichString(const QStringRef &string);
	inline RichString & setRichString(const QString &string) { return setRichString(QStringRef(&string)); }

	quint8 styleFlagsAt(int index) const;
	void setStyleFlagsAt(int index, quint8 styleFlags) const;
	QRgb styleColorAt(int index) const;
	void setStyleColorAt(int index, QRgb rgbColor) const;

	int cummulativeStyleFlags() const;
	bool hasStyleFlags(int styleFlags) const;
	RichString & setStyleFlags(int index, int len, int styleFlags);
	RichString & setStyleFlags(int index, int len, int styleFlags, bool on);
	RichString & setStyleColor(int index, int len, QRgb color);

	void clear();

	RichString & insert(int index, QChar ch);
	RichString & insert(int index, const QString &str);
	RichString & insert(int index, const RichString &str);
	RichString & append(QChar ch);
	RichString & append(const QString &str);
	RichString & append(const RichString &str);
	RichString & prepend(QChar ch);
	RichString & prepend(const QString &str);
	RichString & prepend(const RichString &str);
	RichString & operator+=(QChar ch);
	RichString & operator+=(const QString &str);
	RichString & operator+=(const RichString &str);

	RichString & remove(int index, int len);
	RichString & remove(const QString &str, Qt::CaseSensitivity cs = Qt::CaseSensitive);
	RichString & remove(QChar ch, Qt::CaseSensitivity cs = Qt::CaseSensitive);
	RichString & remove(const QRegExp &rx) { return replace(rx, QString()); }
	RichString & remove(const QRegularExpression &rx) { return replace(rx, QString()); }

	RichString & replace(int index, int len, const QString &replacement);
	RichString & replace(int index, int len, const RichString &replacement);
	RichString & replace(const QString &before, const QString &after, Qt::CaseSensitivity cs = Qt::CaseSensitive);
	RichString & replace(const QString &before, const RichString &after, Qt::CaseSensitivity cs = Qt::CaseSensitive);
	RichString & replace(QChar before, QChar after, Qt::CaseSensitivity cs = Qt::CaseSensitive);
	RichString & replace(QChar ch, const QString &after, Qt::CaseSensitivity cs = Qt::CaseSensitive);
	RichString & replace(QChar ch, const RichString &after, Qt::CaseSensitivity cs = Qt::CaseSensitive);
	RichString & replace(const QRegExp &regExp, const QString &replacement);
	RichString & replace(const QRegExp &regExp, const RichString &replacement);
	RichString & replace(const QRegularExpression &regExp, const QString &replacement);
	RichString & replace(const QRegularExpression &regExp, const RichString &replacement);

	RichStringList split(const QString &sep, QString::SplitBehavior behavior = QString::KeepEmptyParts, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
	RichStringList split(const QChar &sep, QString::SplitBehavior behavior = QString::KeepEmptyParts, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
	RichStringList split(const QRegExp &sep, QString::SplitBehavior behavior = QString::KeepEmptyParts) const;

	RichString left(int len) const;
	RichString right(int len) const;
	RichString mid(int index, int len = -1) const;

	RichString toLower() const;
	RichString toUpper() const;
	RichString toTitleCase(bool lowerFirst) const;
	RichString toSentenceCase(bool lowerFirst, bool *cont) const;

	RichString simplified() const;
	RichString trimmed() const;

	static void simplifyWhiteSpace(QString &text);
	void simplifyWhiteSpace();

	bool operator==(const RichString &richstring) const;
	bool operator!=(const RichString &richstring) const;

	inline int length() const { return QString::length(); }

private:
	inline int length(int index, int len) const { return len < 0 || (index + len) > length() ? length() - index : len; }

private:
	friend QDataStream & ::operator<<(QDataStream &stream, const RichString &string);
	friend QDataStream & ::operator>>(QDataStream &stream, RichString &string);
	friend struct SSHelper;
	RichStringStyle *m_style;
};

inline RichString &
RichString::append(QChar ch)
{
	return insert(length(), ch);
}

inline RichString &
RichString::append(const QString &str)
{
	return insert(length(), str);
}

inline RichString &
RichString::append(const RichString &str)
{
	return insert(length(), str);
}

inline RichString &
RichString::prepend(QChar ch)
{
	return insert(0, ch);
}

inline RichString &
RichString::prepend(const QString &str)
{
	return insert(0, str);
}

inline RichString &
RichString::prepend(const RichString &str)
{
	return insert(0, str);
}

inline RichString &
RichString::operator+=(const QChar ch)
{
	return append(ch);
}

inline RichString &
RichString::operator+=(const QString &str)
{
	return append(str);
}

inline RichString &
RichString::operator+=(const RichString &str)
{
	return append(str);
}

inline RichString &
RichString::remove(int index, int len)
{
	return replace(index, len, QString());
}

inline RichString &
RichString::remove(const QString &str, Qt::CaseSensitivity cs)
{
	return replace(str, QString(), cs);
}

inline RichString &
RichString::remove(QChar ch, Qt::CaseSensitivity cs)
{
	return replace(ch, QString(), cs);
}

inline bool
RichString::operator==(const RichString &richstring) const
{
	return !operator!=(richstring);
}

}

#endif
