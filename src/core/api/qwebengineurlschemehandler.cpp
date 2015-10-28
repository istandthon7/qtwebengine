/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwebengineurlschemehandler.h"

#include "qwebengineurlrequestjob.h"

QT_BEGIN_NAMESPACE

/*!
    \class QWebEngineUrlSchemeHandler
    \brief The QWebEngineUrlSchemeHandler is a base class for handling custom URL schemes.
    \since 5.6

    To implement a custom URL scheme for QtWebEngine, you must write a class derived from this class,
    and reimplement requestStarted().

    \inmodule QtWebEngineCore

*/

/*!
    \fn QWebEngineUrlSchemeHandler::destroyed(QWebEngineUrlSchemeHandler *handler)

    This signal is emitted when the custom URL scheme handler \a handler is deleted.
*/

/*!
    Constructs a new URL scheme handler.

    The handler is created with the parent \a parent.

  */
QWebEngineUrlSchemeHandler::QWebEngineUrlSchemeHandler(QObject *parent)
    : QObject(parent)
{
}

/*!
    Deletes a custom URL scheme handler.
*/
QWebEngineUrlSchemeHandler::~QWebEngineUrlSchemeHandler()
{
    Q_EMIT destroyed(this);
}

/*!
    \fn void QWebEngineUrlSchemeHandler::requestStarted(QWebEngineUrlRequestJob *request)

    This method is called whenever a request \a request for the registered scheme is started.

    This method must be reimplemented by all custom URL scheme handlers.
    The request is asynchronous and does not need to be handled right away.

    \sa QWebEngineUrlRequestJob
*/

QT_END_NAMESPACE
