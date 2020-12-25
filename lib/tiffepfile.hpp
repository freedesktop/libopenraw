/* -*- Mode: C++ -*- */
/*
 * libopenraw - tiffepfile.h
 *
 * Copyright (C) 2007-2020 Hubert Figuière
 * Copyright (C) 2008 Novell, Inc.
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */


#pragma once

#include "ifddir.hpp"
#include "ifdfile.hpp"
#include "io/stream.hpp"
#include "rawfile.hpp"

namespace OpenRaw {
namespace Internals {

/** @addtogroup ifd_parsing
 * @{
 */

/** @brief TIFF.EP conformant files.
 *
 * A more strict TIFF file. This includes DNG, NEF, ERF.
 */
class TiffEpFile
    : public IfdFile
{
public:
    TiffEpFile(const IO::Stream::Ptr &s, Type _type);

protected:

    /** @inherit */
    virtual IfdDir::Ref  _locateCfaIfd() override;
};

/** @} */
}
}
