/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
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
#ifndef DVTABLEHOOK_H
#define DVTABLEHOOK_H

#include <dtkcore_global.h>

#include <QObject>
#include <QSet>
#include <QDebug>

#include <functional>
#include <type_traits>

DCORE_BEGIN_NAMESPACE

class LIBDTKCORESHARED_EXPORT DVtableHook
{
public:
    static inline quintptr toQuintptr(const void *ptr)
    {
        return *(quintptr*)ptr;
    }

    static inline int getVtableSize(quintptr **obj)
    {
        quintptr *begin = *obj;
        while(*begin) ++begin;
        return begin - *obj;
    }

    static inline quintptr *getVtableOfObject(const void *obj)
    {
        return *(quintptr**)obj;
    }

    template <typename T>
    static quintptr *getVtableOfClass()
    {
        QByteArray vtable_symbol(typeid(T).name());
        vtable_symbol.prepend("_ZTV");

        quintptr *vfptr_t1 = reinterpret_cast<quintptr*>(resolve(vtable_symbol.constData()));

        if (!vfptr_t1)
            return nullptr;

        // symbol address + 2 * sizeof(quintptr) = virtal table start address
        return vfptr_t1 + 2;
    }

    static int getDestructFunIndex(quintptr **obj, std::function<void(void)> destoryObjFun);
    static constexpr const QObject *getQObject(...) { return nullptr;}
    static constexpr const QObject *getQObject(const QObject *obj) { return obj;}
    static void autoCleanVtable(const void *obj);
    static bool ensureVtable(const void *obj, std::function<void(void)> destoryObjFun);
    static bool hasVtable(const void *obj);
    static void resetVtable(const void *obj);
    static quintptr resetVfptrFun(const void *obj, quintptr functionOffset);
    static quintptr originalFun(const void *obj, quintptr functionOffset);
    static bool forceWriteMemory(void *adr, const void *data, size_t length);
    static QFunctionPointer resolve(const char *symbol);

    template <typename T> class OverrideDestruct : public T { ~OverrideDestruct() override;};
    template <typename List1, typename List2> struct CheckCompatibleArguments { enum { value = false }; };
    template <typename List> struct CheckCompatibleArguments<List, List> { enum { value = true }; };

    template<typename Fun1, typename Fun2>
    static bool overrideVfptrFun(quintptr *vfptr_t1, Fun1 fun1, quintptr *vfptr_t2, Fun2 fun2, bool forceWrite)
    {
        typedef QtPrivate::FunctionPointer<Fun1> FunInfo1;
        typedef QtPrivate::FunctionPointer<Fun2> FunInfo2;

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

        quintptr *vfun = vfptr_t1 + fun1_offset / sizeof(quintptr);

        // if the fun2 is not virtual function
        if (fun2_offset <= UINT_LEAST16_MAX) {
            fun2_offset = *(vfptr_t2 + fun2_offset / sizeof(quintptr));
        }

        if (forceWrite)
            return forceWriteMemory(vfun, &fun2_offset, sizeof(fun2_offset));

        *vfun = fun2_offset;

        return true;
    }

    template<typename Fun1, typename Fun2>
    static bool overrideVfptrFun(const typename QtPrivate::FunctionPointer<Fun1>::Object *t1, Fun1 fun1,
                      const typename QtPrivate::FunctionPointer<Fun2>::Object *t2, Fun2 fun2)
    {
        typedef QtPrivate::FunctionPointer<Fun1> FunInfo1;
        // 检查析构函数是否为虚
        class OverrideDestruct : public FunInfo1::Object { ~OverrideDestruct() override;};

        if (!ensureVtable((void*)t1, std::bind(&_destory_helper<typename FunInfo1::Object>, t1))) {
            return false;
        }

        quintptr *vfptr_t1 = getVtableOfObject(t1);
        quintptr *vfptr_t2 = getVtableOfObject(t2);

        bool ok = overrideVfptrFun(vfptr_t1, fun1, vfptr_t2, fun2, false);

        if (!ok) {
            // 恢复旧环境
            resetVtable(t1);
        }

        return ok;
    }

    template<typename Class, typename Fun1, typename Fun2>
    static bool overrideVfptrFun(Fun1 fun1, const typename QtPrivate::FunctionPointer<Fun2>::Object *t2, Fun2 fun2)
    {
        quintptr *vfptr_t1 = getVtableOfClass<Class>();

        if (!vfptr_t1) {
            abort();
        }

        quintptr *vfptr_t2 = getVtableOfObject(t2);

        return overrideVfptrFun(vfptr_t1, fun1, vfptr_t2, fun2, true);
    }

