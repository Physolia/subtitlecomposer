/*
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2019 Mladen Milinkovic <max@smoothware.net>
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

#include "videoplayer.h"
#include "videoplayer/backend/ffplayer.h"
#include "scconfig.h"

#include <math.h>

#include <QTimer>
#include <QFileInfo>
#include <QApplication>
#include <QEvent>

#include <QDebug>

#include <KLocalizedString>

#define VOLUME_MULTIPLIER 6.

using namespace SubtitleComposer;


VideoPlayer::VideoPlayer()
	: m_player(new FFPlayer()),
	  m_renderer(new GLRenderer()),
	  m_videoWidget(nullptr),
	  m_filePath(),
	  m_state(Initialized),
	  m_position(-1.0),
	  m_duration(-1.0),
	  m_fps(-1.0),
	  m_playSpeed(.0),
	  m_textStreams(),
	  m_activeAudioStream(-2),
	  m_audioStreams(),
	  m_muted(false),
	  m_volume(100.0)
{
	m_renderer->setOverlay(&m_subOverlay);
	m_player->init(m_renderer);

	setupNotifications();
}

VideoPlayer::~VideoPlayer()
{
	cleanup();
	delete m_player;
}

VideoPlayer *
VideoPlayer::instance()
{
	static VideoPlayer *player = nullptr;
	if(!player) {
		player = new VideoPlayer();
		player->setParent(QApplication::instance());
	}
	return player;
}

bool
VideoPlayer::init(QWidget *videoContainer)
{
	if(!m_videoWidget) {
		m_videoWidget = new VideoWidget(videoContainer);
		m_videoWidget->setVideoLayer(m_renderer);

		connect(m_videoWidget, &VideoWidget::doubleClicked, this, &VideoPlayer::doubleClicked);
		connect(m_videoWidget, &VideoWidget::rightClicked, this, &VideoPlayer::rightClicked);
		connect(m_videoWidget, &VideoWidget::leftClicked, this, &VideoPlayer::leftClicked);
		connect(m_videoWidget, &VideoWidget::wheelUp, this, &VideoPlayer::wheelUp);
		connect(m_videoWidget, &VideoWidget::wheelDown, this, &VideoPlayer::wheelDown);
	} else {
		m_videoWidget->setParent(videoContainer);
	}
	reset();

	return true;
}

void
VideoPlayer::cleanup()
{
	m_player->cleanup();
}

void
VideoPlayer::reset()
{
	m_filePath.clear();

	m_position = -1.0;
	m_duration = -1.0;
	m_fps = -1.0;

	m_activeAudioStream = -2;
	m_textStreams.clear();
	m_audioStreams.clear();

	m_state = Initialized;

	if(m_videoWidget)
		m_videoWidget->videoLayer()->hide();
}

bool
VideoPlayer::playOnLoad()
{
	const QWidget *topLevel = m_videoWidget->topLevelWidget();
	const QWidget *dockWaveform = topLevel->findChild<QWidget *>(QStringLiteral("waveform_dock"));
	const QWidget *dockVideo = topLevel->findChild<QWidget *>(QStringLiteral("player_dock"));
	return SCConfig::videoAutoPlay() && (dockVideo->isVisible() || dockWaveform->isVisible());
}

bool
VideoPlayer::openFile(const QString &filePath)
{
	Q_ASSERT(m_state == Initialized || m_state == Stopped);

	QFileInfo fileInfo(filePath);
	if(!fileInfo.exists() || !fileInfo.isFile() || !fileInfo.isReadable()) {
		emit fileOpenError(filePath, i18n("File does not exist."));   // operation will never succeed
		return false;
	}

	m_filePath = filePath;
	m_state = Opening;

	if(!m_player->open(fileInfo.absoluteFilePath().toUtf8()))
		return false;

	if(m_player->paused() == playOnLoad())
		m_player->pauseToggle();

	return true;
}

void
VideoPlayer::closeFile()
{
	if(m_state < Opening)
		return;
	m_player->close();
	reset();
	emit fileClosed();
}

void
VideoPlayer::play()
{
	if(m_state <= Opening || m_state == Playing)
		return;
	if(m_state < Playing)
		openFile(m_filePath);
	if(m_state == Paused)
		m_player->pauseToggle();
}

void
VideoPlayer::pause()
{
	if(m_state <= Opening || m_state == Paused)
		return;
	m_player->pauseToggle();
}

void
VideoPlayer::togglePlayPaused()
{
	if(m_state == Playing)
		pause();
	else if(m_state > Opening)
		play();
}

bool
VideoPlayer::seek(double seconds)
{
	if(m_state <= Stopped)
		return false;
	m_player->seek(qBound(0., seconds, m_duration));
	return true;
}

bool
VideoPlayer::step(int frameOffset)
{
	if(m_state <= Stopped)
		return false;
	m_player->stepFrame(frameOffset);
	return true;
}

bool
VideoPlayer::stop()
{
	if(m_state <= Stopped)
		return false;
	m_player->close();
	return true;
}

bool
VideoPlayer::selectAudioStream(int streamIndex)
{
	if(m_state <= VideoPlayer::Opening || streamIndex < 0 || streamIndex >= m_audioStreams.size())
		return false;

	if(m_activeAudioStream != streamIndex) {
		m_activeAudioStream = streamIndex;
		m_player->activeAudioStream(streamIndex);
		emit activeAudioStreamChanged(streamIndex);
	}
	return true;
}

void
VideoPlayer::playSpeed(double newRate)
{
	if(m_state <= Opening || newRate < .05 || newRate > 8.)
		return;

	m_player->setSpeed(newRate);
}

void
VideoPlayer::increaseVolume(double amount)
{
	setVolume(m_volume + amount);
	setMuted(false);
}

void
VideoPlayer::decreaseVolume(double amount)
{
	setVolume(m_volume - amount);
}

void
VideoPlayer::setVolume(double volume)
{
	volume = qBound(0., volume, 100.);
	m_player->setVolume(VOLUME_MULTIPLIER * volume / 100.);

	if(m_volume != volume)
		emit volumeChanged(m_volume = volume);
}

void
VideoPlayer::setMuted(bool muted)
{
	m_player->setMuted(muted);

	if(m_muted == muted)
		return;

	m_muted = muted;

	emit muteChanged(m_muted);
}

void
VideoPlayer::setupNotifications()
{
	connect(m_player, &FFPlayer::mediaLoaded, this, [this](){
		emit fileOpened(m_filePath);
		m_fps = m_player->videoFPS();
		emit fpsChanged(m_fps);
		m_videoWidget->videoLayer()->show();
	});
	connect(m_player, &FFPlayer::stateChanged, this, [this](FFPlayer::State ffs){
		static const QMap<FFPlayer::State, State> stateMap
			{{ FFPlayer::Stopped, Stopped }, { FFPlayer::Playing, Playing }, { FFPlayer::Paused, Paused }};
		const State state = stateMap[ffs];
		if(m_state != state) switch(m_state = state) {
		case Stopped: emit stopped(); break;
		case Playing: emit playing(); break;
		case Paused: emit paused(); break;
		default: break; // not possible
		}
	});
	connect(m_renderer, &GLRenderer::resolutionChanged, this, [this](){
		m_videoWidget->setVideoResolution(m_player->videoWidth(), m_player->videoHeight(), m_player->videoSAR());
		m_videoWidget->videoLayer()->show();
	});

	connect(m_player, &FFPlayer::positionChanged, this, [this](double pos){
		if(m_position != (pos = qBound(0., pos, m_duration)))
			emit positionChanged(m_position = pos);
	});
	connect(m_player, &FFPlayer::durationChanged, this, [this](double dur){
		if(m_duration != dur) emit durationChanged(m_duration = dur);
	});
	connect(m_player, &FFPlayer::speedChanged, this, [this](double speed){
		if(m_playSpeed != speed) emit playSpeedChanged(m_playSpeed = speed);
	});

	connect(m_player, &FFPlayer::volumeChanged, this, [this](double volume){
		if(m_volume != (volume = volume * 100. / VOLUME_MULTIPLIER)) {
			m_volume = volume;
			if(!m_muted) emit volumeChanged(m_volume);
		}
	});
	connect(m_player, &FFPlayer::muteChanged, this, [this](bool muted){
		if(m_muted != muted) emit muteChanged(m_muted = muted);
	});

	//connect(m_player, &FFPlayer::videoStreamsChanged, this, [this](const QStringList &streams){});
	connect(m_player, &FFPlayer::audioStreamsChanged, this, [this](const QStringList &streams){
		emit audioStreamsChanged(m_audioStreams = streams);
		emit activeAudioStreamChanged(m_activeAudioStream = m_player->activeAudioStream());
	});
	connect(m_player, &FFPlayer::subtitleStreamsChanged, this, [this](const QStringList &streams){
		emit textStreamsChanged(m_textStreams = streams);
	});
}
