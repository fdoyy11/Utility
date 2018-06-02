/**
* @file       threadsafe_queue.h
* @brief      线程安全队列类
* @details    线程安全队列类声明与定义
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
    * @brief        线程安全队列
    */
    template<typename T, typename M = std::mutex>
    class threadsafe_queue
    {
    public:
        /**
        * @brief        构造方法
        */
        threadsafe_queue() = default;

        /**
        * @brief        析构方法
        */
        virtual ~threadsafe_queue() = default;

        /**
        * @brief        拷贝构造方法
        * @param[in]    other，准备拷贝的另一个实例
        */
        threadsafe_queue(const threadsafe_queue& other)
        {
            std::lock_guard<M> guard(other.mtx);
            data = other.data;
        }

        /**
        * @brief        移动构造方法
        * @param[in]    other，准备移动的另一个实例
        */
        threadsafe_queue(threadsafe_queue&& other)
        {
            std::lock_guard<M> guard(other.mtx);
            data.swap(other.data);
        }

        /**
        * @brief        重载赋值运算符
        * @param[in]    other，准备赋值的另一个实例
        * @return       当前实例
        */
        threadsafe_queue& operator=(const threadsafe_queue& other)
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
        threadsafe_queue& operator=(threadsafe_queue&& other)
        {
            std::lock(mtx, other.mtx);
            data.swap(other.data);
            return (*this);
        }

        /**
        * @brief        入队
        * @param[in]    new_value，需要入队的新元素
        */
        void push(T new_value)
        {
            std::lock_guard<M> guard(mtx);
            data.push(new_value);
        }

        /**
        * @brief        出队
        * @param[out]   value，出队的元素
        * @return       是否成功出队
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
        * @brief        队列的大小
        * @return       队列中元素的数量
        */
        size_t size() const
        {
            std::lock_guard<M> guard(mtx);
            return data.size();
        }

        /**
        * @brief        是否空队列
        * @return       队列是否为空
        */
        bool empty() const
        {
            std::lock_guard<M> guard(mtx);
            return data.empty();
        }

    protected:
        std::queue<T>   data;   ///< 数据队列
        mutable M       mtx;    ///< 同步锁
    };

    /**
    * @brief        使用自旋锁的线程安全队列
    */
    template<typename T>
    using threadsafe_spin_queue = threadsafe_queue<T, spin_lock>;
}
