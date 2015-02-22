/*
 * Copyright (C) 2015, Christian Benjamin Ries
 * Website: http://www.christianbenjaminries.de
 * License: MIT License, http://opensource.org/licenses/MIT
 */

// SceneEditor
#include <SeMdiArea.h>
#include <SeSceneView.h>

// Qt
#include <QDebug>
#include <QLayout>
#include <QMdiArea>

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

SeMdiArea::SeMdiArea(QWidget *parent)
  : QMdiArea(parent)
{
}

SeMdiArea::~SeMdiArea()
{
}
