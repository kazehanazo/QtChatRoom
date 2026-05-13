#include "Log.h"

Log& Log::Instance()
{
    static Log instance;
    return instance;
}

Log::Log(): path("D:/ChatServer/Log"), isClosed(false)
{
    QDir().mkpath(path);
    currentDate = QDate::currentDate().toString("yyyy-MM-dd");
    logFile.setFileName(createFileName());
    if (!logFile.open(QIODevice::Append | QIODevice::Text))
    {
        qWarning() << "Logger: failed to open log file";
    }
    else
    {
        stream.setDevice(&logFile);
    }
    flushTimer.start();
    this->moveToThread(&logThread);
    connect(&logThread, &QThread::started, this, [this](){processLoop();});
    logThread.start();
}

Log::~Log()
{
    closeLog();
}

QString Log::createFileName()
{
    return QString(path + "/%1_%2.log").arg(currentDate).arg(fileIndex, 3 ,10, QChar('0'));
}

QString Log::getLevel(int level)
{
    switch (level) {
    case 0:
        return "DEBUG";
        break;
    case 1:
        return "INFO";
        break;
    case 2:
        return "WARN";
        break;
    case 3:
        return "ERROR";
        break;
    default:
        return "INFO";
        break;
    }
}

void Log::writeLog(int level, const QString &msg)
{
    QString mLevel = getLevel(level);
    QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString line = QString("%1: [%2] %3\n").arg(mLevel, time, msg);
    {
        QMutexLocker locker(&mutex);
        queue.enqueue(line);
        cond.wakeOne();
    }
    emit logMessage(line);
}

void Log::closeLog()
{
    {
        QMutexLocker locker(&mutex);
        isClosed = true;
        cond.wakeAll();
    }
    logThread.quit();
    logThread.wait();
    if (logFile.isOpen())
    {
        stream.flush();
        logFile.close();
    }
}

void Log::checkLogFile()
{
    QString today = QDate::currentDate().toString("yyyy-MM-dd");
    if(today != currentDate)
    {
        currentDate = today;
        fileIndex = 1;
        logFile.close();
        logFile.setFileName(createFileName());
        if (!logFile.open(QIODevice::Append | QIODevice::Text))
        {
            qWarning() << "Logger: failed to open log file";
        }
        else
        {
            stream.setDevice(&logFile);
        }
    }
    else if(logFile.size() > maxFileSizeMB * 1024 * 1024)
    {
        fileIndex++;
        logFile.close();
        logFile.setFileName(createFileName());
        if (!logFile.open(QIODevice::Append | QIODevice::Text))
        {
            qWarning() << "Logger: failed to open log file";
        }
        else
        {
            stream.setDevice(&logFile);
        }
    }
    cleanupOldLogs();
}

void Log::cleanupOldLogs()
{
    QDir dir(path);
    if (!dir.exists()) return;
    QStringList files = dir.entryList(QStringList() << "*.log", QDir::Files);
    const QDate today = QDate::currentDate();
    for (const QString &file : files)
    {
        QString base = file.left(10); // "yyyy-MM-dd"
        QDate date = QDate::fromString(base, "yyyy-MM-dd");
        if (date.isValid() && date.daysTo(today) > cleanDate)
        {
            dir.remove(file);
        }
    }
}

void Log::processLoop()
{
    while (true)
    {
        QString msg;
        {
            QMutexLocker locker(&mutex);
            if (queue.isEmpty() && !isClosed) cond.wait(&mutex);
            if (!queue.isEmpty()) msg = queue.dequeue();
            else if(isClosed) break;
        }
        if (!msg.isEmpty())
        {
            checkLogFile();
            if (logFile.isOpen())
            {
                stream << msg;
                if (flushTimer.elapsed() >= flushTimeout)
                {
                    stream.flush();
                    flushTimer.restart();
                }
            }
            else qWarning() << "Logger: cannot write log, file not open. Log:" << msg;
        }
        else qDebug() << "The log is empty, cannot write log";
    }
    if (logFile.isOpen())
    {
        stream.flush();
        logFile.close();
    }
}

