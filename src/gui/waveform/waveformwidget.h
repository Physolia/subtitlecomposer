#ifndef WAVEFORMWIDGET_H
#define WAVEFORMWIDGET_H
/*
 * Copyright (C) 2010-2021 Mladen Milinkovic <max@smoothware.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "core/time.h"
#include "core/subtitle.h"
#include "videoplayer/waveformat.h"

#include <QList>
#include <QTimer>
#include <QWidget>

QT_FORWARD_DECLARE_CLASS(QRegion)
QT_FORWARD_DECLARE_CLASS(QPolygon)
QT_FORWARD_DECLARE_CLASS(QProgressBar)
QT_FORWARD_DECLARE_CLASS(QToolButton)
QT_FORWARD_DECLARE_CLASS(QScrollBar)
QT_FORWARD_DECLARE_CLASS(QPropertyAnimation)
QT_FORWARD_DECLARE_CLASS(QToolButton)
QT_FORWARD_DECLARE_CLASS(QBoxLayout)

namespace SubtitleComposer {
class WaveBuffer;
class WaveRenderer;
struct WaveZoomData;

class WaveformWidget : public QWidget
{
	Q_OBJECT

private:
	enum DragPosition {
		DRAG_NONE = 0,
		DRAG_SHOW,
		DRAG_LINE,
		DRAG_HIDE
	};

public:
	explicit WaveformWidget(QWidget *parent = nullptr);
	virtual ~WaveformWidget();

	inline double windowSize() const { return (m_timeEnd - m_timeStart).toMillis(); }
	double windowSizeInner(double *autoScrollPadding = nullptr) const;

	void setWindowSize(const double size);

	QWidget *progressWidget();

	QWidget *toolbarWidget();

	inline bool autoScroll() const { return m_autoScroll; }

	inline const Time & rightMousePressTime() const { return m_timeRMBPress; }
	inline const Time & rightMouseReleaseTime() const { return m_timeRMBRelease; }

	SubtitleLine * subtitleLineAtMousePosition() const;

	inline const Time & rightMouseSoonerTime() const {
		return m_timeRMBPress > m_timeRMBRelease ? m_timeRMBRelease : m_timeRMBPress;
	}
	inline const Time & rightMouseLaterTime() const {
		return m_timeRMBPress > m_timeRMBRelease ? m_timeRMBPress : m_timeRMBRelease;
	}

signals:
	void doubleClick(Time time);
	void middleMouseDown(Time time);
	void middleMouseMove(Time time);
	void middleMouseUp(Time time);
	void dragStart(SubtitleLine *line, DragPosition dragPosition);
	void dragEnd(SubtitleLine *line, DragPosition dragPosition);

public slots:
	void setSubtitle(Subtitle *subtitle = 0);
	void setAudioStream(const QString &mediaFile, int audioStream);
	void setNullAudioStream(quint64 msecVideoLength);
	void clearAudioStream();
	void zoomIn();
	void zoomOut();
	void setAutoscroll(bool autoscroll);
	void setScrollPosition(double milliseconds);
	void onSubtitleChanged();
	void setTranslationMode(bool enabled);
	void setShowTranslation(bool showTranslation);

protected:
	void leaveEvent(QEvent *event) override;
	bool eventFilter(QObject *obj, QEvent *event) override;
	void showContextMenu(QPoint pos);

private slots:
	void onPlayerPositionChanged(double seconds);
	void onScrollBarValueChanged(int value);
	void onHoverScrollTimeout();

private:
	QToolButton * createToolButton(const QString &actionName, int iconSize=16);
	void updateActions();
	void updateVisibleLines();
	Time timeAt(int y);
	WaveformWidget::DragPosition subtitleAt(int y, SubtitleLine **result);
	bool scrollToTime(const Time &time, bool scrollToPage);

	void updatePointerTime(int pos);
	bool mousePress(int pos, Qt::MouseButton button);
	bool mouseRelease(int pos, Qt::MouseButton button, QPoint globalPos);

private:
	QString m_mediaFile;
	int m_streamIndex;
	Subtitle *m_subtitle;

	Time m_timeStart;
	Time m_timeCurrent;
	Time m_timeEnd;

	Time m_timeRMBPress;
	Time m_timeRMBRelease;
	bool m_RMBDown;

	bool m_MMBDown;

	QScrollBar *m_scrollBar;
	QPropertyAnimation *m_scrollAnimation;
	bool m_autoScroll;
	bool m_autoScrollPause;
	double m_hoverScrollAmount;
	QTimer m_hoverScrollTimer;

	QWidget *m_toolbar;

	WaveRenderer *m_waveformGraphics;

	QWidget *m_progressWidget;
	QProgressBar *m_progressBar;

	QList<SubtitleLine *> m_visibleLines;
	bool m_visibleLinesDirty;

	SubtitleLine *m_draggedLine;
	DragPosition m_draggedPos;
	Time m_draggedTime;
	double m_draggedOffset;

	QBoxLayout *m_widgetLayout;

	Time m_pointerTime;

	QToolButton *m_btnZoomIn;
	QToolButton *m_btnZoomOut;
	QToolButton *m_btnAutoScroll;

	bool m_translationMode;
	bool m_showTranslation;

	friend class WaveBuffer;
	WaveBuffer *m_wfBuffer;

	WaveZoomData **m_zoomData;

	friend class WaveRenderer;
};
}
#endif
