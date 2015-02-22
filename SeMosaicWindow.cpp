/*
 * Copyright (C) 2015, Christian Benjamin Ries
 * Website: http://www.christianbenjaminries.de
 * License: MIT License, http://opensource.org/licenses/MIT
 */

// SceneEditor
#include <SeMainWindow.h>
#include <SeMosaicWindow.h>
#include <ui_SeMosaicWindow.h>

// Qt
#include <QRgb>
#include <QDebug>
#include <QPaintEvent>
#include <QMouseEvent>

#include <QPainter>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

SeMosaicSelectionFrame::SeMosaicSelectionFrame(QWidget *parent)
  : QFrame(parent)
{
  this->setStyleSheet("background-color: rgba(43, 135, 255, 100);");
}

void SeMosaicSelectionFrame::mousePressEvent(QMouseEvent *ev)
{
  this->mOldPoint = ev->globalPos();
}

void SeMosaicSelectionFrame::mouseMoveEvent(QMouseEvent *ev)
{
  const QPoint delta = ev->globalPos() - mOldPoint;
  this->move(x() + delta.x(), y() + delta.y());
  this->mOldPoint = ev->globalPos();  
  emit selectionChanged(pos().x(), pos().y(), width(), height());
}

void SeMosaicSelectionFrame::setWidth(int width)
{
  QPoint p = this->pos();
  int w = width * SeMosaicWindow::pixelSteps();
  this->setGeometry(p.x(), p.y(), w, this->height());  
  emit selectionChanged(p.x(), p.y(), this->width(), this->height());
}

