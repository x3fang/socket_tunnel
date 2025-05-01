#include <bits/stdc++.h>
#include <windows.h>
#include <string>
#include <queue>
#include <vector>
#include <locale>
#include <codecvt>
// #include "include\fliter.h"
using namespace std;
// 使用示例
int a(int &a)
{
      std::shared_ptr<int> p(&a);
}
int main()
{
      int ap = 10;
      a(ap);
      ap = 20;
      cout << ap;
      return 0;
}