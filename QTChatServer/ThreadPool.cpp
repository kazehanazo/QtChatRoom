#include "ThreadPool.h"

ThreadPool& ThreadPool::Instance()
{
    static ThreadPool pool(16);
    return pool;
}

ThreadPool::ThreadPool(size_t threadCount) : isClosed(false)
{
    for (int i = 0; i < threadCount; ++i)
    {
        QThread* t = QThread::create([this]() {
            auto &mutex = this->mutex;
            auto &cond = this->cond;
            auto &tasks = this->tasks;
            auto &isClosed = this->isClosed;
            while (true)
            {
                std::function<void ()> task;
                {
                    QMutexLocker locker(&mutex);
                    while(tasks.isEmpty() && !isClosed)
                    {
                        cond.wait(&mutex);
                    }
                    if(isClosed && tasks.isEmpty())
                    {
                        return;
                    }
                    task = std::move(tasks.dequeue());

                }
                if(task) task();
            }
        });
        workers.push_back(t);
        t->start();
    }
}

ThreadPool::~ThreadPool()
{
    {
        QMutexLocker locker(&mutex);
        isClosed = true;
    }
    cond.wakeAll();
    for (auto *w : workers)
    {
        w->wait();
        delete w;
    }
}

void ThreadPool::addTask(std::function<void()> task)
{
    QMutexLocker locker(&mutex);
    tasks.enqueue(std::move(task));
    cond.wakeOne();
}
// QMutexLocker locker(&mutex);
// if(!tasks.isEmpty())
// {
//     task = std::move(tasks.dequeue());
// }
// else if(isClosed) break;
// else cond.wait(&mutex);
