/*
 * Copyright (C) 2015, Christian Benjamin Ries
 * Website: http://www.christianbenjaminries.de
 * License: MIT License, http://opensource.org/licenses/MIT
 */

#pragma once

#ifndef __SEMAINWINDOW_H__
#define __SEMAINWINDOW_H__

// Qt
#include <QIcon>
#include <QColor>
#include <QMovie>
#include <QPixmap>
#include <QMainWindow>

// SceneEditor
#include <SeScene.h>
#include <SeSceneLed.h>
#include <SeSceneItem.h>
#include <SeSceneView.h>
#include <SeScenePlayer.h>
#include <SeMosaicWindow.h>
#include <SeWebSocket.h>

namespace Ui {
class SeMainWindow;
}

/**
 * @brief The SeMainWindow class
 */
class SeMainWindow 
  : public QMainWindow
{
  Q_OBJECT    
public:
  explicit SeMainWindow(QWidget *parent = 0);
  ~SeMainWindow();
    
private:
  Ui::SeMainWindow *ui;
  SeMosaicWindow *mpMosaicWindow;
  
  SeScenePlayer *mpScenePlayer;
  SeSceneView *mpSceneView;
  SeScene *mpScene;
  SeSceneLayer *mpCurrentLayer;
  SeSceneLed *mpCurrentLed;
  
  // WebSocket stuff...
  SeWebSocket *mpWebSocket;    
  int numberOfStates_Which_Are_Ok;
  int numberOfStates_Received;
  #define NUMBER_OF_STATES_TO_REACH 10*20
  // loading animation for the WebSocket button
  QMovie *mpLoading;
    
  void initializeGui();
  void setChangeColor(QColor color);
  void setChangeImageFile(const QString & filename);
  
  void showProperties(SeSceneItem *ptr);
  
public:
  void applyPixmap(const QPixmap & pix);
  
  void loadConfiguration(const QString & filename="");
  void storeConfiguration(const QString & filename="");
  
private:
  //! Keeps the current project state, i.e. if it is already stored.
  //! Only stored projects can be exported as CSV and used for later 
  //! Arduino/Raspberry PI use.
  bool mFileStoredAsProject;
  
  QString mCfgFilename;
  QString mDeploymentDirname;
  QString mDeploymentFilename;

  bool closeProject();

  int initializeLayersForPlayer(QList<SeSceneLayer*> & layers);
  
public slots:
  void sceneLayerClicked(const QString & identifier);
  void sceneItemClicked(SeSceneItem *ptr);
  SeSceneLayer *createScene(const QString & identifier);  
  void removeScene(const QString & identifier);
  void duplicateScene(const QString & existingIdentifier, const QString & createdIdentifier);

private slots:
  void on_radioShowRectangle_clicked();
  void on_radioShowCircles_clicked();
  void on_spinDelay_valueChanged(double arg1);
  void on_cmdStartAnimation_clicked();
  void on_cmdStopAnimation_clicked();
  void on_cmdSelectImage_clicked();
  void on_cmdModifyImage_clicked();
  void on_cmdModifyReset_clicked();
  void on_actionLoad_triggered();
  void on_actionSave_triggered();
  void on_actionSave_As_triggered();
  void on_actionNew_triggered();
  void on_chkLayerEnabled_clicked(bool checked);
  void on_cmbChangeMode_currentIndexChanged(int index);  
  void on_cmdChangeColor_clicked();
  void on_cmbChangeMode_2_currentIndexChanged(int index);
  void on_cmdChangeColor_2_clicked();
  void on_actionAbout_Qt_triggered();
  void on_cmdUnselect_clicked();
  void on_cmdSelectAll_clicked();
  void on_actionExi_triggered();
  void on_tabWidget_currentChanged(int index);
  void on_cmdGenerateVideo_clicked();
  void on_cmdDeploy_clicked();
  void on_actionAbout_SceneEditor_triggered();
  void on_cmdDeployWebSocket_clicked();
};

#endif // __SEMAINWINDOW_H__
