/**
* @file       stop_watch.h
* @brief      时间测量类
* @details    时间测量类声明与定义
* @author     fdoyy11 <fdoyy11@hotmail.com>
* @date       2018/12/08
* @copyright  (c) 2018-2022 fdoyy11@hotmail.com
*/
#pragma once
#include <chrono>
#include <stdexcept>

namespace utility
{
    /**
    * @brief        运行时间测量类
    */
    class stop_watch
    {
    public:
        /**
        * @brief      构造方法
        */
        stop_watch()
            : _ts(std::chrono::high_resolution_clock::time_point::min())
            , _isRunning(false)
            , _elapsed(std::chrono::high_resolution_clock::duration::zero())
        {
        }

        /**
        * @brief        初始化新的CStopWatch实例并开始测量运行时间
        * @return       新的CStopWatch实例
        */
        static stop_watch start_new()
        {
            stop_watch sw;
            sw.start();
            return std::move(sw);
        }

        /**
        * @brief        获取当前实例测量得出的总运行时间
        * @return       当前实例测量的总运行时间
        */
        std::chrono::nanoseconds elapsed()
        {
            return _elapsed;
        }

        /**
        * @brief        获取当前实例测量得出的总运行时间（以毫秒为单位）
        * @return       当前实例测量的总运行时间
        */
        int64_t elapsed_milliseconds()
        {
            return std::chrono::duration_cast<std::chrono::milliseconds>(_elapsed).count();
        }

        /**
        * @brief        获取测量器是否正在计时
        * @return       true为正在计时，false为没有计时
        */
        bool is_running()
        {
            return _isRunning;
        }

        /**
        * @brief        停止时间间隔测量，并将运行时间重置为零
        */
        void reset()
        {
            _ts = std::chrono::high_resolution_clock::time_point::min();
            _elapsed = std::chrono::high_resolution_clock::duration::zero();
            _isRunning = false;
        }

        /**
        * @bref         停止时间间隔测量，将运行时间重置为零，然后开始测量运行时间
        */
        void restart()
        {
            _elapsed = std::chrono::high_resolution_clock::duration::zero();
            _ts = std::chrono::high_resolution_clock::now();
            _isRunning = true;
        }

        /**
        * @brief        开始时间测量
        */
        void start()
        {
            if (_isRunning)
            {
                throw std::logic_error("The instance is started.");
            }
            _ts = std::chrono::high_resolution_clock::now();
            _isRunning = true;
        }

        /**
        * @brief        停止时间测量
        */
        void stop()
        {
            if (!_isRunning)
            {
                throw std::logic_error("The instance is stopped.");
            }
            _elapsed += (std::chrono::high_resolution_clock::now() - _ts);
            _isRunning = false;
        }

    protected:
        std::chrono::high_resolution_clock::time_point  _ts;        ///< 测量器启动时间
        bool                                            _isRunning; ///< 是否正在运行
        std::chrono::nanoseconds                        _elapsed;   ///< 测量累计的时间
    };
}
