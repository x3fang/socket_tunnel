#include <windows.h>
#include <string>
#include <queue>
#include <vector>
#include <locale>
#include <codecvt>

// 使用示例
int main()
{
      std::vector<std::string> files = TraverseFiles(L".", L"dll");
      for (const auto &path : files)
      {
            printf("%s\n", path.c_str());
      }
      return 0;
}