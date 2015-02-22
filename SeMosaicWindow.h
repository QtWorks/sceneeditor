/*
 * Copyright (C) 2015, Christian Benjamin Ries
 * Website: http://www.christianbenjaminries.de
 * License: MIT License, http://opensource.org/licenses/MIT
 */

#ifndef __SEMOSAICWINDOW_H__
#define __SEMOSAICWINDOW_H__

#pragma once

// Qt
#include <QWidget>
#include <QDialog>
#include <QPixmap>
#include <QImage>
#include <QFrame>
#include <QLabel>

namespace Ui {
  class SeMosaicWindow;
}

#define PIXEL_FOR_STEP 5

// forward-declaration
class SeMainWindow;

/**
 * @brief The SeMosaicSelectionFrame class
 */
class SeMosaicSelectionFrame 
  : public QFrame
{
  Q_OBJECT
public:
  explicit SeMosaicSelectionFrame(QWidget *parent=0);
  
protected:
  void mousePressEvent(QMouseEvent*);
  void mouseMoveEvent(QMouseEvent*);
  
public:
  void setWidth(int width);
  void setHeight(int height);
  
signals:
  void selectionChanged(int x, int y, int width, int height);
  
private:
  QPoint mOldPoint;
};

/**
 * @brief The SeMosaicImageFrame class
 */
class SeMosaicImageFrame 
  : public QLabel
{
  Q_OBJECT
public:
  explicit SeMosaicImageFrame(QWidget *parent=0);
  
  friend class SeMosaicWindow;
  
  void setImage(const QPixmap & pixmap, bool keepOffset=false);
  void setImage(const QString & filename, bool keepOffset=false);
  
public slots:
  void scale(int factor);  
  
private:
  QPixmap mOriginalImage;
  QPixmap mPreparedImage;
  
protected:
  void cropAndShow();
  
protected:
  void paintEvent(QPaintEvent*);
  void mousePressEvent(QMouseEvent*);
  void mouseMoveEvent(QMouseEvent*);
  
signals:
  void offsetChanged(QPoint);
  void offsetChanged();  
  void scaleChanged();
    
public:
  const QPoint offset() const { return mOffset; }
  void setOffset(const QPoint & offset);
    
private:
  QPoint mOldPoint;
  QPoint mOffset;
};

/**
 * @brief The SeMosaicWindow class
 */
class SeMosaicWindow 
  : public QDialog
{
  Q_OBJECT
  
public:
  explicit SeMosaicWindow(QWidget *parent = 0);
  ~SeMosaicWindow();
  
public:
  const QPoint offset() { return mpImageFrame->offset(); }
  void setOffset(const QPoint & offset);
  
  const QRect selectionGeometry() { return mSelectionFrameGeometry; }
  void setSelectionGeometry(const QRect & geometry);
  
  const int scaleValue();
  void setScaleValue(int value);
  
private:
  void apply();
  
public:
  virtual void accept();
  virtual void done(int r);
  virtual void reject();   
  
private slots:
  void on_cmdApply_clicked();  
  void on_cmdCancel_clicked();  
  void on_cmdOk_clicked();  
  void on_sliderScale_valueChanged(int value);  
  void on_chkShowFrame_toggled(bool checked);  
  void on_spinSizeWidth_valueChanged(int arg1);  
  void on_spinSizeHeight_valueChanged(int arg1);  
  void on_chkMosaic_toggled(bool checked);  
  void on_cmbMosaicMode_activated(const QString &arg1);  
  void on_spinScale_valueChanged(int arg1);
  void on_spinSelectionOffsetX_valueChanged(int soffx);
  void on_spinSelectionOffsetY_valueChanged(int soffy);
  
public slots:
  void updateSelectionFrameGeometry(int x, int y, int width, int height);
  void updateSelectionFrameGeometry();    
  void original2preview();
  
  QColor average(int x, int y, QImage *pimg);
  QColor median(int x, int y, QImage *pimg);
  QColor minimum(int x, int y, QImage *pimg);
  QColor maximum(int x, int y, QImage *pimg);
  
  void pixelArea(int x, int y, QImage *pimg, QList<int> & r, QList<int> & g, QList<int> & b);
    
public:
  template<class T> T mathAvg(const QList<T> & list) {
    T _v = 0.f;
    for(auto vv : list) { _v += vv; }
    return (_v / (T) list.count());
  }
  
public:
  static inline int pixelSteps() { return PIXEL_FOR_STEP; }  
  
private:
  Ui::SeMosaicWindow *ui;
  SeMainWindow *mpMainWindow;
  SeMosaicImageFrame *mpImageFrame;
  SeMosaicSelectionFrame *mpSelectFrame;
  
public:
  enum MosaicMode { Average, Median, Minimum, Maximum };
  
  void setMosaicMode(MosaicMode mode);
  MosaicMode mosaicMode() const { return mRenderMosaicMode; }
  
  void setImage(const QString & imgFilepath, bool keepOffset=false);
  
private:
  bool mRenderMosaic;  
  MosaicMode mRenderMosaicMode;  
  
  QRect mSelectionFrameGeometry;
  
  QPixmap mPixmap;
};

#endif // __SEMOSAICWINDOW_H__
