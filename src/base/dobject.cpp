/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "dobject.h"
#include "private/dobject_p.h"

DCORE_BEGIN_NAMESPACE

DObjectPrivate::DObjectPrivate(DObject *qq)
    : q_ptr(qq)
{

}

DObjectPrivate::~DObjectPrivate()
{

}

DObject::DObject(DObject * /*parent = nullptr*/)
{

}

DObject::DObject(DObjectPrivate &dd, DObject * /*parent = nullptr*/):
    d_d_ptr(&dd)
{

}

DObject::~DObject()
{

}

DCORE_END_NAMESPACE
