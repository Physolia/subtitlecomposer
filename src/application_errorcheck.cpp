/*
    SPDX-FileCopyrightText: 2010-2020 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "config.h"

#include "application.h"
#include "core/subtitleiterator.h"
#include "errors/finderrorsdialog.h"
#include "errors/errorfinder.h"
#include "gui/treeview/lineswidget.h"

using namespace SubtitleComposer;

void
Application::selectNextError()
{
	const int idx = m_linesWidget->currentLineIndex();
	if(!m_errorFinder->findNext(idx)) {
		m_lastFoundLine = nullptr;
		m_errorFinder->find(idx, false);
	}
}

void
Application::selectPreviousError()
{
	const int idx = m_linesWidget->currentLineIndex();
	if(!m_errorFinder->findPrevious(idx)) {
		m_lastFoundLine = nullptr;
		m_errorFinder->find(idx, true);
	}
}

void
Application::detectErrors()
{
	m_lastFoundLine = nullptr;
	m_errorFinder->find(m_linesWidget->currentLineIndex());
}

void
Application::clearErrors()
{
	m_subtitle->clearErrors(RangeList(Range::full()), SubtitleLine::AllErrors);
}

void
Application::toggleSelectedLinesMark()
{
	m_subtitle->toggleMarked(m_linesWidget->selectionRanges());
}