void SeMosaicSelectionFrame::setHeight(int height)
{
  QPoint p = this->pos(); 
  int h = height * SeMosaicWindow::pixelSteps();
  this->setGeometry(p.x(), p.y(), this->width(), h);  
  emit selectionChanged(p.x() ,p.y(), this->width(), this->height());
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

SeMosaicImageFrame::SeMosaicImageFrame(QWidget *parent)
  : QLabel(parent)
{
  this->setStyleSheet("background-color: #ffffff; border: 1px solid black;"); 
}

void SeMosaicImageFrame::setImage(
    const QPixmap &pixmap
  , bool keepOffset
) {
  int w = pixmap.width();
  int h = pixmap.height();
  
  int lblW = QLabel::width();
  int lblH = QLabel::height();
    
  int mode = 0;
    
  if(w < lblW || h < lblH)
  {
    QPixmap pix(lblW * 4, lblH * 4);
    QPainter paint;
    paint.begin(&pix);
    paint.setBrush(QColor(255, 255, 255));
    paint.setPen(QColor(0, 255, 0));
    paint.drawRect(0, 0, lblW, lblH);
    
    paint.drawPixmap(lblW/2.f, lblH/2.f, w, h, pixmap);
    paint.end();
    
    mOriginalImage = pix.copy();
    mPreparedImage = pix.copy();
    
    mode = 1;
  }
  else
  {
    mOriginalImage = pixmap.copy();
    mPreparedImage = pixmap.copy();
    
    mode = 2;
  }
  
  if(keepOffset == false)
  {  
    mOffset.setX(0);
    mOffset.setY(0);
  }
  
  if(mode == 2)
  {
    this->cropAndShow();
  }
  else if(mode == 1)
  {
    this->setPixmap(mOriginalImage);
  }
  else
  {
    // ...
  }    
}

void SeMosaicImageFrame::setImage(
    const QString &filename
  , bool keepOffset
) {
  this->setImage(QPixmap(filename), keepOffset);
}

void SeMosaicImageFrame::scale(int v)
{
  int w = mOriginalImage.width();
  int h = mOriginalImage.height();
     
  float factor = v / 100.f;
  
  float w0 = w * factor;
  float h0 = h * factor;
  
  QPixmap scaledPixmap = mOriginalImage.copy();
  
  scaledPixmap = scaledPixmap.scaled(w0, h0);
  
  QGraphicsScene scene;
  QGraphicsPixmapItem *p = scene.addPixmap(scaledPixmap);
  if(p != NULL)
  {
    p->setPos(100, 100);
    p->update();
    
    QPixmap outputPix(
        scaledPixmap.width() + 200
      , scaledPixmap.height() + 200
    );    
    outputPix.fill(Qt::transparent);    
    QPainter painter(&outputPix);
    scene.render(&painter, QRectF(100.f, 100.f, scaledPixmap.width(), scaledPixmap.height()));
    
    //outputPix.save("C:/temp/scaled.png");
    
    mPreparedImage = outputPix.copy();
  }
    
  this->cropAndShow();
  
  emit scaleChanged();
}

void SeMosaicImageFrame::cropAndShow()
{
  int w = this->width();
  int h = this->height();
  int x = this->mOffset.x();
  int y = this->mOffset.y();
  
  QRect r(x, y, w, h);
  
  QPixmap cropped = mPreparedImage.copy(r);
    
  this->setPixmap(cropped);
}

void SeMosaicImageFrame::paintEvent(QPaintEvent *ev)
{
  QLabel::paintEvent(ev);
}

void SeMosaicImageFrame::mousePressEvent(QMouseEvent *ev)
{
  this->mOldPoint = ev->globalPos();
}

void SeMosaicImageFrame::mouseMoveEvent(QMouseEvent *ev)
{
  const QPoint delta = ev->globalPos() - mOldPoint;
  
  mOffset.rx() += delta.x();
  mOffset.ry() += delta.y();
  
  this->cropAndShow();
  
  this->mOldPoint = ev->globalPos();
  
  emit offsetChanged(mOffset);
  emit offsetChanged();
}

void SeMosaicImageFrame::setOffset(const QPoint &offset)
{
  this->mOffset = offset;

  this->cropAndShow();  
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

SeMosaicWindow::SeMosaicWindow(QWidget *parent) 
  : QDialog(parent)
  , ui(new Ui::SeMosaicWindow)
  , mRenderMosaic(true)
  , mRenderMosaicMode(MosaicMode::Average)
{
  ui->setupUi(this);
  
  mpMainWindow = qobject_cast<SeMainWindow*>(parent);
  
  mpImageFrame = ui->lblImgOriginal;
  mpSelectFrame = new SeMosaicSelectionFrame(mpImageFrame);
  mpSelectFrame->setWidth(ui->spinSizeWidth->value());
  mpSelectFrame->setHeight(ui->spinSizeHeight->value());
  this->setSelectionGeometry(QRect(0, 0, mpSelectFrame->width(), mpSelectFrame->height()));
  this->updateSelectionFrameGeometry();
    
  const QPixmap *ptr = ui->lblImgOriginal->pixmap();
  if(ptr != NULL)
  {  
    mPixmap = ptr->copy();
  }    
  
  QObject::connect( mpSelectFrame, SIGNAL(selectionChanged(int,int,int,int))
                  , this, SLOT(updateSelectionFrameGeometry(int,int,int,int)));
                  
  QObject::connect( mpImageFrame, SIGNAL(offsetChanged())
                  , this, SLOT(original2preview()));
  QObject::connect( mpImageFrame, SIGNAL(scaleChanged())
                  , this, SLOT(original2preview()));                
}

SeMosaicWindow::~SeMosaicWindow()
{
  delete ui;
  
  if(mpSelectFrame != NULL) { delete mpSelectFrame; mpSelectFrame = NULL; }  
}

void SeMosaicWindow::updateSelectionFrameGeometry(int x, int y, int width, int height)
{
  Q_UNUSED(x);
  Q_UNUSED(y);
  Q_UNUSED(width);
  Q_UNUSED(height);

  mSelectionFrameGeometry.setX(x);
  mSelectionFrameGeometry.setY(y);
  mSelectionFrameGeometry.setWidth(width);
  mSelectionFrameGeometry.setHeight(height);

  ui->spinSelectionOffsetX->setValue(x);
  ui->spinSelectionOffsetY->setValue(y);

  //ui->spinSizeWidth->setValue( width / SeMosaicWindow::pixelSteps() );
  //ui->spinSizeHeight->setValue( height / SeMosaicWindow::pixelSteps() );
    
  this->original2preview();
}

void SeMosaicWindow::updateSelectionFrameGeometry()
{
  if(mpSelectFrame == NULL) { return; }
  
  mpSelectFrame->setWidth(ui->spinSizeWidth->value());
  mpSelectFrame->setHeight(ui->spinSizeHeight->value());  
}

void SeMosaicWindow::original2preview()
{
  QPixmap orignalPix = mpImageFrame->pixmap()->copy();
  
  int offsetX = mpSelectFrame->pos().x();
  int offsetY = mpSelectFrame->pos().y();
  int w = mpSelectFrame->width();
  int h = mpSelectFrame->height();
  
  QPixmap previewPix = orignalPix.copy(offsetX, offsetY, w, h);
  
  if(mRenderMosaic == true)
  {
    QImage previewImg = previewPix.toImage();
    QImage targetImg = previewImg.copy();

    for(int x=0; x < previewPix.width(); x += SeMosaicWindow::pixelSteps())
    {
      for(int y=0; y < previewPix.height(); y += SeMosaicWindow::pixelSteps())
      {
        QColor pixelColor;
        
        switch(this->mosaicMode())
        {
          case Average: pixelColor = this->average(x, y, &previewImg); break;
          case Median:  pixelColor = this->median(x, y, &previewImg);  break;
          case Minimum: pixelColor = this->minimum(x, y, &previewImg); break;
          case Maximum: pixelColor = this->maximum(x, y, &previewImg); break;
          default: /* unknown mode */ ;
        }
      
        for(int ix=x; ix < x + SeMosaicWindow::pixelSteps(); ix++)
        {
          for(int iy=y; iy < y + SeMosaicWindow::pixelSteps(); iy++)
          {          
            targetImg.setPixel(ix, iy, pixelColor.rgb());
          }
        }
      }
    }

    QPixmap p = QPixmap::fromImage(targetImg);

    ui->lblImagePreview->setPixmap(p);
  }
  else
  {
    ui->lblImagePreview->setPixmap(previewPix);
  } 
}

QColor SeMosaicWindow::average(int x, int y, QImage *pimg)
{
  if(pimg == NULL) { return QColor(255,255,255); }
  if(x < 0 || y < 0) { return QColor(255,255,255); }
  
  QList<int> r_values;
  QList<int> g_values;
  QList<int> b_values;
  
  this->pixelArea(x, y, pimg, r_values, g_values, b_values);
  
  QList<int> rgbAverage;
  rgbAverage.append(mathAvg<int>(r_values));
  rgbAverage.append(mathAvg<int>(g_values));
  rgbAverage.append(mathAvg<int>(b_values));
  
  return QColor(rgbAverage[0], rgbAverage[1], rgbAverage[2]);
}

QColor SeMosaicWindow::median(int x, int y, QImage *pimg)
{
  if(pimg == NULL) { return QColor(255,255,255); }
  if(x < 0 || y < 0) { return QColor(255,255,255); }
  
  QList<int> r_values;
  QList<int> g_values;
  QList<int> b_values;
  
  this->pixelArea(x, y, pimg, r_values, g_values, b_values);
  
  qSort(r_values.begin(), r_values.end());
  qSort(g_values.begin(), g_values.end());
  qSort(b_values.begin(), b_values.end());
  
  QList<int> middleValues;
  middleValues.append(r_values[SeMosaicWindow::pixelSteps() / 2.f]);
  middleValues.append(g_values[SeMosaicWindow::pixelSteps() / 2.f]); 
  middleValues.append(b_values[SeMosaicWindow::pixelSteps() / 2.f]); 
  
  return QColor(middleValues[0], middleValues[1], middleValues[2]);
}

QColor SeMosaicWindow::minimum(int x, int y, QImage *pimg)
{
  if(pimg == NULL) { return QColor(255,255,255); }
  if(x < 0 || y < 0) { return QColor(255,255,255); }
  
  QList<int> r_values;
  QList<int> g_values;
  QList<int> b_values;
  
  this->pixelArea(x, y, pimg, r_values, g_values, b_values);

  qSort(r_values.begin(), r_values.end());
  qSort(g_values.begin(), g_values.end());
  qSort(b_values.begin(), b_values.end());
  
  return QColor(r_values[0], g_values[0], b_values[0]);
}

QColor SeMosaicWindow::maximum(int x, int y, QImage *pimg)
{
  if(pimg == NULL) { return QColor(255,255,255); }
  if(x < 0 || y < 0) { return QColor(255,255,255); }
  
  QList<int> r_values;
  QList<int> g_values;
  QList<int> b_values;
  
  this->pixelArea(x, y, pimg, r_values, g_values, b_values);
  
  qSort(r_values.begin(), r_values.end());
  qSort(g_values.begin(), g_values.end());
  qSort(b_values.begin(), b_values.end());
  
  return QColor(
      r_values[ r_values.count() - 1]
    , g_values[ g_values.count() - 1]
    , b_values[ b_values.count() - 1]);
}

void SeMosaicWindow::pixelArea(
    int x, int y
  , QImage *pimg
  , QList<int> & r_list
  , QList<int> & g_list
  , QList<int> & b_list
) {
  for(int ix=x; ix < x + SeMosaicWindow::pixelSteps(); ix++)
  {
    for(int iy=y; iy < y + SeMosaicWindow::pixelSteps(); iy++)
    {  
      QRgb rgb = pimg->pixel(ix, iy);
      
      r_list.append(qRed(rgb));
      g_list.append(qGreen(rgb));
      b_list.append(qBlue(rgb));
    }
  }  
}

void SeMosaicWindow::setOffset(const QPoint &offset)
{
  mpImageFrame->setOffset(offset);  
}

void SeMosaicWindow::setSelectionGeometry(const QRect & geometry)
{
  mSelectionFrameGeometry = geometry;
  
  if(mpSelectFrame != NULL)
  {
    mpSelectFrame->setGeometry(mSelectionFrameGeometry);
  }
}

const int SeMosaicWindow::scaleValue()
{
  return ui->spinScale->value();
}

void SeMosaicWindow::setScaleValue(int value)
{
  ui->sliderScale->setValue(value);
  this->on_sliderScale_valueChanged(value);
}

void SeMosaicWindow::apply()
{
  if(mpMainWindow == NULL) { return; }
  
  QPixmap p = ui->lblImagePreview->pixmap()->copy();
  
  mpMainWindow->applyPixmap(p);
}

void SeMosaicWindow::accept()
{  
  QDialog::accept();
}

void SeMosaicWindow::done(int r)
{
  QDialog::done(r);
}

void SeMosaicWindow::reject()
{
  QDialog::reject();
}

void SeMosaicWindow::on_cmdApply_clicked()
{
  this->apply();
}

void SeMosaicWindow::on_cmdCancel_clicked()
{
  this->reject();
}

void SeMosaicWindow::on_cmdOk_clicked()
{
  this->apply();
  this->done(1);
}

void SeMosaicWindow::on_sliderScale_valueChanged(int value)
{
  ui->spinScale->setValue(value);
  mpImageFrame->scale(value);
}

void SeMosaicWindow::on_spinScale_valueChanged(int value)
{
  ui->sliderScale->setValue(value);
  mpImageFrame->scale(value);
}

void SeMosaicWindow::on_chkShowFrame_toggled(bool checked)
{
  if(checked == true) 
  {
    mpSelectFrame->show();
  }  
  else
  {
    mpSelectFrame->hide();
  }
}

void SeMosaicWindow::on_spinSizeWidth_valueChanged(int v)
{
  Q_UNUSED(v);
  this->updateSelectionFrameGeometry();
}

void SeMosaicWindow::on_spinSizeHeight_valueChanged(int v)
{
  Q_UNUSED(v);
  this->updateSelectionFrameGeometry();
}

void SeMosaicWindow::on_chkMosaic_toggled(bool checked)
{
  this->ui->cmbMosaicMode->setEnabled(checked);
  this->mRenderMosaic = checked;  
  this->original2preview();
}

void SeMosaicWindow::setMosaicMode(MosaicMode mode)
{
  this->mRenderMosaicMode = mode;
  this->updateSelectionFrameGeometry();
}

void SeMosaicWindow::setImage(const QString &imgFilepath, bool keepOffset)
{
  if(mpImageFrame != NULL)
  {
    mpImageFrame->setImage(imgFilepath, keepOffset);
  }
}

void SeMosaicWindow::on_cmbMosaicMode_activated(const QString &modename)
{
  QString m = modename.toLower().trimmed();
  
  if(m == "average") { this->setMosaicMode(MosaicMode::Average); }
  else if(m == "median") { this->setMosaicMode(MosaicMode::Median); }
  else if(m == "minimum") { this->setMosaicMode(MosaicMode::Minimum); }
  else if(m == "maximum") { this->setMosaicMode(MosaicMode::Maximum); }
  else
  {
    // unknown mosaic mode
  }
}

void SeMosaicWindow::on_spinSelectionOffsetX_valueChanged(int soffx)
{
  SeMosaicSelectionFrame *f = mpSelectFrame;
  QPoint pos = f->pos();
  pos.setX(soffx);
  f->move(pos);
  this->updateSelectionFrameGeometry(f->pos().x(), f->pos().y(), f->width(), f->height());
}

void SeMosaicWindow::on_spinSelectionOffsetY_valueChanged(int soffy)
{
  SeMosaicSelectionFrame *f = mpSelectFrame;
  QPoint pos = mpSelectFrame->pos();
  pos.setY(soffy);
  mpSelectFrame->move(pos);
  this->updateSelectionFrameGeometry(f->pos().x(), f->pos().y(), f->width(), f->height());
}
