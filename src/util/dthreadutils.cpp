/*
 * Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
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

#include "dthreadutils.h"

DCORE_BEGIN_NAMESPACE

namespace DThreadUtil {
FunctionCallProxy::FunctionCallProxy(QThread *thread)
{
    connect(this, &FunctionCallProxy::callInLiveThread, this, [this] (FunctionType *func) {
        (*func)();
    }, Qt::QueuedConnection);
    connect(thread, &QThread::finished, this, [this] {
        qWarning() << Q_FUNC_INFO << sender() << "the thread finished";
    }, Qt::DirectConnection);
}
} // end namespace DThreadUtil

DCORE_END_NAMESPACE
