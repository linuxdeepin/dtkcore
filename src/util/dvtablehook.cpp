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
#include "dvtablehook.h"

#include <algorithm>
#ifdef Q_OS_LINUX
#include <sys/mman.h>
#include <unistd.h>

QT_BEGIN_NAMESPACE
QFunctionPointer qt_linux_find_symbol_sys(const char *symbol);
QT_END_NAMESPACE

#endif

DCORE_BEGIN_NAMESPACE

QMap<quintptr**, quintptr*> DVtableHook::objToOriginalVfptr;
QMap<const void*, quintptr*> DVtableHook::objToGhostVfptr;
QMap<const void*, quintptr> DVtableHook::objDestructFun;

bool DVtableHook::copyVtable(quintptr **obj)
{
    int vtable_size = getVtableSize(obj);

    if (vtable_size == 0)
        return false;

    // 多开辟一个元素, 新的虚表结构如下:
    // 假设obj对象原虚表长度为2, 表结构为:
    // ┏━━┳━━┳━━┓其中v1 v2为虚函数地址, 0为数组结尾
    // ┃v1┃v2┃\0┃
    // ┗━━┻━━┻━━┛
    // 则新的表结构为:
    // ┏━━┳━━┳━━┳━━┓其中前三个元素为原虚表的复制, sv为原虚表入口地址
    // ┃v1┃v2┃\0┃sv┃
    // ┗━━┻━━┻━━┻━━┛
    vtable_size += 2;

    quintptr *new_vtable = new quintptr[vtable_size];

    memcpy(new_vtable, *obj, (vtable_size - 1) * sizeof(quintptr));

    //! save original vfptr
    objToOriginalVfptr[obj] = *obj;
    // 存储对象原虚表入口地址
    new_vtable[vtable_size - 1] = quintptr(*obj);

    *obj = new_vtable;
    //! save ghost vfptr
    objToGhostVfptr[obj] = new_vtable;

    return true;
}

bool DVtableHook::clearGhostVtable(const void *obj)
{
    objToOriginalVfptr.remove((quintptr**)obj);
    objDestructFun.remove(obj);

    quintptr *vtable = objToGhostVfptr.take(obj);

    if (vtable) {
        delete[] vtable;

        return true;
    }

    return false;
}

/*!
 * \brief 通过遍历尝试找到析构函数在虚表中的位置
 * \param obj
 * \param destoryObjFun
 * \return
 */
int DVtableHook::getDestructFunIndex(quintptr **obj, std::function<void(void)> destoryObjFun)
{
    class _DestoryProbe
    {
    public:
        static quintptr probe(quintptr obj) {
            static quintptr _obj = 0;

            if (obj == 0) {
                obj = _obj;
                _obj = 0;
            } else {
                _obj = obj;
            }

            return obj;
        }

        static void nothing() {

        }
    };

    quintptr *vtable = *obj;
    int vtable_size = getVtableSize(obj);

    if (vtable_size == 0)
        return -1;

    quintptr *new_vtable = new quintptr[vtable_size];
    std::fill(new_vtable, new_vtable + vtable_size, quintptr(&_DestoryProbe::nothing));

    // 给对象设置新的虚表
    *obj = new_vtable;

    int index = -1;

    for (int i = 0; i < vtable_size; ++i) {
        new_vtable[i] = quintptr(&_DestoryProbe::probe);

        // 尝试销毁此对象, 且观察_DestoryProbe::probe是否被调用
        // 如果被调用, 则证明覆盖此虚函数能达到监控对象被销毁的目的
        destoryObjFun();

        if (_DestoryProbe::probe(0) == quintptr(obj)) {
            index = i;
            break;
        }
    }

    // 恢复旧的虚表
    *obj = vtable;
    // 销毁临时虚表
    delete[] new_vtable;

    return index;
}

void DVtableHook::autoCleanVtable(const void *obj)
{
    quintptr fun = objDestructFun.value(obj);

    if (!fun)
        return;

    typedef void(*Destruct)(const void*);
    Destruct destruct = *reinterpret_cast<Destruct*>(&fun);
    // call origin destruct function
    destruct(obj);

    if (hasVtable(obj)) {// 需要判断一下，有可能在执行析构函数时虚表已经被删除
        // clean
        clearGhostVtable(obj);
    }
}

