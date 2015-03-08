/*
 * Copyright (C) 2015, Christian Benjamin Ries
 * Website: http://www.christianbenjaminries.de
 * License: MIT License, http://opensource.org/licenses/MIT
 */

// SceneEditor
#include <SeWebSocket.h>

// Qt
#include <QDebug>
#include <QWebSocket>
#include <QAbstractSocket>

SeWebSocket::SeWebSocket(QObject *parent) 
    : QObject(parent)
{
    QObject::connect(&mWebSocket, &QWebSocket::connected, this, &SeWebSocket::onConnected);
    QObject::connect(&mWebSocket, &QWebSocket::textMessageReceived, this, &SeWebSocket::onMessageReceived);       
    QObject::connect(&mWebSocket, &QWebSocket::disconnected, this, &SeWebSocket::closed);    
}

void SeWebSocket::setUrlAndConnect(const QUrl &url)
{
    mUrl = url;
    mWebSocket.open(QUrl(mUrl));
}

bool SeWebSocket::send(const QString &message)
{
    if(message.isEmpty()) return false;
    if(mWebSocket.state() != QAbstractSocket::SocketState::ConnectedState) return false;
    bool res = mWebSocket.sendTextMessage(message) > 0;
    return res;
}

void SeWebSocket::shutdown()
{
    mWebSocket.close(QWebSocketProtocol::CloseCodeNormal, tr("User shutdown."));
}

void SeWebSocket::onConnected()
{
    emit connected();
}

void SeWebSocket::onMessageReceived(QString m)
{
    emit message(m);
}
