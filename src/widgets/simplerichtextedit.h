#ifndef SIMPLERICHTEXTEDIT_H
#define SIMPLERICHTEXTEDIT_H

/*
 * SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * SPDX-FileCopyrightText: 2010-2019 Mladen Milinkovic <max@smoothware.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "core/sstring.h"

#include <KTextEdit>

#include <QKeySequence>
#include <QVector>
#include <QAction>

QT_FORWARD_DECLARE_CLASS(QEvent)
QT_FORWARD_DECLARE_CLASS(QKeyEvent)
QT_FORWARD_DECLARE_CLASS(QFocusEvent)
QT_FORWARD_DECLARE_CLASS(QMenu)

namespace SubtitleComposer {

class SimpleRichTextEdit : public KTextEdit
{
	Q_OBJECT

public:
	typedef enum {
		Undo = 0, Redo,
		Cut, Copy, Paste, Delete, Clear, SelectAll,
		ToggleBold, ToggleItalic, ToggleUnderline, ToggleStrikeOut,
		CheckSpelling, ToggleAutoSpellChecking,
		AllowTabulations, ChangeTextColor,
		ActionCount
	} Action;

	explicit SimpleRichTextEdit(QWidget *parent = 0);
	virtual ~SimpleRichTextEdit();

	inline bool hasSelection() const { return textCursor().hasSelection(); }
	inline QString selectedText() const { return textCursor().selectedText(); }

	inline bool fontBold() { return fontWeight() == QFont::Bold; }
	inline bool fontStrikeOut() { return currentFont().strikeOut(); }

	inline QAction * action(int action) const { return action >= 0 && action < ActionCount ? m_actions[action] : nullptr; }
	inline QList<QAction *> actions() const { return m_actions.toList(); }

	bool event(QEvent *event) override;

public slots:
	void setSelection(int startIndex, int endIndex);
	void clearSelection();

	inline void setFontBold(bool enabled) { setFontWeight(enabled ? QFont::Bold : QFont::Normal); }
	inline void setFontStrikeOut(bool enabled) { QTextCharFormat f; f.setFontStrikeOut(enabled); textCursor().mergeCharFormat(f); }

	inline void toggleFontBold() { setFontBold(!fontBold()); }
	inline void toggleFontItalic() { setFontItalic(!fontItalic()); }
	inline void toggleFontUnderline() { setFontUnderline(!fontUnderline()); }
	inline void toggleFontStrikeOut() { setFontStrikeOut(!fontStrikeOut()); }
	void changeTextColor();

	void deleteText();
	void undoableClear();

	inline void toggleTabChangesFocus() { setTabChangesFocus(!tabChangesFocus()); }
	inline void toggleAutoSpellChecking() { setCheckSpellingEnabled(!checkSpellingEnabled()); }

protected:
	void setupActions();
	QMenu * createContextMenu(const QPoint &mousePos);

	void contextMenuEvent(QContextMenuEvent *event) override;
	void keyPressEvent(QKeyEvent *event) override;

	void setupWordUnderPositionCursor(const QPoint &globalPos);

protected slots:
	void addToIgnoreList();
	void addToDictionary();
	void replaceWithSuggestion();

protected:
	QVector<QAction *> m_actions = QVector<QAction *>(ActionCount, nullptr);
	QMenu *m_insertUnicodeControlCharMenu;
	QTextCursor m_selectedWordCursor;
	bool m_ignoreTextUpdate = false;
};

}

#endif
