/*
 * Copyright (C) 2015, Christian Benjamin Ries
 * Website: http://www.christianbenjaminries.de
 * License: MIT License, http://opensource.org/licenses/MIT
 */

#pragma once

#ifndef __SEWEBSOCKET_H__
#define __SEWEBSOCKET_H__

#include <QtCore/QObject>
#include <QtWebSockets/QWebSocket>

class SeWebSocket : public QObject
{
    Q_OBJECT
public:
    explicit SeWebSocket(QObject *parent = Q_NULLPTR);
    void setUrlAndConnect(const QUrl &url);
    bool send(const QString & message);
    void shutdown();

Q_SIGNALS:
    void message(QString m);
    void connected();
    void closed();

private Q_SLOTS:
    void onConnected();
    void onMessageReceived(QString message);

private:
    QWebSocket mWebSocket;
    QUrl mUrl;
};

#endif // __SEWEBSOCKET_H__
