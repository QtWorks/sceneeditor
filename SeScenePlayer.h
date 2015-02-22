/*
 * Copyright (C) 2015, Christian Benjamin Ries
 * Website: http://www.christianbenjaminries.de
 * License: MIT License, http://opensource.org/licenses/MIT
 */

#ifndef __SESCENEPLAYER_H__
#define __SESCENEPLAYER_H__

#pragma once

// SceneEditor

// forward-declaration
class SeScene;
class SeSceneItem;
class SeSceneLayer;
class SeSceneLed;
class SeScenePlayer;

// Qt
#include <QProcess>
#include <QObject>
#include <QTimer>
#include <QTime>
#include <QList>

#ifdef WIN32
  #define DEFAULT_EXPORT_DIRECTORE "C:/temp/exports"
#else
  #define DEFAULT_EXPORT_DIRECTORE "~/exports"
#endif

/**
 * @brief The SeScenePlayerTransitions class
 */
class SeScenePlayerTransitions
{
public:
  SeScenePlayerTransitions(QList<SeSceneLayer*> layers, SeScenePlayer *owner);
  ~SeScenePlayerTransitions();
  
  //! \brief ...
  //! \return False if the end of all layers is reached and looping
  //!         has been deactivated. On correct visualization update
  //!         True will be returned.
  bool update();

  const QList<SeSceneLayer*> & layers() const { return ly; }

private:
  QList<double> offsets;
  QList<SeSceneLayer*> ly;
  SeScenePlayer *mpOwner;
};

/**
 * @brief The SeScenePlayer class
 */
class SeScenePlayer 
  : public QObject
{
  Q_OBJECT
public:
  explicit SeScenePlayer(SeScene *playerEnvironment, QObject *parent = 0);
  ~SeScenePlayer();
  
  friend class SeScenePlayerTransitions;
  
  bool setMsecDelay(int msec);
  void setLoop(bool state) { this->mLoop = state; }
    
  void reset();
  bool play();
  bool pause();
  bool stop();  
  
  void setLayers(QList<SeSceneLayer*> layers) { mLayers = layers; }
  
  bool generateImages(const QString & directoryForImages=DEFAULT_EXPORT_DIRECTORE);
  bool generateVideo(const QString & videoPath, const QString & directoryOfImages=DEFAULT_EXPORT_DIRECTORE);

  bool abortVideoGeneration();
  
  bool prepareDeployment();
  
private:
  int mCurrentLayerIndex;
  QList<SeSceneLayer*> mLayers; 
  QTimer mTimer;
  bool mLoop;
  
  QProcess *mpProcess;
  QString mVideoPath;

  bool mIsVideoGenerating;
  bool mAbortIsRequested;

public:
  bool isVideoGenerating() const { return mIsVideoGenerating; }

private slots:
  void processError(QProcess::ProcessError error);
  void processStarted();
  void processReadyStandardOutput();
  void processReadyStandardError();
  void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
  void processStateChanged(QProcess::ProcessState state);
  
private:
  //! The scene which will show the playing scene.
  SeScene *mpScene;
  
  //! the delay between visualization updates
  int mMsecDelay;

  SeScenePlayerTransitions *mpTransitions;
  
public:
  SeScenePlayerTransitions *transitions() const { return mpTransitions; }
  
signals:
  void started();
  void paused();
  void stopped();
  void endReached();
  
private slots:
  void update();
};



#endif // __SESCENEPLAYER_H__

