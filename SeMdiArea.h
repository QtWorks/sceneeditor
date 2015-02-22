/*
 * Copyright (C) 2015, Christian Benjamin Ries
 * Website: http://www.christianbenjaminries.de
 * License: MIT License, http://opensource.org/licenses/MIT
 */

#ifndef __SEMDIAREA_H__
#define __SEMDIAREA_H__

#pragma once

// Qt
#include <QWidget>
#include <QMdiArea>
#include <QMdiSubWindow>

/**
 * @brief The SeMdiArea class
 */
class SeMdiArea 
  : public QMdiArea
{
  Q_OBJECT
public:
  SeMdiArea(QWidget *parent=0);
  ~SeMdiArea();
};



#endif // __SEMDIAREA_H__

