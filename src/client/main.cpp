#include "group.hpp"
#include "json.hpp"
#include "public.hpp"
#include "user.hpp"

#include <arpa/inet.h>
#include <format>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <vector>
using json = nlohmann::json;

// 记录当前系统登录的用户信息
User current_user;
// 记录当前系统登录用户的好友列表信息
std::vector<User> current_friend_list;
// 记录当前系统登录用户的群组列表信息
std::vector<Group> current_group_list;

// 显示当前成功登录用户的基本信息
void showCurrentUserData();
// 接收线程
void readTaskHandler(int clientfd);
// 获取系统时间
std::string getCurrentTime();
// 主聊天页面
void mainMenu(int clientfd);

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cout << "参数错误！" << std::endl;
        std::cout << "使用方法: " << argv[0] << " <ip> <port>" << std::endl;
        return -1;
    }
    std::string ip = argv[1];
    uint16_t port = static_cast<uint16_t>(std::stoi(argv[2]));

    // 创建socket
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd == -1)
    {
        std::cerr << "socket创建失败！" << std::endl;
        return -1;
    }

    // 填写服务器信息
    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    // 连接服务器
    if (connect(clientfd, (sockaddr*)&server_addr, sizeof(server_addr)) == -1)
    {
        std::cerr << "连接服务器失败！" << std::endl;
        close(clientfd);
        return -1;
    }
    while (true)
    {
        std::cout << "------------------欢迎使用------------------" << std::endl;
        std::cout << "                  1.登录                    " << std::endl;
        std::cout << "                  2.注册                    " << std::endl;
        std::cout << "                  3.退出                    " << std::endl;
        std::cout << "-------------------------------------------" << std::endl;
        std::cout << "请选择(1-3): ";
        int choice = 0;
        std::cin >> choice;
        std::cin.ignore(); // 清除输入缓冲区的换行符

        switch (choice)
        {
        case 1:
            // 登录业务
            {
                int id = 0;
                std::string password = "";
                std::cout << "请输入用户ID：";
                std::cin >> id;
                std::cin.ignore(); // 清除输入缓冲区的换行符
                std::cout << "请输入用户密码：";
                getline(std::cin, password);

                // 组织登录json数据
                json js;
                js["msgid"] = static_cast<int>(MsgType::LOGIN_MSG);
                js["id"] = id;
                js["password"] = password;
                // 发送登录数据
                std::string request = js.dump();
                send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
                // 接收服务器响应
                std::string recv_buf;
                char buffer[1024] = {0};
                int len = recv(clientfd, buffer, sizeof(buffer), 0);
                if (len > 0)
                {
                    recv_buf = buffer;
                    json response = json::parse(recv_buf);
                    if (response["errno"].get<int>() == 0)
                    {
                        // 登录成功，记录当前用户信息
                        current_user.setId(response["id"].get<int>());
                        current_user.setName(response["name"].get<std::string>());
                        current_user.setState("online");
                        // 记录当前用户的好友列表信息
                        if (response.contains("friends"))
                        {
                            for (const auto& friend_js : response["friends"])
                            {
                                User friend_user;
                                friend_user.setId(friend_js["id"].get<int>());
                                friend_user.setName(friend_js["name"].get<std::string>());
                                friend_user.setState(friend_js["state"].get<std::string>());
                                current_friend_list.push_back(friend_user);
                            }
                        }
                        // 保存群组信息
                        if (response.contains("groups"))
                        {
                            for (const auto& group_js : response["groups"])
                            {
                                Group group;
                                group.setId(group_js["id"].get<int>());
                                group.setName(group_js["name"].get<std::string>());
                                group.setDesc(group_js["desc"].get<std::string>());
                                // 记录群组成员信息
                                for (const auto& user_js : group_js["users"])
                                {
                                    GroupUser user;
                                    user.setId(user_js["id"].get<int>());
                                    user.setName(user_js["name"].get<std::string>());
                                    user.setRole(user_js["role"].get<std::string>());
                                    user.setState(user_js["state"].get<std::string>());
                                    auto users = group.getUsers();
                                    users.push_back(user);
                                    group.setUsers(users);
                                }
                                current_group_list.push_back(group);
                            }
                        }
                        // 显示当前用户信息
                        showCurrentUserData();
                        // 离线信息
                        if (response.contains("offlinemsg"))
                        {
                            std::cout << "------------------离线消息------------------" << std::endl;
                            for (const auto& msg : response["offlinemsg"])
                            {
                                json decode_msg = json::parse(msg.get<std::string>());
                                std::cout << std::format("id : {} 时间 : {} 内容：{}",
                                                         decode_msg["from"].get<int>(),
                                                         decode_msg["time"].get<std::string>(),
                                                         decode_msg["msg"].get<std::string>())
                                          << std::endl;
                            }
                            std::cout << "-------------------------------------------" << std::endl;
                        }
                        // 启动接收线程
                        std::thread read_task(readTaskHandler, clientfd);
                        read_task.detach();
                        // 进入主聊天页面
                        mainMenu(clientfd);
                    }
                    else
                    {
                        std::cout << "登录失败！错误码："
                                  << response["errno"].get<int>()
                                  << "，错误信息："
                                  << response["errmsg"].get<std::string>()
                                  << std::endl;
                        break;
                    }
                }

                break;
            }
        case 2:
            // 注册业务
            {
                std::string name = "";
                std::string password = "";
                std::cout << "请输入用户名：";
                getline(std::cin, name);
                std::cout << "请输入用户密码：";
                getline(std::cin, password);

                // 组织注册json数据
                json js;
                js["msgid"] = static_cast<int>(MsgType::REG_MSG);
                js["name"] = name;
                js["password"] = password;
                // 发送注册数据
                std::string request = js.dump();
                send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
                std::string recv_buf;
                char buffer[1024] = {0};
                int len = recv(clientfd, buffer, sizeof(buffer), 0);
                if (len > 0)
                {
                    recv_buf = buffer;
                    json response = json::parse(recv_buf);
                    if (response["errno"].get<int>() == 0)
                    {
                        std::cout << "注册成功！您的用户ID为："
                                  << response["id"].get<int>() << "，请牢记！" << std::endl;
                    }
                    else
                    {
                        std::cout << "注册失败！" << std::endl;
                    }
                }
                break;
            }
        case 3:
            // 退出系统
            {
                close(clientfd);
                std::cout << "已退出系统，欢迎下次使用！" << std::endl;
                return 0;
            }
        default:
        {
            std::cout << "输入选项错误，请重新选择！" << std::endl;
            break;
        }
        }
    }
}
void showCurrentUserData()
{
    std::cout << "------------------当前用户信息------------------" << std::endl;
    std::cout << "用户ID：" << current_user.getId() << std::endl;
    std::cout << "用户名：" << current_user.getName() << std::endl;
    std::cout << "------------------好友列表--------------------" << std::endl;
    if (current_friend_list.empty())
    {
        std::cout << "当前无好友，请添加好友！" << std::endl;
    }
    else
    {
        for (const auto& friend_user : current_friend_list)
        {
            std::cout << friend_user.getId() << " " << friend_user.getName()
                      << " " << friend_user.getState() << std::endl;
        }
    }
    std::cout << "------------------群组列表--------------------" << std::endl;
    if (current_group_list.empty())
    {
        std::cout << "当前无群组，请创建或加入群组！" << std::endl;
    }
    else
    {
        int id = 1;
        for (const auto& group : current_group_list)
        {
            std::cout << std::format("------------------群组{}--------------------", id++) << std::endl;
            std::cout << group.getId() << " " << group.getName()
                      << " " << group.getDesc() << ' ' << std::endl;
            std::cout << "群成员列表：" << std::endl;
            for (const auto& GroupUser : group.getUsers())
            {
                std::cout << GroupUser.getId() << " " << GroupUser.getName()
                          << " " << GroupUser.getRole() << std::endl;
            }
        }
    }
    std::cout << "---------------------------------------------" << std::endl;
}

