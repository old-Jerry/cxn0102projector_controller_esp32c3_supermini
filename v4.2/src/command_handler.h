#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <Arduino.h>

class CommandHandler {
public:
    CommandHandler();
    
    // 根据索引发送预定义命令
    void sendCommandByIndex(int index);
    
    // 发送自定义命令
    void sendCustomCommand(const char* cmd);
    
    // 获取命令总数
    int getCommandCount() const;
    
    // 获取命令字符串
    const char* getCommand(int index) const;
    
private:
    static const char* commands[];
    static const int commandCount;
};

#endif // COMMAND_HANDLER_H
