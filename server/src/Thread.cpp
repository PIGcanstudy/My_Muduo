#include "Thread.h"
#include <semaphore.h>

#include "CurrentThread.h"

std::atomic_int Thread::numCreated_(0); //静态成员变量 要在类外单独进行初始化

Thread::Thread(ThreadFunc func, const std::string &name)
    : name_(name)
    , started_(false)
    , joined_(false)
    , tid_(0)
    , func_(std::move(func))
{
    setDefaultName();
}

Thread::~Thread() {
    //线程已经运行起来了，并且没有joined_
    if (started_ && !joined_)
    {
        //thread类提供的分离线程的方法
        thread_->detach();
    }
}

void Thread::setDefaultName() {
    int num = ++numCreated_;
    if(name_.empty())
    {
        char buf[32] = {0};
        snprintf(buf,sizeof buf,"Thread%d",num);
        name_=buf;
    }
}

void Thread::start() {
    started_ = true;
    sem_t sem;
    // 初始化一个信号量
    sem_init(&sem, 0, 0);
    thread_ = std::make_shared<std::thread>([this, &sem]() {
        ++numCreated_;
        // 得到当前线程的tid
        tid_ = CurrentThread::tid();

        // 信号量 + 1，说明tid_已经有了
        sem_post(&sem);

        // 执行线程任务
        func_();
    });

    // 等待返回线程id
    sem_wait(&sem);
}

void Thread::join() {
    joined_ = true;
    thread_->join();
}

