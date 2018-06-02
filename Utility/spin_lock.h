/**
* @file       spin_lock.h
* @brief      自旋锁类
* @details    自旋锁类声明与定义
* @author     fdoyy11 <fdoyy11@hotmail.com>
* @date       2018/06/02
* @copyright  (c) 2018-2022 fdoyy11@hotmail.com
*/
#pragma once
#include <atomic>

namespace utility
{
    /**
    * @brief        自旋锁
    */
    class spin_lock
    {
    public:
        /**
        * @brief        构造函数
        */
        spin_lock()
        {
            flag.clear(std::memory_order_release);
        }

        /**
        * @brief        上锁
        */
        void lock()
        {
            while (flag.test_and_set(std::memory_order_acquire));
        }

        /**
        * @brief        解锁
        */
        void unlock()
        {
            flag.clear(std::memory_order_release);
        }

    private:
        std::atomic_flag flag;      ///< 原子标识
    };
}
