#ifndef LOG_H
#define LOG_H

#include <QObject>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QMutex>
#include <QQueue>
#include <QThread>
#include <QDate>
#include <QWaitCondition>

class Log: public QObject
{
    Q_OBJECT
public:
    static Log& Instance();
    void init();
    QString getLevel(int level);
    void writeLog(int level, const QString &msg);
    void closeLog();

private slots:
    void processLoop();

signals:
    void logMessage(const QString &msg);

private:
    Log();
    ~Log();
    Log(const Log&) = delete;
    Log& operator=(const Log&) = delete;
    QString createFileName();
    void checkLogFile();
    void cleanupOldLogs();

    QString path;
    QFile logFile;
    QTextStream stream;
    QString currentDate;
    QQueue<QString> queue;
    QMutex mutex;
    QWaitCondition cond;
    QThread logThread;
    QElapsedTimer flushTimer;
    bool isClosed;
    int fileIndex = 1;
    int maxFileSizeMB = 5;
    int cleanDate = 7;
    int flushTimeout = 200;
};

#define LOG_DEBUG(msg) Log::Instance().writeLog(0, msg)
#define LOG_INFO(msg) Log::Instance().writeLog(1, msg)
#define LOG_WARN(msg) Log::Instance().writeLog(2, msg)
#define LOG_ERROR(msg) Log::Instance().writeLog(3, msg)
#endif // LOG_H
