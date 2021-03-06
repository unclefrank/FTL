/*
 * Copyright (C) 2007-2015 Frank Mertens.
 *
 * Use of this source is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include <unistd.h> // sysconf
#include <flux/exceptions>
#include <flux/Group>

namespace flux {

Group::Group(gid_t id)
{
    int bufSize = sysconf(_SC_GETGR_R_SIZE_MAX);
    if (bufSize == -1) FLUX_SYSTEM_DEBUG_ERROR(errno);
    String buf(bufSize);
    struct group space;
    memclr(&space, sizeof(struct group));
    struct group *entry = 0;
    int ret = ::getgrgid_r(id, &space, buf->chars(), buf->count(), &entry);
    if ((!entry) && ret == 0) ret = ENOENT;
    if (ret != 0) FLUX_SYSTEM_DEBUG_ERROR(ret);
    load(entry);
}

Group::Group(const char *name)
{
    int bufSize = sysconf(_SC_GETGR_R_SIZE_MAX);
    if (bufSize == -1) FLUX_SYSTEM_DEBUG_ERROR(errno);
    String buf(bufSize);
    struct group space;
    memclr(&space, sizeof(struct group));
    struct group *entry = 0;
    int ret = ::getgrnam_r(name, &space, buf->chars(), buf->count(), &entry);
    if ((!entry) && ret == 0) ret = ENOENT;
    if (ret != 0) FLUX_SYSTEM_RESOURCE_ERROR(ret, name);
    load(entry);
}

void Group::load(struct group *entry)
{
    id_ = entry->gr_gid;
    name_ = entry->gr_name;
    members_ = StringList::create();
    char **pcs = entry->gr_mem;
    while (*pcs) {
        members_->append(*pcs);
        ++pcs;
    }
}

} // namespace flux
