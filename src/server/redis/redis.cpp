#include "redis.hpp"
#include <iostream>

Redis::Redis()
    : publish_context_(nullptr), subscribe_context_(nullptr)
{
}

Redis::~Redis()
{
    if (publish_context_ != nullptr)
    {
        redisFree(publish_context_);
    }
    if (subscribe_context_ != nullptr)
    {
        redisFree(subscribe_context_);
    }
}

bool Redis::connect()
{
    // 连接发布订阅对象
    publish_context_ = redisConnect("127.0.0.1", 6379);
    if (publish_context_ == nullptr)
    {
        if (publish_context_)
        {
            std::cerr << "Redis publish connect error: " << publish_context_->errstr;
        }
        else
        {
            std::cerr << "Redis publish connect error: can't allocate redis context";
        }
        return false;
    }

    // 连接订阅对象
    subscribe_context_ = redisConnect("127.0.0.1", 6379);
    if (subscribe_context_ == nullptr)
    {
        if (subscribe_context_)
        {
            std::cerr << "Redis subscribe connect error: " << subscribe_context_->errstr;
        }
        else
        {
            std::cerr << "Redis subscribe connect error: can't allocate redis context";
        }
        return false;
    }
    // 启动一个线程，专门接受订阅的消息
    std::thread t([&]()
                  { observer_channel_message(); });
    t.detach();
    std::cout << "Redis connect success!" << std::endl;
    return true;
}

bool Redis::publish(int channel, std::string message)
{
    redisReply* reply = (redisReply*)redisCommand(publish_context_, "PUBLISH %d %s", channel, message.c_str());
    if (reply == nullptr)
    {
        std::cerr << "Redis publish command error!" << std::endl;
        return false;
    }
    freeReplyObject(reply);
    return true;
}

bool Redis::subscribe(int channel)
{
    // 只是订阅，不让线程被阻塞
    if(REDIS_ERR==redisAppendCommand(subscribe_context_, "SUBSCRIBE %d", channel))
    {
        std::cerr << "Redis subscribe command error!" << std::endl;
        return false;
    }
    int done=0;
    while(!done)
    {
        if(REDIS_ERR==redisBufferWrite(subscribe_context_,&done))
        {
            std::cerr << "Redis subscribe command error!" << std::endl;
            return false;
        }
    }
    return true;
}

// 向redis指定的通道unsubscribe取消订阅消息
bool Redis::unsubscribe(int channel)
{
    if (REDIS_ERR == redisAppendCommand(this->subscribe_context_, "UNSUBSCRIBE %d", channel))
    {
        std::cerr << "unsubscribe command failed!" << std::endl;
        return false;
    }
    // redisBufferWrite可以循环发送缓冲区，直到缓冲区数据发送完毕（done被置为1）
    int done = 0;
    while (!done)
    {
        if (REDIS_ERR == redisBufferWrite(this->subscribe_context_, &done))
        {
            std::cerr << "unsubscribe command failed!" << std::endl;
            return false;
        }
    }
    return true;
}

void Redis::observer_channel_message()
{
    redisReply* reply = nullptr;
    while (redisGetReply(subscribe_context_, (void**)&reply) == REDIS_OK)
    {
        // 订阅收到的消息是一个数组
        if (reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr)
        {
            int channel = std::stoi(reply->element[1]->str);
            std::string message = reply->element[2]->str;
            // 回调上报给service
            if (notify_message_handler_)
            {
                notify_message_handler_(channel, message);
            }
        }
        freeReplyObject(reply);
    }
    std::cerr << "Redis subscribe disconnected!" << std::endl;
}

void Redis::init_notify_handler(std::function<void(int, std::string)> handler)
{
    notify_message_handler_ = handler;
}

        