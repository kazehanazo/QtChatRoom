#ifndef THREADPOOL_H
#define THREADPOOL_H

#pragma once
#include <QThread>
#include <QVector>
#include <QQueue>
#include <QMutex>
#include <QWaitCondition>
#include <functional>

class ThreadPool
{
public:
    static ThreadPool& Instance();
    void addTask(std::function<void()> task);
    ~ThreadPool();
private:
    explicit ThreadPool(size_t threadCount = 8);
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    QVector<QThread*> workers;
    QQueue<std::function<void()>> tasks;
    QMutex mutex;
    QWaitCondition cond;
    bool isClosed;
};

#endif // THREADPOOL_H
