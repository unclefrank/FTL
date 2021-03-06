/*
 * Copyright (C) 2007-2015 Frank Mertens.
 *
 * Use of this source is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef FLUX_REFGUARD_H
#define FLUX_REFGUARD_H

#include <flux/Ref>

namespace flux {

template<class T>
class RefGuard
{
public:
    RefGuard(Ref<T> *ref): ref_(ref) {}
    ~RefGuard() { *ref_ = 0; }
private:
    Ref<T> *ref_;
};

} // namespace flux

#endif // FLUX_REFGUARD_H
