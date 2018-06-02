/**
* @file       event.h
* @brief      �¼���
* @details    �¼��������붨��
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
    * @brief        �¼�����
    */
    class base_event
    {
    public:
        /**
        * @brief        ���췽��
        * @param[in]    is_manual���Ƿ񴴽�Ϊ�ֹ������¼�
        * @param[in]    signal����ʼ���¼�����״̬��trueΪ����״̬��
        *               falseΪ�Ǵ���״̬
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
        * @brief        ��������
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
        * @brief        ����С���������
        * @details      ���¼�������ΪHANDLEʹ����Ӧ���ڵȴ�����
        */
        operator HANDLE(void)
        {
            return handle;
        }
#else
        /**
        * @brief        �㲥
        * @details      �������еȴ����¼����߳�
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
        * @brief        �����¼�
        * @details      �����¼�������ȴ����¼����̣߳���Ϊ�Զ������¼���ֻ����
        *               һ�����ڵȴ����¼����̣߳���Ϊ�˹������¼��򼤻���������
        *               ���ڵȴ����¼����߳�
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
        * @brief        �����¼�
        * @details      �����¼������¼��ɼ���״̬����Ϊ�Ǽ���״̬
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
        * @brief        �ȴ��¼�
        * @details      ������ǰ�̣߳��ȴ��¼�������
        * @param[in]    timeout����ʱʱ��
        * @return       �Ƿ�ɹ��ȴ����¼�������
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
                    // �¼����ڴ���״̬
                    result = true;
                    break;
                }
                result = pthread_cond_wait(&cond, &mtx) == 0;
            } while (false);
            if (result)
            {
                if (!is_manual)
                {
                    // �Զ���λ
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
                    // �¼����ڴ���״̬
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
                    // �Զ���λ
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
        * @brief        �Ƴ��������캯��
        */
        base_event(const base_event& other) = delete;

        /**
        * @brief        �Ƴ���ֵ�����
        */
        base_event& operator=(const base_event& other) = delete;

    protected:
#ifdef _WIN32
        HANDLE              handle;     ///< �¼�ʹ�þ��
#else
        pthread_mutex_t     mtx;        ///< �����¼������Ļ�����
        pthread_cond_t      cond;       ///< �¼�ʹ�õ���������
        pthread_condattr_t  cond_attr;  ///< ������������ʹ�õ�����
        bool                is_manual;  ///< ��ʶ�¼��Ƿ��˹������¼��� trueΪ�˹����ã�falseΪ�Զ�����
        bool                signal;     ///< ��ʶ�¼���ǰ״̬��trueΪ����״̬��falseΪ�ȴ�״̬
#endif
    };

    /**
    * @brief        �Զ���λ�¼�
    */
    class auto_reset_event : public base_event
    {
    public:
        /**
        * @brief        ���췽��
        * @param[in]    signal����ʼ���¼��ź�
        */
        auto_reset_event(bool signal = false)
            : base_event(false, signal)
        {
        }
    };

    /**
    * @brief    �ֹ���λ�¼�
    */
    class manual_reset_event : public base_event
    {
    public:
        /**
        * @brief        ���췽��
        * @param[in]    signal����ʼ���¼��ź�
        */
        manual_reset_event(bool signal = false)
            : base_event(true, signal)
        {
        }
    };
}
