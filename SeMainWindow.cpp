/*
 * Copyright (C) 2015, Christian Benjamin Ries
 * Website: http://www.christianbenjaminries.de
 * License: MIT License, http://opensource.org/licenses/MIT
 */

// Qt
#include <QFile>
#include <QImage>
#include <QDebug>
#include <QLayout>
#include <QFileInfo>
#include <QSettings>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QColorDialog>
#include <QTreeWidgetItemIterator>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

// SceneEditor
#include <SeGeneral.h>
#include <SeScene.h>
#include <SeSceneLed.h>
#include <SeSceneView.h>
#include <SeSceneLayer.h>
#include <SeTreeScenes.h>
#include <SeMainWindow.h>
#include <ui_SeMainWindow.h>

namespace SceneEditor 
{
  QStatusBar *__statusBar;
}

SeMainWindow::SeMainWindow(QWidget *parent) 
  : QMainWindow(parent)
  , ui(new Ui::SeMainWindow)
  , mpMosaicWindow(NULL)
  , mpScenePlayer(NULL)
  , mpSceneView(NULL)
  , mpScene(NULL)
  , mpCurrentLed(NULL)
{
  ui->setupUi(this);
  
  SceneEditor::__statusBar = this->statusBar();
  
  this->initializeGui();
  
  if(ui->subwindow != NULL)
  {
    ui->subwindow->showMaximized();
  }
    
  mpMosaicWindow = new SeMosaicWindow(this);
  
  QObject::connect(ui->treeScenes, SIGNAL(sceneLayerClicked(QString)), this, SLOT(sceneLayerClicked(QString)));
  QObject::connect(ui->treeScenes, SIGNAL(createScene(QString)), this, SLOT(createScene(QString)));
  QObject::connect(ui->treeScenes, SIGNAL(removeScene(QString)), this, SLOT(removeScene(QString)));
  QObject::connect(ui->treeScenes, SIGNAL(duplicateScene(QString,QString)), this, SLOT(duplicateScene(QString,QString)));
  
  mpSceneView = dynamic_cast<SeSceneView*>(ui->gview);
  if(mpSceneView != NULL)
  {
    mpScene = mpSceneView->seScene();
    
    QObject::connect(mpScene, SIGNAL(item(SeSceneItem*)), this, SLOT(sceneItemClicked(SeSceneItem*)));
  }
  
  mpScenePlayer = new SeScenePlayer(mpScene);
  QObject::connect(mpScenePlayer, &SeScenePlayer::started, [&](){
    ui->cmdStartAnimation->setEnabled(false);
    ui->cmdGenerateVideo->setEnabled(false);
    ui->cmdDeploy->setEnabled(false);
    ui->cmdStopAnimation->setEnabled(true);
  });
  QObject::connect(mpScenePlayer, &SeScenePlayer::paused, [&](){
    ui->cmdStartAnimation->setEnabled(true);   
    ui->cmdGenerateVideo->setEnabled(false);
    ui->cmdDeploy->setEnabled(false);
  });
  QObject::connect(mpScenePlayer, &SeScenePlayer::stopped, [&](){
    ui->cmdStopAnimation->setEnabled(false);
    ui->cmdGenerateVideo->setEnabled(true);
    ui->cmdDeploy->setEnabled(true);
    ui->cmdStartAnimation->setEnabled(true);
  });
  QObject::connect(mpScenePlayer, &SeScenePlayer::endReached, [&](){
    QMessageBox::information(this, tr("Playback finished!"), tr("The visualization reached it's end."));
    ui->cmdStopAnimation->setEnabled(false);
    ui->cmdGenerateVideo->setEnabled(true);
    ui->cmdStartAnimation->setEnabled(true);
    ui->cmdDeploy->setEnabled(true);
  });
}

SeMainWindow::~SeMainWindow()
{
  delete ui;
  
  if(mpMosaicWindow != NULL) { delete mpMosaicWindow; mpMosaicWindow = NULL; }
  if(mpScenePlayer != NULL) { delete mpScenePlayer; mpScenePlayer = NULL; }
}

