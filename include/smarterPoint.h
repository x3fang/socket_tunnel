#ifndef SMATERPOINT_H
#define SMATERPOINT_H
#include <type_traits>
#include <cstddef>

namespace SmarterPoint
{
      template <typename T>
      class smarterPoint
      {
      private:
            T *p = nullptr;
            int *quote = nullptr;

      public:
            smarterPoint(T *ptr = nullptr) : p(ptr), quote(new int((ptr ? 1 : 0))) {}

            smarterPoint(const smarterPoint &other) : p(other.p), quote(other.quote)
            {
                  if (quote)
                        ++(*quote);
            }

            smarterPoint &operator=(const smarterPoint &other)
            {
                  if (this != &other)
                  {
                        if (quote && --(*quote) == 0)
                        {
                              delete p;
                              delete quote;
                        }
                        p = other.p;
                        quote = other.quote;
                        if (quote)
                              ++(*quote);
                  }
                  return *this;
            }

            ~smarterPoint()
            {
                  if (quote && --(*quote) == 0)
                  {
                        delete p;
                        delete quote;
                  }
            }

            // 启用 operator-> 仅当 T 非 void
            template <typename U = T>
            typename std::enable_if<!std::is_void_v<U>, U *>::type
            operator->() const noexcept
            {
                  return p;
            }

            // 启用 operator* 仅当 T 非 void
            template <typename U = T>
            typename std::enable_if<!std::is_void_v<U>, U &>::type
            operator*() const noexcept
            {
                  return *p;
            }

            T *get() const noexcept
            {
                  return p;
            }
      };

      // 比较运算符等保持原样
      template <typename First, typename Second>
      inline bool operator==(const smarterPoint<First> &p1, const smarterPoint<Second> &p2)
      {
            return p1.get() == p2.get();
      }
}
#endif