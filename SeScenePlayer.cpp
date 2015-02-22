/*
 * Copyright (C) 2015, Christian Benjamin Ries
 * Website: http://www.christianbenjaminries.de
 * License: MIT License, http://opensource.org/licenses/MIT
 */

// SceneEditor
#include <SeGeneral.h>
#include <SeScenePlayer.h>
#include <SeSceneLayer.h>
#include <SeSceneLed.h>
#include <SeScene.h>

// Qt
#include <QCoreApplication>
#include <QDesktopServices>
#include <QApplication>
#include <QMessageBox>
#include <QTextStream>
#include <QStatusBar>
#include <QDateTime>
#include <QProcess> 
#include <QObject>
#include <QDebug>
#include <QList>
#include <QFile>
#include <QTime>
#include <QUrl>
#include <QDir>

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<class T> T minOf(T v[2]) { return (v[0] < v[1] ? v[0] : v[1]); }
template<class T> T maxOf(T v[2]) { return (v[0] > v[1] ? v[0] : v[1]); }

template<class T> T deltaOf(T v[2])
{
  T minV = (v[0] < v[1] ? v[0] : v[1]);
  T maxV = (v[0] > v[1] ? v[0] : v[1]);  
  return maxV - minV;
}

template<class T> T sign(T v[2])
{
  if(v[0] < v[1]) return 1.f;
  else            return -1.f;
}

SeScenePlayerTransitions::SeScenePlayerTransitions(
    QList<SeSceneLayer*> layers
  , SeScenePlayer *owner
) : mpOwner(owner)
{
  int n = layers.count();
  double td = mpOwner->mMsecDelay / 1000.f;

  SeSceneLayer *pbeforeLast = NULL;

  double T = 0.f;

  for(int index = 0; index < n; ++index)
  {
    SeSceneLayer *pcurrent = layers.at(index);
    SeSceneLayer *pnext = NULL;
    
    if(index + 1 >= n)
    {
      pnext = pcurrent;
    }
    else
    {
      pnext = layers.at(index + 1);    
    }
    
    int nextIndex = index + 1;
    while(pnext->properties().enabled() == false)
    {
      qDebug() << QString("Layer disabled: %1").arg(pnext->identifier());
    
      if(nextIndex + 1 >= n)
      {
        pnext = pcurrent;
      }
      else
      {
        nextIndex++;
        
        pnext = layers.at(nextIndex);
      }
    }
    
    double tlen = pcurrent->delay();
    double ttd = tlen / td;           // number of layer for pcurrent
    
    int upperEnd = static_cast<int>(ttd + 0.5f);
    
    for(int ml=0; ml < upperEnd; ml += 1)
    {
      this->offsets.append(T + ((double) ml * td));
    }      
      
    int numberOfColumns = pcurrent->mColumns;
    int numberOfRows = pcurrent->mRows;
      
    QList<SeSceneLayer*> __steppingLayers;
    for(int ml=0; ml < upperEnd; ml += 1)
    {
      SeSceneLayer *p = new SeSceneLayer(numberOfRows, numberOfColumns);
      p->initialize<SeSceneLed>();
      __steppingLayers.append(p);
    }
     
    for(int column=0; column < numberOfColumns; column++)
    {
      for(int row=0; row < numberOfRows; row++)
      { 
          SeSceneLed *ledCurrent = (SeSceneLed*) pcurrent->sceneItem(column, row);
          SeSceneLed *ledNext = (SeSceneLed*) pnext->sceneItem(column, row);
                    
          if(ledCurrent->properties().transitionMode() == SeSceneItemProperties::TransitionMode::Hard)
          {         
            for(int ml=0; ml < upperEnd; ml += 1)
            {
              SeSceneLayer *__stepLayer = __steppingLayers.at(ml);
              SeSceneLed *__led = (SeSceneLed*) __stepLayer->sceneItem(column, row);            
              __led->properties() = ledCurrent->properties();
            }
          
            continue;
          }
          
          SE_CONT4NULL(ledCurrent);
          SE_CONT4NULL(ledNext);          
        
          QColor cCurrent = ledCurrent->properties().brushColor();
          QColor cNext = ledNext->properties().brushColor();
        
          float red[2]   = { cCurrent.redF(),   cNext.redF()   };
          float green[2] = { cCurrent.greenF(), cNext.greenF() };
          float blue[2]  = { cCurrent.blueF(), cNext.blueF() };
          //float alpha[2] = { cCurrent.alphaF(), cNext.alphaF() };
        
          float d_red    = deltaOf<float>(red)   / (float) upperEnd;
          float d_green  = deltaOf<float>(green) / (float) upperEnd;
          float d_blue   = deltaOf<float>(blue)  / (float) upperEnd;
          //float d_alpha  = deltaOf<int>(alpha) / (float) upperEnd;
          
          float sign_red    = sign<float>(red);
          float sign_green  = sign<float>(green);
          float sign_blue   = sign<float>(blue);
          //float sign_alpha  = sign<float>(alpha);
          
          //qDebug() << " ---------------------------------------------------- ";
          
          for(int ml=0; ml < upperEnd; ml += 1)
          {
            float r =   red[0] + sign_red   * (d_red   * (float) ml);
            float g = green[0] + sign_green * (d_green * (float) ml);
            float b =  blue[0] + sign_blue  * (d_blue  * (float) ml);
            //float a = alpha[0] + sign_alpha * (d_alpha * (float) ml);
              
            QColor c;
            
            c.setRedF(r);
            c.setGreenF(g);
            c.setBlueF(b);
            
            //qDebug() << "C: " << c;
              
            SeSceneLayer *p = __steppingLayers.at(ml);
            SeSceneLed *__led = (SeSceneLed*) p->sceneItem(column, row);            
            __led->properties().setBrushColor(c);
            __led->properties().setPenColor(c);
                        
            QCoreApplication::processEvents();
          }        
          
          ;
      } // for(rows)        
    } // for(columns)
      
    for(int i=0; i < __steppingLayers.count(); i++)
    {
      this->ly.append(__steppingLayers.at(i));
    }
    
    T += tlen;
    
    pbeforeLast = pnext;
  }
  
#ifdef QT_DEBUG
  qDebug() << "Generated!";
  qDebug() << "  Layer: " << this->ly.count();
  qDebug() << "  Offsets: " << this->offsets.count();
#endif

  int numberOfLoaded = 0;
  int numberOfLayer = this->ly.count();

  // add them all to the view
  for(SeSceneLayer *p : this->ly)
  {
    p->hide();
    mpOwner->mpScene->addItem(p);
    
    numberOfLoaded++;
    
    float percentage = numberOfLoaded / (float) numberOfLayer * 100.f;
    
    QString m = QString("Loading... %1, %2 done...")
      .arg(percentage)
      .arg(p->identifier());
    
    SceneEditor::__statusBar->showMessage(m);
    
    QCoreApplication::processEvents();
  }
    
  ;;
}

