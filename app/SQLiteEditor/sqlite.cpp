// MIT License
//
// Copyright (c) 2017 Niraj Desai
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "sqlite.h"

SQLite::SQLite(QObject *parent) :
    QObject(parent),
    m_dbThread(NULL),
    m_databasePath(QUrl(""))
{
    //    qRegisterMetaType< QList<QSqlRecord> >( "QList<QSqlRecord>" );

    qDebug() << __PRETTY_FUNCTION__;
    setStatus(Ready);
    connect(this, SIGNAL(databasePathChanged(QUrl)),
            this, SLOT(createThread(QUrl)));
}

SQLite::~SQLite()
{
    if(m_dbThread){
        m_dbThread->quit();
        m_dbThread->wait();
    }
}

void SQLite::createThread(QUrl databasePath)
{
    qDebug() << __PRETTY_FUNCTION__;
    if(databasePath.isEmpty()) return;

    m_query = "";
    if(m_dbThread && m_dbThread->isRunning())
    {
        qDebug() << "m_dbThread && m_dbThread->isRunning()";
        m_dbThread->quit();
        m_dbThread->wait();
        delete m_dbThread;
        qDebug() << "m_dbThread delete";
    }

    m_dbThread = new DbThread(this, databasePath.toString(QUrl::PreferLocalFile));
    connect( m_dbThread, SIGNAL( results( const QList<QSqlRecord>& ) ),
             this, SLOT( slotResults( const QList<QSqlRecord>& ) ) );
    connect( m_dbThread, SIGNAL(ready(bool)), this, SLOT(OndbThreadReady(bool)));
    m_dbThread->start();
}

void SQLite::executeQuery(QString queryStatement)
{
    qDebug() << __PRETTY_FUNCTION__;
    setStatus(Loading);
    m_query = queryStatement;
    m_dbThread->execute(queryStatement);
}

void SQLite::slotResults(const QList<QSqlRecord> &result)
{
    qDebug() << __PRETTY_FUNCTION__;
    qDebug() << "count="<<result.count();


    QVariantList val;

    QListIterator<QSqlRecord> sqlRecordsIterator(result);

    while(sqlRecordsIterator.hasNext())
    {
        QSqlRecord sqlRecord = sqlRecordsIterator.next();
        QVariantMap dataMap;
        for(int column = 0; column < sqlRecord.count(); column++)
        {
            //qDebug() << sqlRecord.fieldName(column);
            dataMap.insert(sqlRecord.fieldName(column), sqlRecord.value(column));
        }
        val.append(QVariant::fromValue(dataMap));
    }
    qDebug() << "count of list=" << val.count();
    resultsReady(val, m_query);
    setStatus(Ready);
}

void SQLite::OndbThreadReady(bool b)
{
    qDebug() << __PRETTY_FUNCTION__ << b;
    if(b){
        dbThreadStarted();
    }
}

void SQLite::dbThreadStarted()
{
    qDebug() << __PRETTY_FUNCTION__;
    connect(this, SIGNAL(queryChanged(QString)),
            this, SLOT(executeQuery(QString)));
    databaseOpened();
}