void readTaskHandler(int clientfd)
{
    char buffer[1024] = {0};
    while (true)
    {
        memset(buffer, 0, sizeof(buffer));
        int len = recv(clientfd, buffer, sizeof(buffer), 0);
        if (len > 0)
        {
            std::string recv_buf = buffer;
            json js = json::parse(recv_buf);
            // 根据消息类型进行不同处理
            int msg_type = js["msgid"].get<int>();
            switch (static_cast<MsgType>(msg_type))
            {
            case MsgType::ONE_CHAT_MSG:
            {
                std::cout << std::format("\n{} 对您说：{}",
                                         js["from"].get<int>(),
                                         js["msg"].get<std::string>())
                          << std::endl;
                break;
            }
            case MsgType::GROUP_CHAT_MSG:
            {
                std::cout << std::format("\n群[{}] {} 对大家说：{}",
                                         js["groupid"].get<int>(),
                                         js["from"].get<int>(),
                                         js["msg"].get<std::string>())
                          << std::endl;
                break;
            }
            default:
                break;
            }
        }
    }
}

std::string getCurrentTime()
{
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&now_c));
    return std::string(buffer);
}

void mainMenu(int clientfd)
{
    while (true)
    {
        std::cout << "------------------主菜单------------------" << std::endl;
        std::cout << "                  1.聊天                    " << std::endl;
        std::cout << "                  2.加好友                  " << std::endl;
        std::cout << "                  3.创建群组                " << std::endl;
        std::cout << "                  4.加入群组                " << std::endl;
        std::cout << "                  5.群聊                    " << std::endl;
        std::cout << "                  6.退出                    " << std::endl;
        std::cout << "-------------------------------------------" << std::endl;
        std::cout << "请选择(1-6): ";
        int choice = 0;
        std::cin >> choice;
        std::cin.ignore(); // 清除输入缓冲区的换行符

        switch (choice)
        {
        case 1:
            // 聊天业务
            {
                int to_id = 0;
                std::string chat_msg = "";
                std::cout << "请输入聊天对象的ID：";
                std::cin >> to_id;
                std::cin.ignore(); // 清除输入缓冲区的换行符
                std::cout << "请输入聊天内容：";
                getline(std::cin, chat_msg);

                // 组织聊天json数据
                json js;
                js["msgid"] = static_cast<int>(MsgType::ONE_CHAT_MSG);
                js["from"] = current_user.getId();
                js["to"] = to_id;
                js["msg"] = chat_msg;
                js["time"] = getCurrentTime();
                // 发送聊天数据
                std::string request = js.dump();
                send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
                break;
            }
        case 2:
            // 添加好友业务
            {
                int friend_id = 0;
                std::cout << "请输入要添加的好友ID：";
                std::cin >> friend_id;
                std::cin.ignore(); // 清除输入缓冲区的换行符

                // 组织添加好友json数据
                json js;
                js["msgid"] = static_cast<int>(MsgType::ADD_FRIEND_MSG);
                js["id"] = current_user.getId();
                js["friendid"] = friend_id;
                // 发送添加好友数据
                std::string request = js.dump();
                send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
                break;
            }
        case 3:
            // 创建群组业务
            {
                std::string group_name = "";
                std::string group_desc = "";
                std::cout << "请输入群组名称：";
                getline(std::cin, group_name);
                std::cout << "请输入群组描述：";
                getline(std::cin, group_desc);
                // 组织创建群组json数据
                json js;
                js["msgid"] = static_cast<int>(MsgType::CREATE_GROUP_MSG);
                js["id"] = current_user.getId();
                js["groupname"] = group_name;
                js["groupdesc"] = group_desc;
                // 发送创建群组数据
                std::string request = js.dump();
                send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
                break;
            }
        case 4:
            // 加入群组业务
            {
                int group_id = 0;
                std::cout << "请输入要加入的群组ID：";
                std::cin >> group_id;
                std::cin.ignore(); // 清除输入缓冲区的换行符
                json js;
                js["msgid"] = static_cast<int>(MsgType::ADD_GROUP_MSG);
                js["id"] = current_user.getId();
                js["groupid"] = group_id;
                // 发送加入群组数据
                std::string request = js.dump();
                send(clientfd, request.c_str(), strlen(request.c_str()) + 1,0);
                break;
            }
        case 5:
            // 群聊业务
            {
                int group_id = 0;
                std::string chat_msg = "";
                std::cout << "请输入群组ID：";
                std::cin >> group_id;
                std::cin.ignore(); // 清除输入缓冲区的换行符
                std::cout << "请输入聊天内容：";
                getline(std::cin, chat_msg);
                // 组织群聊json数据
                json js;
                js["msgid"] = static_cast<int>(MsgType::GROUP_CHAT_MSG);
                js["from"] = current_user.getId();
                js["groupid"] = group_id;
                js["msg"] = chat_msg;
                js["time"] = getCurrentTime();
                // 发送群聊数据
                std::string request = js.dump();
                send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
                break;
            }
        case 6:
            // 退出主菜单
            {
                std::cout << "已退出主菜单，欢迎下次使用！" << std::endl;

                return;
            }
        default:
            {
                std::cout << "输入选项错误，请重新选择！" << std::endl;
                break;
            }
        }
    }
}