SeScenePlayerTransitions::~SeScenePlayerTransitions()
{
  for(SeSceneLayer *p : ly)
  {
    delete p;
  }
  ly.clear();
}

bool SeScenePlayerTransitions::update()
{
  for(int i=0; i < this->ly.count(); i++)
  {
    SeSceneLayer *p = this->ly.at(i);
    
    if(p != NULL)
    {
      p->hide();
    }
  }
  
  int & runIndex = mpOwner->mCurrentLayerIndex;
  bool runLooped = mpOwner->mLoop;

  if(runIndex >= ly.count() && runLooped == true)
  {
    runIndex = 0;
  }
  else if(runIndex < ly.count() )
  {
    runIndex++;
  }
  else
  {  
    runIndex++;
  }
  
  if(runIndex >= ly.count() && runLooped == false)
  { 
    return false;
  }
  
  if(runIndex >= ly.count()) { runIndex = 0; }
  
  SceneEditor::__statusBar->showMessage(QString("Scene %1 of %2!").arg(runIndex).arg(ly.count()));
  
  SeSceneLayer *p = this->ly.at(runIndex);
  if(p != NULL)
  {
    p->show();
    p->update();
  }
  
  QCoreApplication::processEvents();
  
  return true;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

SeScenePlayer::SeScenePlayer(SeScene * playerEnvironment, QObject *parent) 
  : QObject(parent)
  , mpScene(playerEnvironment)
  , mCurrentLayerIndex(-1)
  , mMsecDelay(100)
  , mLoop(false)
  , mpTransitions(NULL)
  , mpProcess(NULL)
  , mIsVideoGenerating(false)
{
  QObject::connect(&mTimer, SIGNAL(timeout()), this, SLOT(update()));
  
  mpProcess = new QProcess();
  QObject::connect(mpProcess, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processError(QProcess::ProcessError)));
  QObject::connect(mpProcess, SIGNAL(started()), this, SLOT(processStarted()));
  QObject::connect(mpProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(processReadyStandardOutput()));
  QObject::connect(mpProcess, SIGNAL(readyReadStandardError()), this, SLOT(processReadyStandardError()));
  QObject::connect(mpProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(processFinished(int,QProcess::ExitStatus)));
  QObject::connect(mpProcess, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(processStateChanged(QProcess::ProcessState)));
}

SeScenePlayer::~SeScenePlayer()
{
}

