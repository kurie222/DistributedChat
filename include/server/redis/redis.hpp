#ifndef REDIS_HPP
#define REDIS_HPP

#include <functional>
#include <hiredis/hiredis.h>
#include <string>
#include <thread>

class Redis
{
public:
    Redis();
    ~Redis();

    // 连接redis服务器
    bool connect();

    // 向redis指定通道channel发布消息
    bool publish(int channel, std::string message);

    // 向redis指定通道subscribe订阅消息
    bool subscribe(int channel);

    // 向redis指定通道unsubscribe取消订阅消息
    bool unsubscribe(int channel);

    // 在独立线程中接收订阅的消息
    void observer_channel_message();

    // 向service上报订阅得到的消息
    void init_notify_handler(std::function<void(int, std::string)> handler);

private:
    // 发布订阅对象
    redisContext* publish_context_;
    // 订阅对象
    redisContext* subscribe_context_;
    // 给service上报消息的回调函数
    std::function<void(int, std::string)> notify_message_handler_;
};

#endif // REDIS_HPP