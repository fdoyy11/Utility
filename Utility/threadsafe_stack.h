/**
* @file       threadsafe_stack.h
* @brief      线程安全堆栈类
* @details    线程安全堆栈类声明与定义
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
    * @brief        线程安全栈
    */
    template<typename T, typename M = std::mutex>
    class threadsafe_stack
    {
    public:
        /**
        * @brief        构造方法
        */
        threadsafe_stack() = default;

        /**
        * @brief        析构方法
        */
        virtual ~threadsafe_stack() = default;

        /**
        * @brief        拷贝构造方法
        * @param[in]    other，准备拷贝的另一个实例
        */
        threadsafe_stack(const threadsafe_stack& other)
        {
            std::lock_guard<M> guard(other.mtx);
            data = other.data;
        }

        /**
        * @brief        移动构造方法
        * @param[in]    other，准备移动的另一个实例
        */
        threadsafe_stack(threadsafe_stack&& other)
        {
            std::lock_guard<M> guard(other.mtx);
            data.swap(other.data);
        }

        /**
        * @brief        重载赋值运算符
        * @param[in]    other，准备赋值的另一个实例
        * @return       当前实例
        */
        threadsafe_stack& operator=(const threadsafe_stack& other)
        {
            std::lock(mtx, other.mtx);
            data = other.data;
            return (*this);
        }

        /**
        * @brief        重载移动赋值运算符
        * @param[in]    other，准备移动的另一个实例
        * @return       当前实例
        */
        threadsafe_stack& operator=(threadsafe_stack&& other)
        {
            std::lock(mtx, other.mtx);
            data.swap(other.data);
            return (*this);
        }

        /**
        * @brief        入栈
        * @param[in]    newValue，准备入栈的元素
        */
        void push(T new_value)
        {
            std::lock_guard<M> guard(mtx);
            data.push(new_value);
        }

        /**
        * @brief        出栈
        * @param[out]   value，接收出栈的元素
        * @return       是否拥有元素出栈
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
        * @brief        栈的大小
        * @return       栈中元素的数量
        */
        size_t size() const
        {
            std::lock_guard<M> guard(mtx);
            return data.size();
        }

        /**
        * @brief        是否空栈
        * @return       栈是否为空
        */
        bool empty() const
        {
            std::lock_guard<M> guard(mtx);
            return data.empty();
        }

    protected:
        std::stack<T>   data;   ///< 保存数据
        mutable M       mtx;    ///< 同步锁
    };

    /**
    * @brief        使用自旋锁的线程安全栈
    */
    template<typename T>
    using threadsafe_spin_stack = threadsafe_stack<T, spin_lock>;
}
