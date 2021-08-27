/*
 * Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
 *
 * Author:     Wang Fei <wangfeia@uniontech.com>
 *
 * Maintainer: Wang Fei <wangfeia@uniontech.com>
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

#include <gtest/gtest.h>
#include "dsecurestring.h"

DCORE_USE_NAMESPACE


class ut_DSecureString : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;
    DSecureString *secureString;
    QString string;
};

void ut_DSecureString::SetUp()
{
    secureString = new DSecureString(string);
}

void ut_DSecureString::TearDown()
{
    if (secureString) {
        delete secureString;
        secureString = nullptr;
    }
}

TEST_F(ut_DSecureString, testString)
{
    QString test = secureString->fromLatin1("test");
    ASSERT_TRUE(test == QString("test"));
}