    template<typename Fun1, typename Fun2>
    static bool overrideVfptrFun(Fun1 fun1, const typename QtPrivate::FunctionPointer<Fun2>::Object *t2, Fun2 fun2)
    {
        typedef QtPrivate::FunctionPointer<Fun1> FunInfo1;
        return overrideVfptrFun<typename FunInfo1::Object>(fun1, t2, fun2);
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
    static typename std::enable_if<QtPrivate::FunctionPointer<Fun2>::ArgumentCount >= 0, bool>::type
            overrideVfptrFun(quintptr *vfptr_t1, Fun1 fun1, Fun2 fun2, bool forceWrite)
    {
        typedef QtPrivate::FunctionPointer<Fun1> FunInfo1;
        typedef QtPrivate::FunctionPointer<Fun2> FunInfo2;

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

        quintptr *vfun = vfptr_t1 + fun1_offset / sizeof(quintptr);

        if (forceWrite)
            return forceWriteMemory(vfun, &fun2_offset, sizeof(fun2_offset));

        *vfun = fun2_offset;

        return true;
    }

    template<typename StdFun, typename Func> struct StdFunWrap {};
    template<typename StdFun, class Obj, typename Ret, typename... Args>
    struct StdFunWrap<StdFun, Ret (Obj::*) (Args...)> {
        typedef std::function<Ret(Obj*, Args...)> StdFunType;
        static inline StdFunType fun(StdFunType f, bool check = true) {
            static StdFunType fun = f;
            static bool initialized = false;
            if (initialized && check) {
                qWarning("The StdFunWrap is dirty! Don't use std::bind(use lambda functions).");
            }
            initialized = true;
            return fun;
        }
        static Ret call(Obj *o, Args... args) {
            return fun(call, false)(o, std::forward<Args>(args)...);
        }
    };
    template<typename StdFun, class Obj, typename Ret, typename... Args>
    struct StdFunWrap<StdFun, Ret (Obj::*) (Args...) const> : StdFunWrap<StdFun, Ret (Obj::*) (Args...)>{};

    template<typename Fun1, typename Fun2>
    static inline typename std::enable_if<QtPrivate::FunctionPointer<Fun2>::ArgumentCount == -1, bool>::type
            overrideVfptrFun(quintptr *vfptr_t1, Fun1 fun1, Fun2 fun2, bool forceWrite)
    {
        typedef QtPrivate::FunctionPointer<Fun1> FunInfo1;
        const int FunctorArgumentCount = QtPrivate::ComputeFunctorArgumentCount<Fun2, typename FunctionPointer<Fun1>::Arguments>::Value;

        Q_STATIC_ASSERT_X((FunctorArgumentCount >= 0),
                          "Function1 and Function2 arguments are not compatible.");
        const int Fun2ArgumentCount = (FunctorArgumentCount >= 0) ? FunctorArgumentCount : 0;
        typedef typename QtPrivate::FunctorReturnType<Fun2, typename QtPrivate::List_Left<typename FunctionPointer<Fun1>::Arguments, Fun2ArgumentCount>::Value>::Value Fun2ReturnType;

        Q_STATIC_ASSERT_X((QtPrivate::AreArgumentsCompatible<Fun2ReturnType, typename FunInfo1::ReturnType>::value),
                          "Function1 and Function2 return type are not compatible.");

        StdFunWrap<Fun2, Fun1>::fun(fun2);
        return overrideVfptrFun(vfptr_t1, fun1, StdFunWrap<Fun2, Fun1>::call, forceWrite);
    }

    template<typename Fun1, typename Fun2>
    static bool overrideVfptrFun(const typename QtPrivate::FunctionPointer<Fun1>::Object *t1, Fun1 fun1, Fun2 fun2)
    {
        typedef QtPrivate::FunctionPointer<Fun1> FunInfo1;
        // 检查析构函数是否为虚
        class OverrideDestruct : public FunInfo1::Object { ~OverrideDestruct() override;};

        if (!ensureVtable((void*)t1, std::bind(&_destory_helper<typename FunInfo1::Object>, t1))) {
            return false;
        }

        bool ok = overrideVfptrFun(getVtableOfObject(t1), fun1, fun2, false);

        if (!ok) {
            // 恢复旧环境
            resetVtable(t1);
        }

        return true;
    }

    template<typename Class, typename Fun1, typename Fun2>
    static bool overrideVfptrFun(Fun1 fun1, Fun2 fun2)
    {
        quintptr *vfptr_t1 = getVtableOfClass<Class>();

        if (!vfptr_t1) {
            abort();
        }

        return overrideVfptrFun(vfptr_t1, fun1, fun2, true);
    }

    template<typename Fun1, typename Fun2>
    static bool overrideVfptrFun(Fun1 fun1, Fun2 fun2)
    {
        typedef QtPrivate::FunctionPointer<Fun1> FunInfo1;
        return overrideVfptrFun<typename FunInfo1::Object>(fun1, fun2);
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
    static bool clearGhostVtable(const void *obj);

    template<typename T>
    static void _destory_helper(const T *obj) {
        delete obj;
    }

    static QMap<quintptr**, quintptr*> objToOriginalVfptr;
    static QMap<const void*, quintptr*> objToGhostVfptr;
    static QMap<const void*, quintptr> objDestructFun;
};

DCORE_END_NAMESPACE

#endif // DVTABLEHOOK_H
