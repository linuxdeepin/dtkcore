/*
 * Copyright (C) 2016 ~ 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DSINGLETON_H
#define DSINGLETON_H

#include "dtkcore_global.h"

DCORE_BEGIN_NAMESPACE

/*!
  a simple singleton template for std c++ 11 or later.
  
  example:

   \code
   class ExampleSingleton : public QObject, public Dtk::DSingleton<ExampleSingleton>
   {
       Q_OBJECT
       friend class DSingleton<ExampleSingleton>;
   };
   \endcode

  \note: for Qt, "public DSingleton<ExampleSingleton>" must be after QObject.
 */

/*!
  通过c++11的特性实现的单例模板
  
  使用示例:

```
   class ExampleSingleton : public QObject, public Dtk::DSingleton<ExampleSingleton>
   {
       Q_OBJECT
       friend class DSingleton<ExampleSingleton>;
   };
```

  \note 对于Qt程序 public DSingleton<ExampleSingleton>" 必须在卸载QObject后面出现。
 */

template <class T>
class LIBDTKCORESHARED_EXPORT DSingleton
{
public:
    QT_DEPRECATED_X("Use ref")
    static inline T *instance()
    {
        static T  *_instance = new T;
        return _instance;
    }

    static T& ref()
    {
        static T instance;
        return instance;
    }

    DSingleton(T&&) = delete;
    DSingleton(const T&) = delete;
    void operator= (const T&) = delete;

protected:
    DSingleton() = default;
    virtual ~DSingleton() = default;
};

DCORE_END_NAMESPACE

#endif // DSINGLETON_H
