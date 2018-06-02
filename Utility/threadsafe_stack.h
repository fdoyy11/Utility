/**
* @file       threadsafe_stack.h
* @brief      �̰߳�ȫ��ջ��
* @details    �̰߳�ȫ��ջ�������붨��
* @author     fdoyy11 <fdoyy11@hotmail.com>
* @date       2018/06/02
* @copyright  (c) 2018-2022 fdoyy11@hotmail.com
*/
#pragma once
#include <mutex>
#include <stack>
#include "spin_lock.h"

namespace utility
{
    /**
    * @brief        �̰߳�ȫջ
    */
    template<typename T, typename M = std::mutex>
    class threadsafe_stack
    {
    public:
        /**
        * @brief        ���췽��
        */
        threadsafe_stack() = default;

        /**
        * @brief        ��������
        */
        virtual ~threadsafe_stack() = default;

        /**
        * @brief        �������췽��
        * @param[in]    other��׼����������һ��ʵ��
        */
        threadsafe_stack(const threadsafe_stack& other)
        {
            std::lock_guard<M> guard(other.mtx);
            data = other.data;
        }

        /**
        * @brief        �ƶ����췽��
        * @param[in]    other��׼���ƶ�����һ��ʵ��
        */
        threadsafe_stack(threadsafe_stack&& other)
        {
            std::lock_guard<M> guard(other.mtx);
            data.swap(other.data);
        }

        /**
        * @brief        ���ظ�ֵ�����
        * @param[in]    other��׼����ֵ����һ��ʵ��
        * @return       ��ǰʵ��
        */
        threadsafe_stack& operator=(const threadsafe_stack& other)
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
        threadsafe_stack& operator=(threadsafe_stack&& other)
        {
            std::lock(mtx, other.mtx);
            data.swap(other.data);
            return (*this);
        }

        /**
        * @brief        ��ջ
        * @param[in]    newValue��׼����ջ��Ԫ��
        */
        void push(T new_value)
        {
            std::lock_guard<M> guard(mtx);
            data.push(new_value);
        }

        /**
        * @brief        ��ջ
        * @param[out]   value�����ճ�ջ��Ԫ��
        * @return       �Ƿ�ӵ��Ԫ�س�ջ
        */
        bool try_pop(T& value)
        {
            std::lock_guard<M> guard(mtx);
            bool result = false;
            if (!data.empty())
            {
                value = data.top();
                data.pop();
                result = true;
            }
            return result;
        }

        /**
        * @brief        ջ�Ĵ�С
        * @return       ջ��Ԫ�ص�����
        */
        size_t size() const
        {
            std::lock_guard<M> guard(mtx);
            return data.size();
        }

        /**
        * @brief        �Ƿ��ջ
        * @return       ջ�Ƿ�Ϊ��
        */
        bool empty() const
        {
            std::lock_guard<M> guard(mtx);
            return data.empty();
        }

    protected:
        std::stack<T>   data;   ///< ��������
        mutable M       mtx;    ///< ͬ����
    };

    /**
    * @brief        ʹ�����������̰߳�ȫջ
    */
    template<typename T>
    using threadsafe_spin_stack = threadsafe_stack<T, spin_lock>;
}
