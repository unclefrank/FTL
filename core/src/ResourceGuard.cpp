/*
 * Copyright (C) 2007-2015 Frank Mertens.
 *
 * Use of this source is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include <flux/ResourceContext>
#include <flux/ResourceGuard>

namespace flux {

ResourceGuard::ResourceGuard(String resource)
{
    resourceContext()->push(resource);
}

ResourceGuard::~ResourceGuard()
{
    resourceContext()->pop();
}

} // namespace flux
