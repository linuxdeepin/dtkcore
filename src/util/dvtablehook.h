/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef DVTABLEHOOK_H
#define DVTABLEHOOK_H

#include <dtkcore_global.h>

#include <QObject>
#include <QSet>
#include <QDebug>

#include <functional>

DCORE_BEGIN_NAMESPACE

class DVtableHook
{
public:
    static inline quintptr toQuintptr(void *ptr)
    {
        return *(quintptr*)ptr;
    }

    static inline int getVtableSize(quintptr **obj)
    {
        quintptr *begin = *obj;
        while(*begin) ++begin;
        return begin - *obj;
    }

    static int getDestructFunIndex(quintptr **obj, std::function<void(void)> destoryObjFun);
    static constexpr const QObject *getQObject(...) { return nullptr;}
    static constexpr const QObject *getQObject(const QObject *obj) { return obj;}
    static void autoCleanVtable(void *obj);
    static bool ensureVtable(void *obj, std::function<void(void)> destoryObjFun);
    static bool hasVtable(void *obj);
    static void resetVtable(void *obj);
    static quintptr resetVfptrFun(void *obj, quintptr functionOffset);
    static quintptr originalFun(void *obj, quintptr functionOffset);

    template <typename T> class OverrideDestruct : public T { ~OverrideDestruct() override;};
    template <typename List1, typename List2> struct CheckCompatibleArguments { enum { value = false }; };
    template <typename List> struct CheckCompatibleArguments<List, List> { enum { value = true }; };
    template<typename Fun1, typename Fun2>
    static bool overrideVfptrFun(const typename QtPrivate::FunctionPointer<Fun1>::Object *t1, Fun1 fun1,
                      const typename QtPrivate::FunctionPointer<Fun2>::Object *t2, Fun2 fun2)
    {
        typedef QtPrivate::FunctionPointer<Fun1> FunInfo1;
        typedef QtPrivate::FunctionPointer<Fun2> FunInfo2;
        // 检查析构函数是否为虚
        class OverrideDestruct : public FunInfo1::Object { ~OverrideDestruct() override;};

        //compilation error if the arguments does not match.
        Q_STATIC_ASSERT_X((CheckCompatibleArguments<typename FunInfo1::Arguments, typename FunInfo2::Arguments>::value),
                          "Function1 and Function2 arguments are not compatible.");
        Q_STATIC_ASSERT_X((CheckCompatibleArguments<QtPrivate::List<typename FunInfo1::ReturnType>, QtPrivate::List<typename FunInfo2::ReturnType>>::value),
                          "Function1 and Function2 return type are not compatible..");

        //! ({code}) in the form of a code is to eliminate - Wstrict - aliasing build warnings
        quintptr fun1_offset = toQuintptr(&fun1);
        quintptr fun2_offset = toQuintptr(&fun2);

        if (fun1_offset < 0 || fun1_offset > UINT_LEAST16_MAX)
            return false;

        if (!ensureVtable((void*)t1, std::bind(&_destory_helper<typename FunInfo1::Object>, t1))) {
            return false;
        }

        quintptr *vfptr_t1 = *(quintptr**)t1;
        quintptr *vfptr_t2 = *(quintptr**)t2;

        if (fun2_offset > UINT_LEAST16_MAX)
            *(vfptr_t1 + fun1_offset / sizeof(quintptr)) = fun2_offset;
        else
            *(vfptr_t1 + fun1_offset / sizeof(quintptr)) = *(vfptr_t2 + fun2_offset / sizeof(quintptr));

        return true;
    }

