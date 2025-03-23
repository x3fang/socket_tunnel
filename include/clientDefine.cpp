#include "clientDefine.h"
bool send_message(SOCKET sock, const std::string &message)
{
      std::ostringstream oss;
      oss << message.size() << "\r\n\r\n\r\n\r\n\r\n"
          << message; // 构建消息，包含长度和实际数据
      std::string formatted_message = oss.str();

      int total_sent = 0;
      int message_length = formatted_message.size();
      const char *data = formatted_message.c_str();

      while (total_sent < message_length)
      {
            int bytes_sent = send(sock, data + total_sent, message_length - total_sent, 0);
            if (bytes_sent == SOCKET_ERROR)
            {
                  return false; // 发送失败
            }
            total_sent += bytes_sent;
      }
      return true; // 发送成功
}
bool receive_message(SOCKET sock, std::string &message)
{
      std::string length_str;
      char buffer[16384] = {0};
      int received;

      // 首先读取长度部分，直到接收到 \r\n
      while (true)
      {
            received = recv(sock, buffer, 1, 0); // 每次读取一个字节
            if (received <= 0)
            {
                  return false; // 连接断开或读取出错
            }
            if (buffer[0] == '\r')
            {
                  // 继续读取\n
                  received = recv(sock, buffer, 1, 0);
                  if (received <= 0 || buffer[0] != '\n')
                  {
                        return false; // 格式错误
                  }

                  for (int i = 1; i <= 4; i++)
                  {
                        received = recv(sock, buffer, 1, 0);
                        if (received <= 0 || buffer[0] != '\r')
                        {
                              return false; // 格式错误
                        }
                        received = recv(sock, buffer, 1, 0);
                        if (received <= 0 || buffer[0] != '\n')
                        {
                              return false; // 格式错误
                        }
                  }
                  break; // 读取到 \r\n，退出循环
            }
            length_str += buffer[0];
      }

      int data_length = std::stoi(length_str); // 转换长度字符串为整数
      message.resize(data_length);

      int total_received = 0;
      while (total_received < data_length)
      {
            received = recv(sock, &message[total_received], data_length - total_received, 0);
            if (received <= 0)
            {
                  return false; // 连接断开或读取出错
            }
            total_received += received;
      }

      return true; // 接收成功
}