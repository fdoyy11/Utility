/**
* @file       threadsafe_queue.h
* @brief      �̰߳�ȫ������
* @details    �̰߳�ȫ�����������붨��
* @author     fdoyy11 <fdoyy11@hotmail.com>
* @date       2018/06/02
* @copyright  (c) 2018-2022 fdoyy11@hotmail.com
*/
#pragma once
#include <mutex>
#include <queue>
#include "spin_lock.h"

namespace utility
{
    /**
    * @brief        �̰߳�ȫ����
    */
    template<typename T, typename M = std::mutex>
    class threadsafe_queue
    {
    public:
        /**
        * @brief        ���췽��
        */
        threadsafe_queue() = default;

        /**
        * @brief        ��������
        */
        virtual ~threadsafe_queue() = default;

        /**
        * @brief        �������췽��
        * @param[in]    other��׼����������һ��ʵ��
        */
        threadsafe_queue(const threadsafe_queue& other)
        {
            std::lock_guard<M> guard(other.mtx);
            data = other.data;
        }

        /**
        * @brief        �ƶ����췽��
        * @param[in]    other��׼���ƶ�����һ��ʵ��
        */
        threadsafe_queue(threadsafe_queue&& other)
        {
            std::lock_guard<M> guard(other.mtx);
            data.swap(other.data);
        }

        /**
        * @brief        ���ظ�ֵ�����
        * @param[in]    other��׼����ֵ����һ��ʵ��
        * @return       ��ǰʵ��
        */
        threadsafe_queue& operator=(const threadsafe_queue& other)
        {
            std::lock(mtx, other.mtx);
            data = other.data;
            return (*this);
        }

        /**
        * @brief        �����ƶ���ֵ�����
        * @param[in]    other��׼���ƶ�����һ��ʵ��
        * @return       ��ǰʵ��
        */
        threadsafe_queue& operator=(threadsafe_queue&& other)
        {
            std::lock(mtx, other.mtx);
            data.swap(other.data);
            return (*this);
        }

        /**
        * @brief        ���
        * @param[in]    new_value����Ҫ��ӵ���Ԫ��
        */
        void push(T new_value)
        {
            std::lock_guard<M> guard(mtx);
            data.push(new_value);
        }

        /**
        * @brief        ����
        * @param[out]   value�����ӵ�Ԫ��
        * @return       �Ƿ�ɹ�����
        */
        bool try_pop(T& value)
        {
            std::lock_guard<M> guard(mtx);
            bool result = false;
            if (!data.empty())
            {
                value = data.front();
                data.pop();
                result = true;
            }
            return result;
        }

        /**
        * @brief        ���еĴ�С
        * @return       ������Ԫ�ص�����
        */
        size_t size() const
        {
            std::lock_guard<M> guard(mtx);
            return data.size();
        }

        /**
        * @brief        �Ƿ�ն���
        * @return       �����Ƿ�Ϊ��
        */
        bool empty() const
        {
            std::lock_guard<M> guard(mtx);
            return data.empty();
        }

    protected:
        std::queue<T>   data;   ///< ���ݶ���
        mutable M       mtx;    ///< ͬ����
    };

    /**
    * @brief        ʹ�����������̰߳�ȫ����
    */
    template<typename T>
    using threadsafe_spin_queue = threadsafe_queue<T, spin_lock>;
}
