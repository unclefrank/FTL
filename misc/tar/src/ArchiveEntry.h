/*
 * Copyright (C) 2007-2015 Frank Mertens.
 *
 * Use of this source is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef FLUXTAR_ARCHIVEENTRY_H
#define FLUXTAR_ARCHIVEENTRY_H

#include <sys/types.h>
#include <flux/String>

namespace flux {
namespace tar {

class ArchiveReader;

class ArchiveEntry: public Object
{
public:
    enum Type {
        Regular     = '0',
        Link        = '1',
        Symlink     = '2',
        CharDevice  = '3',
        BlockDevive = '4',
        Directory   = '5',
        Fifo        = '6',
        MinType     = Regular,
        MaxType     = Fifo
    };

    static Ref<ArchiveEntry> create() {
        return new ArchiveEntry();
    }

    inline int type() const { return type_; }
    inline String path() const { return path_; }
    inline String linkPath() const { return linkPath_; }
    inline double lastModified() const { return lastModified_; }
    inline uid_t userId() const { return userId_; }
    inline gid_t groupId() const { return groupId_; }
    inline String userName() const { return userName_; }
    inline String groupName() const { return groupName_; }
    inline int mode() const { return mode_; }
    inline off_t offset() const { return offset_; }
    inline off_t size() const { return size_; }

    int type_;
    String path_;
    String linkPath_;
    double lastModified_;
    uid_t userId_;
    gid_t groupId_;
    String userName_;
    String groupName_;
    int mode_;
    off_t offset_;
    off_t size_;

private:
    ArchiveEntry()
        : type_('0')
    {}
};

}} // namespace flux::tar

#endif // FLUXTAR_ARCHIVEENTRY_H