    template<typename Func> struct FunctionPointer { };
    template<class Obj, typename Ret, typename... Args> struct FunctionPointer<Ret (Obj::*) (Args...)>
    {
        typedef QtPrivate::List<Obj*, Args...> Arguments;
    };
    template<class Obj, typename Ret, typename... Args> struct FunctionPointer<Ret (Obj::*) (Args...) const>
    {
        typedef QtPrivate::List<Obj*, Args...> Arguments;
    };
    template<typename Fun1, typename Fun2>
    static bool overrideVfptrFun(const typename QtPrivate::FunctionPointer<Fun1>::Object *t1, Fun1 fun1, Fun2 fun2)
    {
        typedef QtPrivate::FunctionPointer<Fun1> FunInfo1;
        typedef QtPrivate::FunctionPointer<Fun2> FunInfo2;
        // 检查析构函数是否为虚
        class OverrideDestruct : public FunInfo1::Object { ~OverrideDestruct() override;};

        Q_STATIC_ASSERT(!FunInfo2::IsPointerToMemberFunction);
        //compilation error if the arguments does not match.
        Q_STATIC_ASSERT_X((CheckCompatibleArguments<typename FunctionPointer<Fun1>::Arguments, typename FunInfo2::Arguments>::value),
                          "Function1 and Function2 arguments are not compatible.");
        Q_STATIC_ASSERT_X((CheckCompatibleArguments<QtPrivate::List<typename FunInfo1::ReturnType>, QtPrivate::List<typename FunInfo2::ReturnType>>::value),
                          "Function1 and Function2 return type are not compatible..");

        //! ({code}) in the form of a code is to eliminate - Wstrict - aliasing build warnings
        quintptr fun1_offset = toQuintptr(&fun1);
        quintptr fun2_offset = toQuintptr(&fun2);

        if (fun1_offset < 0 || fun1_offset > UINT_LEAST16_MAX)
            return false;

        if (!ensureVtable((void*)t1, std::bind(&_destory_helper<typename FunInfo1::Object>, t1))) {
            return false;
        }

        quintptr *vfptr_t1 = *(quintptr**)t1;
        *(vfptr_t1 + fun1_offset / sizeof(quintptr)) = fun2_offset;

        return true;
    }

    template<typename Fun1>
    static bool resetVfptrFun(const typename QtPrivate::FunctionPointer<Fun1>::Object *obj, Fun1 fun)
    {
        return resetVfptrFun((void*)obj, toQuintptr(&fun)) > 0;
    }

    template<typename Fun>
    static Fun originalFun(const typename QtPrivate::FunctionPointer<Fun>::Object *obj, Fun fun)
    {
        quintptr o_fun = originalFun((void*)obj, toQuintptr(&fun));

        return *reinterpret_cast<Fun*>(o_fun);
    }

    template<typename Fun, typename... Args>
    static typename QtPrivate::FunctionPointer<Fun>::ReturnType
    callOriginalFun(typename QtPrivate::FunctionPointer<Fun>::Object *obj, Fun fun, Args&&... args)
    {
        quintptr fun_offset = toQuintptr(&fun);

        class _ResetVFun
        {
        public:
            ~_ResetVFun() {
                *(vfptr + offset / sizeof(quintptr)) = oldFun;
            }
            quintptr *vfptr = nullptr;
            quint16 offset = 0;
            quintptr oldFun = 0;
        };

        _ResetVFun rvf;

        rvf.vfptr = *(quintptr**)(obj);
        rvf.offset = fun_offset;
        rvf.oldFun = DVtableHook::resetVfptrFun((void*)obj, fun_offset);

        if (!rvf.oldFun) {
            qWarning() << "Reset the function failed, object:" << obj;
            abort();
        }

        // call
        return (obj->*fun)(std::forward<Args>(args)...);
    }

private:
    static bool copyVtable(quintptr **obj);
    static bool clearGhostVtable(void *obj);

    template<typename T>
    static void _destory_helper(const T *obj) {
        delete obj;
    }

    static QMap<quintptr**, quintptr*> objToOriginalVfptr;
    static QMap<void*, quintptr*> objToGhostVfptr;
    static QMap<void*, quintptr> objDestructFun;
};

DCORE_END_NAMESPACE

#endif // DVTABLEHOOK_H
