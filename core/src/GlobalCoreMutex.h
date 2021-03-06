/*
 * Copyright (C) 2007-2015 Frank Mertens.
 *
 * Use of this source is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef FLUX_GLOBALCOREMUTEX_H
#define FLUX_GLOBALCOREMUTEX_H

#include <flux/Object>
#include <flux/SpinLock>
#include <flux/Ref>

namespace flux {

class GlobalCoreMutex;

class GlobalCoreMutexInitializer
{
public:
    GlobalCoreMutexInitializer();
private:
    static int count_;
};

namespace { GlobalCoreMutexInitializer globalCoreMutexInitializer; }

class GlobalCoreMutex: public SpinLock
{
public:
    static GlobalCoreMutex *instance();

private:
    GlobalCoreMutex() {}
};

inline GlobalCoreMutex *globalCoreMutex() { return GlobalCoreMutex::instance(); }

} // namespace flux

#endif // FLUX_GLOBALCOREMUTEX_H