SeSceneLayer *SeMainWindow::createScene(const QString &identifier)
{
  if(mpScene == NULL) { return NULL; }

  ui->actionNew->setEnabled(true);

  SeSceneLayer *p = mpScene->addLayer(DEFAULT_ROWS, DEFAULT_COLUMNS);
  p->setIdentifier(identifier);
  return p;
}

void SeMainWindow::removeScene(const QString &identifier)
{
  if(mpScene == NULL) { return; }

  SeSceneLayer *p = mpScene->layer(identifier);
  if(p != NULL)
  {
    delete p; 
    p = NULL;
  }
  
  mpCurrentLayer = NULL;
  mpCurrentLed = NULL;
  
  ui->groupBoxProperties->setEnabled(false);
  ui->groupBoxMosaic->setEnabled(false);
  ui->groupBoxMultiselection->setEnabled(false);
}

void SeMainWindow::duplicateScene(const QString &existingIdentifier, const QString &createdIdentifier)
{
  if(mpScene == NULL) { return; }
  
  SeSceneLayer *srcLayer = mpScene->layer(existingIdentifier);
  SeSceneLayer *targetLayer = this->createScene(createdIdentifier);
  
  if(targetLayer != NULL)
  {
    mpScene->addItem(targetLayer);
  }
    
  if(srcLayer == NULL || targetLayer == NULL)
  {
    QMessageBox::critical(
        this
      , tr("Missing layer!")
      , tr("One of the required layer is missing: %1, %2")
          .arg(existingIdentifier)
          .arg(createdIdentifier)
     );
      
    return;
  }

  targetLayer->deepCopy(srcLayer);  
}

void SeMainWindow::initializeGui()
{
  ui->actionNew->setEnabled(false);
  ui->groupBoxProperties->setEnabled(false);
  ui->groupBoxMosaic->setEnabled(false);
  ui->groupBoxMultiselection->setEnabled(false);
  this->setChangeColor(QColor(255, 255, 255));
}

void SeMainWindow::setChangeColor(QColor c)
{
  QString css = QString("border: 1px solid black; background-color: rgba(%1,%2,%3,%4);")
    .arg(c.red())
    .arg(c.green())
    .arg(c.blue())
    .arg(c.alpha());
    
  ui->cmdChangeColor->setStyleSheet(css);
  ui->cmdChangeColor_2->setStyleSheet(css);
}

void SeMainWindow::setChangeImageFile(const QString &imageFilename)
{
  if(imageFilename.isEmpty() == true)
  {    
    ui->cmdModifyReset->setEnabled(false);
    ui->cmdModifyImage->setEnabled(false);
  }
  else
  {  
    QPixmap p(imageFilename);
    if(p.isNull() == false)
    {
      mpCurrentLayer->properties().setOriginalFilePath(imageFilename);
      mpCurrentLayer->properties().setOriginalPixmap(p);
    
      QString imgInfo = QString("%1x%2")
        .arg(p.width())
        .arg(p.height());
        
      ui->lblImageInfo->setText(imgInfo);
    }
    
    ui->cmdModifyReset->setEnabled(true);
    ui->cmdModifyImage->setEnabled(true);
  }
}

void SeMainWindow::showProperties(SeSceneItem *ptr)
{
  SeSceneLed *p = dynamic_cast<SeSceneLed*>(ptr);

  if(ptr == NULL || p == NULL)
  {
    ui->groupBoxProperties->setEnabled(false);
    ui->groupBoxMultiselection->setEnabled(false);
    ui->groupBoxMosaic->setEnabled(false);
    ui->linePropertyColumn->setText("");
    ui->linePropertyRow->setText("");
    ui->cmbChangeMode->setCurrentIndex(0);
    this->setChangeColor(QColor(255, 255, 255));
  }
  else
  {
    ui->groupBoxProperties->setEnabled(true);
    ui->groupBoxMosaic->setEnabled(true);
    ui->groupBoxMultiselection->setEnabled(true);
    ui->linePropertyColumn->setText(QString("%1").arg(p->properties().column()));
    ui->linePropertyRow->setText(QString("%1").arg(p->properties().row()));
    ui->cmbChangeMode->setCurrentIndex((int) p->properties().transitionMode());
    this->setChangeColor(p->properties().brushColor());
  }  
}

