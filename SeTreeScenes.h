/*
 * Copyright (C) 2015, Christian Benjamin Ries
 * Website: http://www.christianbenjaminries.de
 * License: MIT License, http://opensource.org/licenses/MIT
 */

#ifndef __SETREESCENES_H__
#define __SETREESCENES_H__

#pragma once

// Qt
#include <QDebug>
#include <QWidget>
#include <QMdiArea>
#include <QKeyEvent>
#include <QJsonArray>
#include <QStringList>
#include <QTreeWidget>
#include <QTreeWidgetItem>

/**
 * @brief The SeTreeSceneItem class
 */
class SeTreeSceneItem
  : public QTreeWidgetItem
{
public:
  explicit SeTreeSceneItem(const QString & uuid);
  ~SeTreeSceneItem();
  
  enum Roles {
    Uuid = Qt::UserRole + 1
  };
  
public:
  static void resetInstanceCounter() { __instances = 0; }
  
private:
  static int __instances;  
    
public:
  QVariant data(int column, int role) const;
  void setData(int column, int role, const QVariant &value);  
};

/**
 * @brief The SeTreeScenes class
 */
class SeTreeScenes 
  : public QTreeWidget
{
  Q_OBJECT
public:
  explicit SeTreeScenes(QWidget *parent = 0);
  ~SeTreeScenes();
  
  QStringList identifiers();
  
  int indexOf(const QString & identifier);
  
protected:
  void contextMenuEvent(QContextMenuEvent *e); 
  void keyPressEvent(QKeyEvent *e); 
  void keyReleaseEvent(QKeyEvent *e);
  
signals:
  void sceneLayerClicked(const QString & identifier);
  void createScene(const QString & identifier);
  void removeScene(const QString & identifier);
  void duplicateScene(const QString & existingIdentifier, const QString & createdIdentifier);
      
public slots:
  SeTreeSceneItem *addScene(const QString & identifier="", bool informEnvironment=true);
  bool removeScene();
  bool duplicateScene();
};

#endif // __SETREESCENES_H__
