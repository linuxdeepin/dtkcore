/*
 * Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
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

#include "ut_dutil.h"
#include <QGuiApplication>

#ifdef QT_DEBUG
#include <sanitizer/asan_interface.h>
#endif

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    DTimedLoop loop;

    testing::InitGoogleTest(&argc, argv);
    int retVal = RUN_ALL_TESTS();

#ifdef QT_DEBUG
    __sanitizer_set_report_path("asan.log");
#endif

    return loop.exec(0, "main execution") + retVal;
}