void SeMainWindow::applyPixmap(const QPixmap &pix)
{
  if(mpCurrentLayer == NULL)
  {
    QMessageBox::critical(
        this
      , tr("No layer selected!")
      , tr("Please select a frame for apply image.")
    );
    
    return;
  }

  if(pix.isNull())
  {
    QMessageBox::critical(
        this
      , tr("Pixmap is Null!")
      , tr("Your selected pixmap frame is not valid."));
      
    return;
  }
  else
  {
    QImage img = pix.toImage();
    
    int w = mpCurrentLayer->numberOfColumns();
    int h = mpCurrentLayer->numberOfRows();
  
    for(int ix=0; ix < w; ix++)
    {
      for(int iy=0; iy < h; iy++)
      {
        QPoint p(ix * SeMosaicWindow::pixelSteps(), iy * SeMosaicWindow::pixelSteps());
        
        QRgb rgb = img.pixel(p);
        
        mpCurrentLayer->sceneItem(ix, iy)->properties().setBrushColor(rgb);
      }
    }
    
    mpCurrentLayer->update();  
  }
}

void SeMainWindow::loadConfiguration(const QString &filename)
{
  mCfgFilename = filename;
  
  QString fileContent;
  
  QFile f(filename);
  if(f.open(QIODevice::ReadOnly))
  {
    fileContent = f.readAll();
    f.close();
  }
  
  mFileStoredAsProject = true;
    
  if(fileContent.isEmpty())
  {
    QMessageBox::critical(this, tr("Empty file!"), tr("The file is empty."));
  }
  else
  {
    QJsonDocument doc = QJsonDocument::fromJson(fileContent.toLocal8Bit());
    QJsonArray ar = doc.array();
    
    SeTreeSceneItem *firstTreeItem = NULL;
    
    for(int i=0; i < ar.count(); i++)
    {
      QJsonObject o = ar[i].toObject();
      
      QString identifier = o["Properties"].toObject()["Identifier"].toString();
      
      int percentage = i / (float) ar.count() * 100.f;
      if(i + 1 >= ar.count()) { percentage = 100; }
      
      SceneEditor::__statusBar->showMessage(QString("Loading scene: %1, %2% done!")
                      .arg(identifier)
                      .arg(percentage));
      
      SeSceneLayer *p = this->createScene(identifier);
      p->properties().restore(o["Properties"].toObject());
      p->loadJson(o);      
      
      SeTreeSceneItem *pp = ui->treeScenes->addScene(identifier, false);
      if(firstTreeItem == NULL)
      {
        firstTreeItem = pp;
      }
    }
    
    ui->treeScenes->setCurrentItem(firstTreeItem);
    emit ui->treeScenes->sceneLayerClicked(
      firstTreeItem->data(0, SeTreeSceneItem::Roles::Uuid).toString()
    );
  }
  
  SceneEditor::__statusBar->clearMessage();
  
  mpScene->update();
}

