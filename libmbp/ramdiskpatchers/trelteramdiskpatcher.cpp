/*
 * Copyright (C) 2015  Andrew Gunnerson <andrewgunnerson@gmail.com>
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

#include "ramdiskpatchers/trelteramdiskpatcher.h"

#include <regex>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>

#include "patcherconfig.h"
#include "ramdiskpatchers/coreramdiskpatcher.h"


namespace mbp
{

/*! \cond INTERNAL */
class TrelteBaseRamdiskPatcher::Impl
{
public:
    const PatcherConfig *pc;
    const FileInfo *info;
    CpioFile *cpio;

    PatcherError error;
};
/*! \endcond */


/*!
    \class TrelteRamdiskPatcher
    \brief Handles common ramdisk patching operations for the Samsung Galaxy Note 4

    This patcher handles the patching of ramdisks for the Samsung Galaxy Note 4.
    Starting from version 9.0.0, every Android ramdisk is supported.
 */


TrelteBaseRamdiskPatcher::TrelteBaseRamdiskPatcher(const PatcherConfig * const pc,
                                                   const FileInfo * const info,
                                                   CpioFile * const cpio) :
    m_impl(new Impl())
{
    m_impl->pc = pc;
    m_impl->info = info;
    m_impl->cpio = cpio;
}

TrelteBaseRamdiskPatcher::~TrelteBaseRamdiskPatcher()
{
}

PatcherError TrelteBaseRamdiskPatcher::error() const
{
    return m_impl->error;
}

////////////////////////////////////////////////////////////////////////////////

const std::string TrelteDefaultRamdiskPatcher::Id = "trelte/default";

TrelteDefaultRamdiskPatcher::TrelteDefaultRamdiskPatcher(const PatcherConfig * const pc,
                                                         const FileInfo *const info,
                                                         CpioFile *const cpio)
    : TrelteBaseRamdiskPatcher(pc, info, cpio)
{
}

std::string TrelteDefaultRamdiskPatcher::id() const
{
    return Id;
}

bool TrelteDefaultRamdiskPatcher::patchRamdisk()
{
    CoreRamdiskPatcher corePatcher(m_impl->pc, m_impl->info, m_impl->cpio);

    if (!corePatcher.patchRamdisk()) {
        m_impl->error = corePatcher.error();
        return false;
    }

    if (!corePatcher.useGeneratedFstabAuto()) {
        m_impl->error = corePatcher.error();
        return false;
    }

    if (!corePatcher.fixChargerMountAuto()) {
        m_impl->error = corePatcher.error();
        return false;
    }

    return true;
}

}