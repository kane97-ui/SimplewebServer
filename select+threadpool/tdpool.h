#ifndef _THREAD_POOL_H
#define _THREAD_POOL_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <functional>
#include <iostream>
#include <condition_variable>
#include <future>
 

#include <assert.h>
using namespace std;
class ThreadPool{
    public:
        typedef function<void(int i)> func;
        //类模版std::function是一种通用、多态的函数封装。std::function的实例可以对任何可以调用的目标实体进行存储、复制、和调用操作，这些目标实体包括普通函数、Lambda表达式、函数指针、以及其它函数对象等。std::function对象是对C++中现有的可调用实体的一种类型安全的包裹（我们知道像函数指针这类可调用实体，是类型不安全的）。
        enum taskProritytask{low, middle, high};
        typedef pair<taskProritytask, pair<func, int>> task;
        ThreadPool(int m_thred_size = 3);
        ~ThreadPool();
        void start();
        void end();
        void AddTask(const task&);
    
    private:
        pair<func, int> take();
        void ThreadLoop();
        bool m_started;
        int m_thread_size;
        vector<thread*> threads;
        condition_variable m_cond;
        struct TaskPriorityCmp
            {
                bool operator()(const task& a, const task& b) const
                {
                    return a.first > b.first;
                }
            };

        priority_queue<task, vector<task>, TaskPriorityCmp> task_queue;
        mutex m_mutex;

};
#endif