void SeMainWindow::storeConfiguration(const QString &filename)
{
  if(filename.isEmpty() && mCfgFilename.isEmpty())
  {
    QString fname = QFileDialog::getSaveFileName(this, tr("Target filename..."));
    if(fname.isEmpty()) { return; }
    
    mCfgFilename = fname;
  }
  
  SceneEditor::__statusBar->showMessage(tr("Storing scene: %1").arg(filename));
  
  QJsonArray ar;
  
  QStringList ids = ui->treeScenes->identifiers();
  for(QString id : ids)
  {
    SeSceneLayer *layer = mpScene->layer(id);
    SE_CONT4NULL(layer);
    
    SceneEditor::__statusBar->showMessage(tr("Query data of scene: %1").arg(id));
    
    ar.append(layer->toJson());
  }
  
  QJsonDocument doc;
  doc.setArray(ar);
  
  QFile outputFile(mCfgFilename);
  if(outputFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
  {
    outputFile.write(doc.toJson().simplified());
    outputFile.close();
    
    mFileStoredAsProject = true;
  }
  
  SceneEditor::__statusBar->clearMessage();
}

void SeMainWindow::sceneLayerClicked(const QString &identifier)
{
  SceneEditor::__statusBar->clearMessage();

  bool res = mpScene->showLayer(identifier);
  if(res == false)
  {
    mpCurrentLayer = NULL;
  
    ui->spinDelay->setValue(1.0f);
    ui->groupBoxMosaic->setEnabled(false);
    ui->chkLayerEnabled->setChecked(false);
  }
  else
  {
    mpCurrentLayer = mpScene->layer(identifier);
  
    this->setChangeImageFile(mpCurrentLayer->properties().originalFilePath());
  
    ui->spinDelay->setValue(mpCurrentLayer->delay());
    ui->chkLayerEnabled->setChecked(mpCurrentLayer->properties().enabled());
    ui->groupBoxMosaic->setEnabled(true);
  }

  SeTreeScenes *p = ui->treeScenes;
  if(p != NULL)
  {
    bool r = p->topLevelItem(0) != NULL;
    
    ui->cmdGenerateVideo->setEnabled(r);
    ui->cmdStartAnimation->setEnabled(r);
    ui->cmdDeploy->setEnabled(r);
  }
}

void SeMainWindow::sceneItemClicked(SeSceneItem *ptr)
{
  SeSceneLed *p = dynamic_cast<SeSceneLed*>(ptr);
  if(p == NULL)
  {
    mpCurrentLed = NULL;
  }
  else
  {
    mpCurrentLed = p;
  }
  
  this->showProperties(p);
  
  SceneEditor::__statusBar->showMessage(tr("LED selected: %1").arg(p->identifier()));
  
  ui->tabWidget->setCurrentIndex(1);
}

void SeMainWindow::on_cmdChangeColor_clicked()
{
  QColor currentColor = mpCurrentLed->properties().brushColor();

  if(currentColor.isValid() == false) 
  {
    currentColor = QColor(255, 255, 255);
  }

  QColorDialog dlg;
  dlg.setCurrentColor(currentColor);
  int res = dlg.exec();
  
  if(res == 0) { return; }
  
  QColor c = dlg.selectedColor();
  
  if(c.isValid() == false) { return; }
  
  this->setChangeColor(c);
  
  if(mpCurrentLed != NULL)
  {
    mpCurrentLed->properties().setBrushColor(c);
    mpCurrentLed->properties().setPenColor(c);
  }
}

void SeMainWindow::on_radioShowRectangle_clicked()
{
  if(ui->radioShowRectangle->isChecked())
  {
    QList<SeSceneLayer*> layers;
    /*int n =*/ mpScene->sceneLayers(layers);
    for(auto p : layers)
    {
      p->changeShapeMode(SeSceneItemProperties::ShapeMode::ShapeRect);
    }
    
    mpScene->update();
  }
}

void SeMainWindow::on_radioShowCircles_clicked()
{
  if(ui->radioShowCircles->isChecked())
  {
    QList<SeSceneLayer*> layers;
    /*int n =*/ mpScene->sceneLayers(layers);
    for(auto p : layers)
    {
      p->changeShapeMode(SeSceneItemProperties::ShapeMode::ShapeCircle);
    }
    
    mpScene->update();
  }    
}

void SeMainWindow::on_cmdSelectImage_clicked()
{
  if(mpCurrentLayer == NULL) { return; }
  
  QFileInfo info(mpCurrentLayer->properties().originalFilePath());
  
  QString imageFilename = QFileDialog::getOpenFileName(
                              this
                            , tr("Select image for Mosaic style...")
                            , info.absoluteDir().absolutePath()
                           );
                          
  this->setChangeImageFile(imageFilename);                           
}

void SeMainWindow::on_spinDelay_valueChanged(double value)
{
  if(mpCurrentLayer == NULL) { return; }
  mpCurrentLayer->setDelay(value);
}

void SeMainWindow::on_cmdStartAnimation_clicked()
{
  QList<SeSceneLayer*> layers;
  
  int n = this->initializeLayersForPlayer(layers);
  if(n < 0) { return; }
  
  mpScenePlayer->reset();
  mpScenePlayer->setLayers(layers);
  mpScenePlayer->setLoop(ui->chkLoop->isChecked());  
  mpScenePlayer->play();
}

void SeMainWindow::on_cmdGenerateVideo_clicked()
{
  if(mpScenePlayer->isVideoGenerating())
  {
    int res = QMessageBox::question(
        this
      , tr("Video is processing!")
      , tr("There is alreay a video in progress.\nDo you like to cancel the process?")
    );
    
    if(res == QMessageBox::Yes)
    {
      mpScenePlayer->abortVideoGeneration();      
    }
    
    return;
  }

  QList<SeSceneLayer*> layers;
  
  int n = this->initializeLayersForPlayer(layers);
  if(n < 0) { return; }

  QString videoTarget = QDir::homePath();
  if(videoTarget[videoTarget.length()-1] != '/') { videoTarget += "/"; }

  videoTarget += "Exports/";
    
  QDir dir(videoTarget);
  if(dir.exists() == false)
  {
    dir.mkpath(videoTarget);
  }
    
  QDateTime dt = QDateTime::currentDateTime();
    
  videoTarget += QString("%1-Scene.mp4").arg(dt.toString("dd-MM-yyyy hh-mm"));

  mpScenePlayer->reset();
  mpScenePlayer->setLayers(layers);
  mpScenePlayer->generateVideo(videoTarget);
}

void SeMainWindow::on_cmdDeploy_clicked()
{
  QList<SeSceneLayer*> layers;
  
  int n = this->initializeLayersForPlayer(layers);
  if(n < 0) { return; }
  
  mpScenePlayer->reset();
  mpScenePlayer->setLayers(layers);
  bool res = mpScenePlayer->prepareDeployment();  
  
  if(res == true)
  {
    const QList<SeSceneLayer*> & layers = mpScenePlayer->transitions()->layers();
    
    if(layers.count() < 0)
    {
      QMessageBox::critical(
            this
          , tr("Transition set is empty")
          , tr("There are not transitions available.")
        );
        
      return;
    }
    
    qDebug() << "Create Avr/Arduino configuration!";
    
    QFileInfo info(mCfgFilename);
    mDeploymentDirname = info.absoluteDir().absolutePath();
    mDeploymentFilename = QString("%1/%2.csv")
      .arg(mDeploymentDirname)
      .arg(info.baseName());

    if(mDeploymentFilename.isEmpty())
    {
      // Should not be empty!?
    }
    else
    {
      QFile fcsv(mDeploymentFilename);
      if(fcsv.open(QIODevice::WriteOnly | QIODevice::Truncate))
      {
        QTextStream s(&fcsv);

        for(SeSceneLayer *player : layers)
        {      
          SE_CONT4NULL(player);
          
          s << player->toAvrCsv();
          #ifdef WIN32
            s << "\r\n";
          #else
            s << "\n";
          #endif
        }
      
        fcsv.close();
      }
            
      QStringList apps;
      int r = -1;
      
      #ifdef WIN32
        apps << "notepad++.exe";
        apps << "C:/Program Files (x86)/Notepad++/notepad++.exe";
        apps << "wordpad.exe";
        apps << "notepad.exe";
      #else
        apps << "gedit";
        apps << "kedit";        
        apps << "kate"; 
        apps << "gvim";
      #endif
      
      for(QString appname : apps)
      {
        r = QProcess::execute(appname, QStringList() << mDeploymentFilename);
        if(r >= 0) { break; }
      }
      
      if(r < 0)
      {
        QMessageBox::critical(
              this
            , tr("Missing editor!")
            , tr("Can not open result file '%1'.\nPlease install one of the following applications:\n%2")
                .arg(mDeploymentFilename)
                .arg(apps.join("\n")) 
          );
      }
    }
  }
}

int SeMainWindow::initializeLayersForPlayer(QList<SeSceneLayer*> & layers)
{
  if(mpScenePlayer == NULL) { return -1; }
  if(mpScene == NULL) { return -1; }
  
  int n = mpScene->sceneLayers(layers);
  if(n <= 0)
  {
    QMessageBox::critical(
        this
      , tr("No layer!")
      , tr("No layer available for playing."));      
      
    return -1;
  }
  
  QStringList ids = ui->treeScenes->identifiers();
  
  layers.clear();
  
  for(QString id : ids)
  {
    layers.append( mpScene->layer(id) );
  }

  // following for-loop just apply the ordering index to the properties
  // in the moment this is not really neccessarry, but maybe for some future cases
  for(SeSceneLayer *ptrLayer : layers)
  {
    SE_CONT4NULL(ptrLayer);
    
    QString id = ptrLayer->identifier();      
    int index = ui->treeScenes->indexOf(id);
    if(index < 0)
    {
      QMessageBox::critical(this, tr("Missing Scene"), tr("Scene is missing: %1").arg(id));        
    }
    else
    {
      ptrLayer->properties().setIndex(index);
      #ifdef QT_DEBUG
        qDebug() << QString("Id %1 on pos: %2").arg(id).arg(index);
      #endif
    }
  }
  
  return layers.count();
}

void SeMainWindow::on_cmdStopAnimation_clicked()
{
  if(mpScenePlayer == NULL) { return; }
  mpScenePlayer->stop();
}

void SeMainWindow::on_cmbChangeMode_currentIndexChanged(int index)
{
  if(mpCurrentLed == NULL) { return; }
  mpCurrentLed->properties().setTransitionMode((SeSceneItemProperties::TransitionMode)index);
}

void SeMainWindow::on_cmdModifyImage_clicked()
{
  if(mpMosaicWindow == NULL) { return; }
  
  mpMosaicWindow->setImage(mpCurrentLayer->properties().originalFilePath(), true);
  mpMosaicWindow->setModal(true);
  
  if(mpCurrentLayer != NULL)
  {
    // Scale
    int scaleValue = mpCurrentLayer->properties().scale();
    mpMosaicWindow->setScaleValue(scaleValue);
    
    // Offset
    QPoint offset = mpCurrentLayer->properties().offset();        
    mpMosaicWindow->setOffset(offset); 
    
    // Selection Frame    
    QRect r = mpCurrentLayer->properties().selectionGeometry();
    if(r.isEmpty() || r.isEmpty() || r.isNull())
    {
      // ...
    }
    else
    {
      mpMosaicWindow->setSelectionGeometry(r);
    }
  }
  
  mpMosaicWindow->original2preview();
  
  int res = mpMosaicWindow->exec();
  
  if(res == 1)
  {
    QPoint offset = mpMosaicWindow->offset();
    QRect selectionRect = mpMosaicWindow->selectionGeometry();
    int scaleValue = mpMosaicWindow->scaleValue();
    
    if(mpCurrentLayer != NULL)
    {
      mpCurrentLayer->properties().setOffset(offset);
      mpCurrentLayer->properties().setSelectionGeometry(selectionRect);
      mpCurrentLayer->properties().setScale(scaleValue);
    }
  }
}

void SeMainWindow::on_cmdModifyReset_clicked()
{
  ui->lblImageInfo->setText("-.-");
  ui->cmdModifyReset->setEnabled(false);
  ui->cmdModifyImage->setEnabled(false);
}

void SeMainWindow::on_actionLoad_triggered()
{
  bool res = false;

  if(ui->actionNew->isEnabled())
  {
    res = closeProject();
    if(res == false)
    {
      return;
    }
  }

  QString fname = QFileDialog::getOpenFileName(this, tr("Scene filename..."));
  if(fname.isEmpty()) { return; }
  
  this->loadConfiguration(fname);
}

void SeMainWindow::on_actionSave_triggered()
{
  this->storeConfiguration();
}

void SeMainWindow::on_actionSave_As_triggered()
{
  QString fname = QFileDialog::getSaveFileName(this, tr("Target filename..."));
  if(fname.isEmpty()) { return; }
  
  this->storeConfiguration(fname);
}

void SeMainWindow::on_actionNew_triggered()
{
  bool res = this->closeProject();
  SeTreeSceneItem::resetInstanceCounter();
  Q_UNUSED(res);
}

bool SeMainWindow::closeProject()
{
  QMessageBox::StandardButton res = QMessageBox::question(
      this
    , tr("Create a new scene configuration?")
    , tr("Do you like to close the current configuration?")
  );
  
  if(res == QMessageBox::No)
  {
    return false;
  }
  
  this->storeConfiguration();
  
  QStringList ids = ui->treeScenes->identifiers();
  for(QString id : ids)
  {
    SeSceneLayer *layer = mpScene->layer(id);
    SE_CONT4NULL(layer);
    
    delete layer;
    layer = NULL;
  }  
  ui->treeScenes->clear();
  
  ui->actionNew->setEnabled(false);
  
  mDeploymentFilename.clear();
  mDeploymentDirname.clear();
  mCfgFilename.clear();
  
  mFileStoredAsProject = false;
  
  return true;
}

void SeMainWindow::on_chkLayerEnabled_clicked(bool checked)
{
  if(mpCurrentLayer == NULL) { return; }
  
  mpCurrentLayer->properties().setEnabled(checked);
}

void SeMainWindow::on_cmbChangeMode_2_currentIndexChanged(int index)
{
  if(mpCurrentLayer == NULL) { return; }
  if(index == 0) { return; }
  
  index = index - 1;
  
  QList<SeSceneLed*> leds; 
  int n = mpScene->selectedLeds(leds);
  
  if(n <= 0) { return; }

  for(SeSceneLed *pled : leds)
  {
    pled->properties().setTransitionMode((SeSceneItemProperties::TransitionMode)index);
  }
  
  mpScene->update();
}

void SeMainWindow::on_cmdChangeColor_2_clicked()
{
  if(mpCurrentLayer == NULL) { return; }
  
  QList<SeSceneLed*> leds; 
  int n = mpScene->selectedLeds(leds);

  if(n <= 0) { return; }

  QColor currentColor = mpCurrentLed->properties().brushColor();

  if(currentColor.isValid() == false) 
  {
    currentColor = QColor(255, 255, 255);
  }

  QColor c = QColorDialog::getColor(currentColor);
  
  if(c.isValid() == false) { return; }
  
  this->setChangeColor(c);
  
  for(SeSceneLed *pled : leds)
  {
    pled->properties().setBrushColor(c);
    pled->properties().setPenColor(c);
  }
  
  mpScene->update();
}

void SeMainWindow::on_actionAbout_Qt_triggered()
{
  QMessageBox::aboutQt(this, tr("About Qt..."));
}

void SeMainWindow::on_cmdUnselect_clicked()
{
  if(mpScene == NULL) { return; }
  
  QList<SeSceneLed*> leds;
  int n = mpScene->sceneLeds(leds, mpCurrentLayer);
  
  SceneEditor::__statusBar->showMessage(tr("%1 LEDs selected!").arg(n));
  
  for(SeSceneLed *pitem : leds)
  {
    SE_CONT4NULL(pitem);
    pitem->setSelected(true);
  }
}

void SeMainWindow::on_cmdSelectAll_clicked()
{
  if(mpScene == NULL) { return; }
  
  QList<SeSceneLed*> leds;
  int n = mpScene->sceneLeds(leds, mpCurrentLayer);
  
  SceneEditor::__statusBar->showMessage(tr("%1 LEDs selected!").arg(n));
  
  for(SeSceneLed *pitem : leds)
  {
    SE_CONT4NULL(pitem);
    pitem->setSelected(false);
  }
}

void SeMainWindow::on_actionExi_triggered()
{
  this->storeConfiguration();
  QCoreApplication::exit();
}

void SeMainWindow::on_tabWidget_currentChanged(int index)
{
  Q_UNUSED(index);
  if(mpScene == NULL) { return; }  
  ui->cmbChangeMode_2->setCurrentIndex(0);
}

void SeMainWindow::on_actionAbout_SceneEditor_triggered()
{
    QString copyright = tr("Copyright %1 2015, %2").arg("&copy;").arg("Christian Benjamin Ries");
    QString author = tr("Author: %1").arg("Christian Benjamin Ries");    
    QString web = tr("Website: %1").arg("www.christianbenjaminries.de");
    QString build = tr("Build: %1, %2").arg(__TIME__).arg(__DATE__);
    QString license = tr("License: %1").arg("MIT License, see www.opensource.org/licenses/MIT");
    
    QMessageBox::information(this
        , tr("About SceneEditor...")
        , tr("%1\n%2\n%3\n%4\n%5")
            .arg(copyright)
            .arg(author)
            .arg(web)
            .arg(build)
            .arg(license)
        );
}
