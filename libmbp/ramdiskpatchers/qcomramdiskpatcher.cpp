/*
 * Copyright (C) 2014  Andrew Gunnerson <andrewgunnerson@gmail.com>
 *
 * This file is part of MultiBootPatcher
 *
 * MultiBootPatcher is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MultiBootPatcher is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MultiBootPatcher.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ramdiskpatchers/qcomramdiskpatcher.h"

#include <regex>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>

#include <cppformat/format.h>

#include "ramdiskpatchers/coreramdiskpatcher.h"
#include "private/logging.h"


namespace mbp
{

/*! \cond INTERNAL */
class QcomRamdiskPatcher::Impl
{
public:
    const PatcherConfig *pc;
    const FileInfo *info;
    CpioFile *cpio;

    PatcherError error;
};
/*! \endcond */


static const std::string FstabRegex =
        "^(#.+)?(/dev/\\S+)\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)";

static const std::string CachePartition =
        "/dev/block/platform/msm_sdcc.1/by-name/cache";

static const std::string System = "/system";
static const std::string Cache = "/cache";
static const std::string Data = "/data";


QcomRamdiskPatcher::QcomRamdiskPatcher(const PatcherConfig * const pc,
                                       const FileInfo * const info,
                                       CpioFile * const cpio) :
    m_impl(new Impl())
{
    m_impl->pc = pc;
    m_impl->info = info;
    m_impl->cpio = cpio;
}

QcomRamdiskPatcher::~QcomRamdiskPatcher()
{
}

PatcherError QcomRamdiskPatcher::error() const
{
    return m_impl->error;
}

std::string QcomRamdiskPatcher::id() const
{
    return std::string();
}

bool QcomRamdiskPatcher::patchRamdisk()
{
    return false;
}

bool QcomRamdiskPatcher::addMissingCacheInFstab(const std::vector<std::string> &additionalFstabs)
{
    std::vector<std::string> fstabs;
    for (auto const &file : m_impl->cpio->filenames()) {
        if (boost::starts_with(file, "fstab.")) {
            fstabs.push_back(file);
        }
    }

    fstabs.insert(fstabs.end(),
                  additionalFstabs.begin(), additionalFstabs.end());

    for (auto const &fstab : fstabs) {
        std::vector<unsigned char> contents;
        if (!m_impl->cpio->contents(fstab, &contents)) {
            m_impl->error = m_impl->cpio->error();
            return false;
        }

        std::vector<std::string> lines;
        boost::split(lines, contents, boost::is_any_of("\n"));

        // Some Android 4.2 ROMs mount the cache partition in the init
        // scripts, so the fstab has no cache line
        bool hasCacheLine = false;

        for (auto it = lines.begin(); it != lines.end(); ++it) {
            auto &line = *it;

            std::smatch what;
            bool hasMatch = std::regex_search(
                    line, what, std::regex(FstabRegex));

            if (hasMatch && what[3] == Cache) {
                hasCacheLine = true;
            }
        }

        if (!hasCacheLine) {
            std::string cacheLine = "{0} /cache ext4 {1} {2}";
            std::string mountArgs = "nosuid,nodev,barrier=1";
            std::string voldArgs = "wait,check";

            lines.push_back(fmt::format(
                    cacheLine, CachePartition, mountArgs, voldArgs));
        }

        std::string strContents = boost::join(lines, "\n");
        contents.assign(strContents.begin(), strContents.end());
        m_impl->cpio->setContents(fstab, std::move(contents));
    }

    return true;
}

bool QcomRamdiskPatcher::stripManualMounts(const std::string &filename)
{
    std::vector<unsigned char> contents;
    if (!m_impl->cpio->contents(filename, &contents)) {
        m_impl->error = m_impl->cpio->error();
        return false;
    }

    std::vector<std::string> lines;
    boost::split(lines, contents, boost::is_any_of("\n"));

    const std::regex reWaitCache("^\\s+wait\\s+/dev/.*/cache");
    const std::regex reCheckFsCache("^\\s+check_fs\\s+/dev/.*/cache");
    const std::regex reMountCache("^\\s+mount\\s+ext4\\s+/dev/.*/cache");
    const std::regex reWaitData("^\\s+wait\\s+/dev/.*/userdata");
    const std::regex reCheckFsData("^\\s+check_fs\\s+/dev/.*/userdata");
    const std::regex reMountData("^\\s+mount\\s+ext4\\s+/dev/.*/userdata");

    for (auto it = lines.begin(); it != lines.end(); ++it) {
        if (std::regex_search(*it, reWaitCache)
                || std::regex_search(*it, reCheckFsCache)
                || std::regex_search(*it, reMountCache)
                || std::regex_search(*it, reWaitData)
                || std::regex_search(*it, reCheckFsData)
                || std::regex_search(*it, reMountData)) {
            it->insert(it->begin(), '#');
        }
    }

    std::string strContents = boost::join(lines, "\n");
    contents.assign(strContents.begin(), strContents.end());
    m_impl->cpio->setContents(filename, std::move(contents));

    return true;
}

}