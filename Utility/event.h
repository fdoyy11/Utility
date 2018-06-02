/**
* @file       event.h
* @brief      事件类
* @details    事件类声明与定义
* @author     fdoyy11 <fdoyy11@hotmail.com>
* @date       2018/06/02
* @copyright  (c) 2018-2022 fdoyy11@hotmail.com
*/
#pragma once
#include <chrono>
#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/time.h>
#include <pthread.h>
#include <memory.h>
#endif

namespace utility
{
    /**
    * @brief        事件基类
    */
    class base_event
    {
    public:
        /**
        * @brief        构造方法
        * @param[in]    is_manual，是否创建为手工重置事件
        * @param[in]    signal，初始化事件触发状态，true为触发状态，
        *               false为非触发状态
        */
        base_event(bool is_manual, bool signal)
        {
#ifdef _WIN32
            handle = ::CreateEvent(NULL, is_manual ? TRUE : FALSE,
                signal ? TRUE : FALSE, NULL);
#else
            pthread_mutex_init(&mtx, NULL);
            pthread_condattr_init(&cond_attr);
            pthread_condattr_setclock(&cond_attr, CLOCK_MONOTONIC);
            pthread_cond_init(&cond, &cond_attr);
            this->signal = signal;
            this->is_manual = is_manual;
#endif
        }

        /**
        * @brief        析构函数
        */
        ~base_event(void)
        {
#ifdef _WIN32
            ::CloseHandle(handle);
#else
            pthread_cond_destroy(&cond);
            pthread_condattr_destroy(&cond_attr);
            pthread_mutex_destroy(&mtx);
#endif
        }

#ifdef _WIN32
        /**
        * @brief        重载小括号运算符
        * @details      让事件可以作为HANDLE使用以应用于等待函数
        */
        operator HANDLE(void)
        {
            return handle;
        }
#else
        /**
        * @brief        广播
        * @details      激活所有等待此事件的线程
        */
        void broadcast()
        {
            pthread_mutex_lock(&mtx);
            pthread_cond_broadcast(&cond);
            signal = true;
            pthread_mutex_unlock(&mtx);
        }
#endif

        /**
        * @brief        触发事件
        * @details      触发事件，激活等待此事件的线程，若为自动重置事件则只激活
        *               一条正在等待此事件的线程，若为人工重置事件则激活所有所有
        *               正在等待此事件的线程
        */
        void set(void)
        {
#ifdef _WIN32
            ::SetEvent(handle);
#else
            pthread_mutex_lock(&mtx);
            pthread_cond_signal(&cond);
            signal = true;
            pthread_mutex_unlock(&mtx);
#endif
        }

        /**
        * @brief        重置事件
        * @details      重置事件，把事件由激活状态重置为非激活状态
        */
        void reset(void)
        {
#ifdef _WIN32
            ::ResetEvent(handle);
#else
            pthread_mutex_lock(&mtx);
            struct timespec tv;
            clock_gettime(CLOCK_MONOTONIC, &tv);
            pthread_cond_timedwait(&cond, &mtx, &tv);
            signal = false;
            pthread_mutex_unlock(&mtx);
#endif
        }

        /**
        * @brief        等待事件
        * @details      阻塞当前线程，等待事件被触发
        * @param[in]    timeout，超时时间
        * @return       是否成功等待到事件被触发
        */
#ifdef _WIN32
        bool wait()
        {
            return ::WaitForSingleObject(handle, INFINITE) == WAIT_OBJECT_0;
        }

        template<typename _Rep, typename _Period>
        bool Wait(std::chrono::duration<_Rep, _Period> duration)
        {
            auto timeout = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
            return ::WaitForSingleObject(cond, static_cast<DWORD>(timeout)) == WAIT_OBJECT_0;
        }
#else
        bool Wait()
        {
            bool result = false;
            pthread_mutex_lock(&mtx);
            do
            {
                if (signal)
                {
                    // 事件处于触发状态
                    result = true;
                    break;
                }
                result = pthread_cond_wait(&cond, &mtx) == 0;
            } while (false);
            if (result)
            {
                if (!is_manual)
                {
                    // 自动复位
                    signal = false;
                }
                else
                {
                    pthread_cond_signal(&cond);
                }
            }
            pthread_mutex_unlock(&mtx);
            return result;
        }


        template<typename _Rep, typename _Period>
        bool Wait(std::chrono::duration<_Rep, _Period> duration)
        {
            bool result = false;
            pthread_mutex_lock(&mtx);
            do
            {
                if (signal)
                {
                    // 事件处于触发状态
                    result = true;
                    break;
                }

                auto timeout = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
                struct timespec tv;
                clock_gettime(CLOCK_MONOTONIC, &tv);
                unsigned int timeout_sec = timeout / 1000;
                unsigned int timeout_nsec = (timeout % 1000) * 1000000;
                tv.tv_sec += timeout_sec;
                tv.tv_nsec += timeout_nsec;
                result = pthread_cond_timedwait(&cond, &mtx, &tv) == 0;
            } while (false);
            if (result)
            {
                if (!is_manual)
                {
                    // 自动复位
                    signal = false;
                }
                else
                {
                    pthread_cond_signal(&cond);
                }
            }
            pthread_mutex_unlock(&mtx);
            return result;
        }
#endif

        /**
        * @brief        移除拷贝构造函数
        */
        base_event(const base_event& other) = delete;

        /**
        * @brief        移除赋值运算符
        */
        base_event& operator=(const base_event& other) = delete;

    protected:
#ifdef _WIN32
        HANDLE              handle;     ///< 事件使用句柄
#else
        pthread_mutex_t     mtx;        ///< 保护事件操作的互斥量
        pthread_cond_t      cond;       ///< 事件使用的条件变量
        pthread_condattr_t  cond_attr;  ///< 创建条件变量使用的属性
        bool                is_manual;  ///< 标识事件是否人工重置事件， true为人工重置，false为自动重置
        bool                signal;     ///< 标识事件当前状态，true为触发状态，false为等待状态
#endif
    };

    /**
    * @brief        自动复位事件
    */
    class auto_reset_event : public base_event
    {
    public:
        /**
        * @brief        构造方法
        * @param[in]    signal，初始化事件信号
        */
        auto_reset_event(bool signal = false)
            : base_event(false, signal)
        {
        }
    };

    /**
    * @brief    手工复位事件
    */
    class manual_reset_event : public base_event
    {
    public:
        /**
        * @brief        构造方法
        * @param[in]    signal，初始化事件信号
        */
        manual_reset_event(bool signal = false)
            : base_event(true, signal)
        {
        }
    };
}
