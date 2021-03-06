/*
 * Copyright (C) 2007-2015 Frank Mertens.
 *
 * Use of this source is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include <flux/File>
#include <flux/Dir>
#include "utils.h"
#include "exceptions.h"
#include "ServiceWorker.h"
#include "ErrorLog.h"
#include "DirectoryInstance.h"
#include "MediaTypeDatabase.h"
#include "DirectoryDelegate.h"

namespace fluxnode {

Ref<DirectoryDelegate> DirectoryDelegate::create(ServiceWorker *worker)
{
    return new DirectoryDelegate(worker);
}

DirectoryDelegate::DirectoryDelegate(ServiceWorker *worker):
    ServiceDelegate(worker),
    directoryInstance_(worker->serviceInstance())
{}

void DirectoryDelegate::process(Request *request)
{
    String path = directoryInstance_->path() + "/" + request->target();
    path = path->canonicalPath();
    String prefix = path->head(directoryInstance_->path()->count());
    if (path->head(directoryInstance_->path()->count()) != directoryInstance_->path()) throw Forbidden();

    Ref<FileStatus> fileStatus = FileStatus::read(path);
    if (!fileStatus->exists()) throw NotFound();

    {
        String h;
        if (request->lookup("If-Modified-Since", &h)) {
            Ref<Date> cacheDate = scanDate(h);
            if (cacheDate) {
                if (fileStatus->lastModified() <= cacheDate->time()) {
                    status(304);
                    begin();
                    end();
                    return;
                }
            }
        }
    }

    header("Last-Modified", formatDate(Date::create(fileStatus->lastModified())));

    if (fileStatus->type() == File::Directory) {
        String indexPath;
        const char *indexNames[] = { "index.html", "index.htm" };
        for (int i = 0, n = sizeof(indexNames) / sizeof(indexNames[0]); i < n; ++i) {
            String candidate = path->expandPath(indexNames[i]);
            if (File::exists(candidate)) {
                indexPath = candidate;
                break;
            }
        }
        if (indexPath != "") deliverFile(indexPath);
        else listDirectory(request, path);
    }
    else if (fileStatus->type() == File::Regular) {
        deliverFile(path);
    }
    else {
        streamFile(path);
    }
}

void DirectoryDelegate::listDirectory(Request *request, String path)
{
    Ref<Dir> dir = Dir::open(path);

    header("Content-Type", "text/html");
    chunk() <<
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head>\n"
        "<title>" << request->target() << "</title>\n" <<
        "<style type=\"text/css\">\n"
        "a.file { float: left; width: 100pt; font-size: 10pt; padding: 10pt; overflow: hidden; text-overflow: ellipsis }\n"
        "</style>\n"
        "</head>\n"
        "<body>\n";

    String prefix = request->target()->canonicalPath();
    if (prefix->count() > 0) {
        if (prefix->at(prefix->count() - 1) != '/')
            prefix += "/";
    }

    Ref<StringList> entries = StringList::create();
    for (String name; dir->read(&name);) {
        if (name == "." || name == "..") continue;
        if ((!directoryInstance_->showHidden()) && name->at(0) == '.') continue;
        entries->append(name);
    }
    entries = entries->sort();

    for (int i = 0; i < entries->count(); ++i) {
        String name = entries->at(i);
        chunk() << "<a class=\"file\" href=\"" << prefix << name << "\">" << name << "</a>\n";
    }

    chunk() <<
        "</body>\n"
        "</html>\n";
}

void DirectoryDelegate::deliverFile(String path)
{
    String content = File::open(path)->map();
    String mediaType = mediaTypeDatabase()->lookup(path, content);
    if (mediaType != "") header("Content-Type", mediaType);
    begin(content->count());
    write(content);
    end();
}

void DirectoryDelegate::streamFile(String path)
{
    String mediaType = mediaTypeDatabase()->lookup(path, "");
    if (mediaType != "") header("Content-Type", mediaType);
    Ref<File> file = File::open(path);
    Ref<ByteArray> buf = ByteArray::create(0x10000);
    while (true) {
        int n = file->read(buf);
        if (n == 0) break;
        write(buf->select(0, n));
    }
}

} // namespace fluxnode