bool DVtableHook::ensureVtable(const void *obj, std::function<void ()> destoryObjFun)
{
    quintptr **_obj = (quintptr**)(obj);

    if (objToOriginalVfptr.contains(_obj)) {
        // 不知道什么原因, 此时obj对象的虚表已经被还原
        if (objToGhostVfptr.value((void*)obj) != *_obj) {
            clearGhostVtable((void*)obj);
        } else {
            return true;
        }
    }

    if (!copyVtable(_obj))
        return false;

    // 查找对象的析构函数
    int index = getDestructFunIndex(_obj, destoryObjFun);

    // 虚析构函数查找失败
    if (index < 0) {
        qWarning("Failed do override destruct function");
        qDebug() << "object:" << obj;
        abort();
    }

    quintptr *new_vtable = *_obj;
    // 保存对象真实的析构函数
    objDestructFun[(void*)obj] = new_vtable[index];

    // 覆盖析构函数, 用于在对象析构时自动清理虚表
    new_vtable[index] = reinterpret_cast<quintptr>(&autoCleanVtable);

    return true;
}

/*!
 * \brief DVtableHook::hasVtable 对象的虚表已经被覆盖时返回true，否则返回false
 * \param obj
 * \return
 */
bool DVtableHook::hasVtable(const void *obj)
{
    quintptr **_obj = (quintptr**)(obj);

    return objToGhostVfptr.contains(_obj);
}

void DVtableHook::resetVtable(const void *obj)
{
    quintptr **_obj = (quintptr**)obj;
    int vtable_size = getVtableSize(_obj);
    // 获取obj对象原本虚表的入口
    quintptr *vfptr_t2 = (quintptr*)(*_obj)[vtable_size + 1];

    if (!vfptr_t2)
        return;

    if (!clearGhostVtable(obj))
        return;

    // 还原虚表
    *_obj = vfptr_t2;
}

/*!
 * \brief 将偏移量为functionOffset的虚函数还原到原本的实现
 * \param obj
 * \param functionIndex
 * \return 如果成功, 返回还原之前obj对象虚表中存储的函数指针, 否则返回0
 */
quintptr DVtableHook::resetVfptrFun(const void *obj, quintptr functionOffset)
{
    quintptr *vfptr_t1 = *(quintptr**)obj;
    quintptr current_fun = *(vfptr_t1 + functionOffset / sizeof(quintptr));
    quintptr origin_fun = originalFun(obj, functionOffset);

    if (!origin_fun) {
        return 0;
    }

    // reset to original fun
    *(vfptr_t1 + functionOffset / sizeof(quintptr)) = origin_fun;

    return current_fun;
}

/*!
 * \brief 获取obj对象偏移量为functionOffset的虚函数原本的函数指针
 * \param obj
 * \param functionOffset
 * \return 如果obj对象虚表没有被覆盖, 或者函数偏移量正确, 将返回0
 */
quintptr DVtableHook::originalFun(const void *obj, quintptr functionOffset)
{
    quintptr **_obj = (quintptr**)obj;
    int vtable_size = getVtableSize(_obj);
    // 获取obj对象原本虚表的入口
    quintptr *vfptr_t2 = (quintptr*)(*_obj)[vtable_size + 1];

    if (!vfptr_t2) {
        qWarning() << "Not override the object virtual table" << obj;

        return 0;
    }

    if (functionOffset > UINT_LEAST16_MAX) {
        qWarning() << "Is not a virtual function, function address: 0x" << hex << functionOffset;

        return 0;
    }

    return *(vfptr_t2 + functionOffset / sizeof(quintptr));
}

bool DVtableHook::forceWriteMemory(void *adr, const void *data, size_t length)
{
#ifdef Q_OS_LINUX
    int page_size = sysconf(_SC_PAGESIZE);
    quintptr x = reinterpret_cast<quintptr>(adr);
    void *new_adr = reinterpret_cast<void*>((x - page_size - 1) & ~(page_size -1));
    size_t override_data_length = length + x - reinterpret_cast<quintptr>(new_adr);

    // 失败时直接放弃
    if (mprotect(new_adr, override_data_length, PROT_READ | PROT_WRITE)) {
        return false;
    }
#endif
    // 复制数据
    memcpy(adr, data, length);
#ifdef Q_OS_LINUX
    // 恢复内存标志位
    mprotect(new_adr, override_data_length, PROT_READ);
#endif

    return true;
}

QFunctionPointer DVtableHook::resolve(const char *symbol)
{
#ifdef Q_OS_LINUX
    return QT_PREPEND_NAMESPACE(qt_linux_find_symbol_sys)(symbol);
#else
    // TODO
    return nullptr;
#endif
}

DCORE_END_NAMESPACE