bool SeScenePlayer::generateImages(const QString &directoryForImages)
{
  // remove any previously generated file within the target directory
  QDir dir(directoryForImages);
  
  if(dir.exists() == false)
  {
    dir.mkpath(directoryForImages);
  }
  
  dir.setNameFilters(QStringList() << "image-*.png");
  dir.setFilter(QDir::Files);
  for(int i=0; i < dir.entryList().size(); i++) {
    dir.remove(dir.entryList().at(i));
  }

  const QList<SeSceneLayer*> & layers = mpTransitions->layers();
  
  int numberOfLoaded = 0;
  int numberOfLayer = layers.count();

  int maxNumber = numberOfLayer * 2;
  
  auto exportFnc = [&](SeSceneLayer *p) 
  {
    QString exportName = QString("%1/image-%2.png")
                            .arg(directoryForImages)
                            .arg(numberOfLoaded, 6, 10, QLatin1Char('0'));
                            
    mpScene->exportLayer(p, exportName);
    
    numberOfLoaded++;
    
    float percentage = numberOfLoaded / (float) maxNumber * 100.f;
    
    QString m = QString("Image created %1%, %2 done...")
      .arg(percentage)
      .arg(p->identifier());
    
    SceneEditor::__statusBar->showMessage(m);
  };
  
  // forward
  for(int i=0; i < numberOfLayer; i++)
  {
    if(mAbortIsRequested == true) { break; }
  
    SeSceneLayer *p = layers.at(i);  
    exportFnc(p);
    QCoreApplication::processEvents();
  }
  
  // backward
  for(int i=numberOfLayer - 1; i != 0; --i)
  {
    if(mAbortIsRequested == true) { break; }

    SeSceneLayer *p = layers.at(i);  
    exportFnc(p);     
    QCoreApplication::processEvents();
  }

  if(mAbortIsRequested == true)
  {
    mAbortIsRequested = false;    
    return false;
  }

  return true;
}

bool SeScenePlayer::generateVideo(const QString &videoPath, const QString &directoryOfImages)
{
  if(mIsVideoGenerating)
  {
    qDebug() << "The video is already processing.";
  
    return false;
  }

  mIsVideoGenerating = true;

  if(mpTransitions == NULL)
  {
    mpTransitions = new SeScenePlayerTransitions(mLayers, this);

    this->generateImages();
  }

  QStringList lookUpDirectories;
  
#ifdef WIN32  
  QString ffmpegExe("ffmpeg.exe");
  lookUpDirectories << "ffmpeg.exe";
  lookUpDirectories << "ThirdParty/ffmpeg.exe";
  lookUpDirectories << "bin/ffmpeg.exe";
  lookUpDirectories << QString("%1/ffmpeg.exe").arg(QDir::homePath());
  lookUpDirectories << QString("%1/bin/ffmpeg.exe").arg(QDir::homePath());
  lookUpDirectories << QString("%1/Documents/ffmpeg.exe").arg(QDir::homePath());
  lookUpDirectories << QString("%1/Documents/bin/ffmpeg.exe").arg(QDir::homePath());
  lookUpDirectories << QString("%1/Documents/ThirdParty/ffmpeg.exe").arg(QDir::homePath());
  lookUpDirectories << QString("%1/ThirdParty/ffmpeg.exe").arg(QDir::homePath());
#else
  QString ffmpegExe("ffmpeg");
  lookUpDirectories << "ffmpeg";
  lookUpDirectories << "ThirdParty/ffmpeg";
  lookUpDirectories << "bin/ffmpeg";
  lookUpDirectories << "/usr/local/bin/ffmpeg";
  lookUpDirectories << "/usr/bin/ffmpeg";
  lookUpDirectories << "/bin/ffmpeg";
  lookUpDirectories << QString("%1/ffmpeg").arg(QDir::homePath());
  lookUpDirectories << QString("%1/bin/ffmpeg").arg(QDir::homePath());
  lookUpDirectories << QString("%1/ThirdParty/ffmpeg").arg(QDir::homePath());
#endif

  bool exist = false;

  for(int i=0; i < lookUpDirectories.count(); i++)
  {
    QString p = lookUpDirectories.at(i);
  
    QFile f(p);
    if(f.exists())
    {
      ffmpegExe = p;
      exist = true;
      break;
    }
  }
  
  if(exist == false)
  {
    QMessageBox::critical(NULL,
         tr("ffmpeg.exe is missing")
       , tr("The ffmpeg.exe is missing.\nPath: %1").arg(ffmpegExe)
      );
  }
  else
  {
    qDebug() << "Use ffmpeg version of path: " << ffmpegExe;
  
    QString path = directoryOfImages;
    if(path[path.length()-1] != '/') { path += "/"; }
    
    mVideoPath = videoPath;
    
    mpProcess->setWorkingDirectory(directoryOfImages);
    mpProcess->setArguments(QStringList() << "-y" << "-r" << "60" << "-i" << "image-%06d.png" << videoPath);
    mpProcess->setProgram(ffmpegExe);    
    mpProcess->start();
  }
  
  return true;
}

