/*
    SPDX-FileCopyrightText: 2021 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "richcss.h"

#include <QDebug>

using namespace SubtitleComposer;


namespace SubtitleComposer {
class RichClass {
public:
	RichClass() {}
};
}

RichCSS::RichCSS(QObject *parent)
	: QObject(parent)
{
}

RichCSS::~RichCSS()
{

}

typedef bool (*charCompare)(QChar ch);

inline static int
skipChar(const QStringRef text, int off, const charCompare &cf)
{
	auto it = text.cbegin() + off;
	const auto end = text.cend();
	// FIXME: skip comment blocks
	while(it != end && cf(*it))
		it++;
	return it - text.cbegin();
}

void
RichCSS::parse(const QStringRef &css)
{
	int off = 0;
	while(off < css.size()) {
		off = skipChar(css, off, [](QChar c){ return c.isSpace(); });
		int end = skipChar(css, off, [](QChar c){ return !c.isSpace() && c != QChar('{') && c.isPrint(); });
		QStringRef cssSel = css.mid(off, end - off);
		off = end;

		off = skipChar(css, off, [](QChar c){ return c.isSpace() || c == QChar('{'); });
		end = skipChar(css, off, [](QChar c){ return c != QChar('}'); });
		QStringRef cssStyles = css.mid(off, end - off);
		off = end;

		if(!cssSel.isEmpty() && !cssStyles.isEmpty())
			parseCssBlock(cssSel, cssStyles);
		else
			qWarning() << "invalid css - selector:" << cssSel << "rules:" << cssStyles;
	}
}

inline static bool
nonSepSel(const QChar *c)
{
	if(c->isSpace())
		return false;
	const char l = c->toLatin1();
	return l != '>' && l != '+' && l != '~';
}

static bool
copyCssChar(QChar *&dst, const QChar *&c, const QChar *se)
{
	if(*c != QChar('\\')) {
		*dst++ = *c;
		return false;
	}

	const QChar *e = c + 7; // max 6 hex digits
	uint32_t ucs32 = 0x80000000;
	const QChar *n = c + 1;
	for(;;) {
		if(n == se || n == e)
			break;
		const char d = n->toLatin1();
		if(d >= '0' && d <= '9')
			ucs32 += d - '0';
		else if(d >= 'a' && d <= 'f')
			ucs32 += d - 'a' + 10;
		else if(d >= 'A' && d <= 'F')
			ucs32 += d - 'A' + 10;
		else
			break;
		ucs32 <<= 4;
		n++;
	}
	if(ucs32 != 0x80000000) {
		// parsed unicode char
		// NOTE: Spec at https://www.w3.org/International/questions/qa-escapes says that
		//       space after 6th digit is not needed, but can be included.
		//       Their examples ignore it - we do too.
		c = n != e && n->isSpace() ? n : n - 1;
		*dst++ = QChar(ucs32 >> 4);
	} else {
		// copy backslash
		*dst++ = *c++;
		// and next char
		if(c != se)
			*dst++ = *c;
	}
	return true;
}

QString
RichCSS::parseCssSelector(QStringRef selector)
{
	QString str;
	str.resize(selector.size());
	QChar *sel = str.data();
	const QChar *c = selector.cbegin();
	const QChar *se = selector.cend();
	while(c != se && c->isSpace())
		c++;
	for(; c != se; c++) {
		if(c->isSpace()) {
			// NOTE: if c is space *p can't underflow - we skipped starting spaces
			const QChar *p = sel - 1;
			const QChar *n = c + 1;
			if(n != se && nonSepSel(p) && nonSepSel(n))
				*sel++ = QChar::Space;
			continue;
		}
		if(copyCssChar(sel, c, se))
			continue;
		if(*c == QChar('[')) {
			while(++c != se) {
				if(c->isSpace())
					continue;
				if(copyCssChar(sel, c, se))
					continue;
				if(*c == QChar(']'))
					break;
				if(*c == QChar('"')) {
					while(++c != se) {
						if(copyCssChar(sel, c, se))
							continue;
						if(*c == QChar('"'))
							break;
					}
				}
			}
		}
	}
	str.truncate(sel - str.data());
	return str;
}

inline static bool
nonSepVal(const QChar *c)
{
	if(c->isSpace())
		return false;
	const char l = c->toLatin1();
	return l != '(' && l != ')' && l != '"' && l != '\'' && l != ',';
}

static QString
cleanupCssValue(QStringRef value)
{
	QString str;
	str.resize(value.size());
	QChar *val = str.data();
	const QChar *c = value.cbegin();
	const QChar *se = value.cend();
	while(c != se && c->isSpace())
		c++;
	for(; c != se; c++) {
		if(c->isSpace()) {
			// NOTE: if c is space *p can't underflow - we skipped starting spaces
			const QChar *p = val - 1;
			const QChar *n = c + 1;
			if(n != se && nonSepVal(p) && nonSepVal(n))
				*val++ = QChar::Space;
			continue;
		}
		if(copyCssChar(val, c, se))
			continue;
		if(*c == QChar('"') || *c == QChar('\'')) {
			const QChar ce = *c;
			while(++c != se) {
				if(copyCssChar(val, c, se))
					continue;
				if(*c == ce)
					break;
			}
		}
	}
	str.truncate(val - str.data());
	return str;
}

QMap<QByteArray, QString>
RichCSS::parseCssRules(QStringRef rules)
{
	QMap<QByteArray, QString> cssRules;
	const auto ite = rules.cend();
	auto it = rules.cbegin();
	auto ie = it;
	for(;;) {
		for(; ie != ite && *ie != QChar(':'); ++ie);
		if(ie == ite)
			break;
		QStringRef cssKey = rules.mid(it - rules.cbegin(), ie - it).trimmed();

		it = ++ie;
		for(;;) {
			while(ie != ite && *ie != QChar(';') && *ie != QChar('"') && *ie != QChar('\''))
				++ie;
			if(ie == ite || *ie == QChar(';'))
				break;
			const QChar sc = *ie;
			while(ie != ite && *ie != sc)
				++ie;
			if(ie != ite)
				++ie;
		}
		QString cssValue = cleanupCssValue(rules.mid(it - rules.cbegin(), ie - it).trimmed());

		if(!cssKey.isEmpty() && !cssValue.isEmpty())
			cssRules.insert(cssKey.toLatin1(), cssValue);

		if(ie == ite)
			break;
		it = ++ie;
	}
	return cssRules;
}

void
RichCSS::parseCssBlock(QStringRef selector, QStringRef rules)
{
	QString cssSel = parseCssSelector(selector);
	QMap<QByteArray, QString> cssRules = parseCssRules(rules);
}