bool SeScenePlayer::abortVideoGeneration()
{
  mAbortIsRequested = true;

  if(mpProcess != NULL)
  {
    mpProcess->kill();
  }

  return true;
}

bool SeScenePlayer::prepareDeployment()
{
  if(mpTransitions == NULL)
  {
    mpTransitions = new SeScenePlayerTransitions(mLayers, this);
  }

  return true;
}

void SeScenePlayer::processError(QProcess::ProcessError error)
{
  QStringList errMsg;
  errMsg << "The process failed to start. Either the invoked program is missing, or you may have insufficient permissions to invoke the program.";
  errMsg << "The process crashed some time after starting successfully.";
  errMsg << "The last waitFor...() function timed out. The state of QProcess is unchanged, and you can try calling waitFor...() again.";
  errMsg << "An error occurred when attempting to write to the process. For example, the process may not be running, or it may have closed its input channel.";
  errMsg << "An error occurred when attempting to read from the process. For example, the process may not be running.";
  errMsg << "An unknown error occurred. This is the default return value of error().";

  qDebug() << "Process error: " << error;
  qDebug() << "Process error description: " << errMsg.at(error);
  
  qDebug() << "  > Program:   " << mpProcess->program();
  qDebug() << "  > Arguments: " << mpProcess->arguments().join(" ");  
  
  mIsVideoGenerating = false; 
}

void SeScenePlayer::processStarted()
{
  SceneEditor::__statusBar->showMessage(tr("Video creation started..."));
}

void SeScenePlayer::processReadyStandardOutput()
{
#ifdef QT_DEBUG
  QString output = mpProcess->readAllStandardOutput();  
  qDebug() << output;
#endif
}

void SeScenePlayer::processReadyStandardError()
{
#ifdef QT_DEBUG
  QString output = mpProcess->readAllStandardError();  
  qDebug() << output;
#endif
}

void SeScenePlayer::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  Q_UNUSED(exitCode);
  Q_UNUSED(exitStatus);

  SceneEditor::__statusBar->showMessage(tr("Video finished!")); 
  
  QFileInfo info(mVideoPath);
  QString videoDirname = info.dir().absolutePath();
  QUrl url = QUrl::fromLocalFile(videoDirname);
#ifdef QT_DEBUG
  qDebug() << "Target directory: " << url;
#endif
  QDesktopServices::openUrl(url);
  
  mIsVideoGenerating = false;
}

void SeScenePlayer::processStateChanged(QProcess::ProcessState state)
{
  switch(state)
  {
    case QProcess::NotRunning: qDebug() << tr("Not running..."); break;
    case QProcess::Starting: qDebug() << tr("Starting..."); break;
    case QProcess::Running: qDebug() << tr("Running..."); break;
    default:
      qDebug() << tr("Unknown state: %1").arg(state);
  }
}

bool SeScenePlayer::setMsecDelay(int msec)
{
  if(mTimer.isActive()) { return false; }  
  this->mMsecDelay = msec;  
  mTimer.setInterval(msec);
  return true;
}

void SeScenePlayer::reset()
{
  mTimer.stop();  
  mCurrentLayerIndex = 0;
  
  if(mpTransitions != NULL)
  {
    delete mpTransitions;
    mpTransitions = NULL;
  }
  
  this->mCurrentLayerIndex = 0;
}

bool SeScenePlayer::play()
{
  if(mCurrentLayerIndex < 0) { return false; }
  if(mLayers.count() <= 0) { return false; }

  if(mpTransitions == NULL)
  {
    mpTransitions = new SeScenePlayerTransitions(mLayers, this);
  }

  mTimer.setInterval(mMsecDelay);
  mTimer.start(); 
  
  emit started(); 

  return true;
}

bool SeScenePlayer::pause()
{  
  mTimer.stop();  
  emit paused();
  return true;
}

bool SeScenePlayer::stop()
{
  this->reset();
  emit stopped();
  return true;
}

void SeScenePlayer::update()
{
  if(mpTransitions != NULL)
  {
    bool res = mpTransitions->update();
    
    if(res == false)
    {
      mTimer.stop();
          
      emit endReached();
    }
    
    mpScene->update();
  }
  
  QCoreApplication::processEvents();
}
