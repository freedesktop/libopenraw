/*
 * libopenraw - xtranspattern.h
 *
 * Copyright (C) 2012-2020 Hubert Figuière
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

#include "mosaicinfo.hpp"

namespace OpenRaw {
namespace Internals {

/** @defgroup fujifilm Fujfilm parsing
 * @ingroup internals
 *
 * @brief Fujifilm specific parsing.
 *
 * @{
 */

/**
 * @brief The X-Trans CMOS 6x6 pattern starting with the Fuji X-Pro1.
 *
 * When RafFile::isXTrans() is true, this is the type a MosaicInfo returned.
 */
class XTransPattern
	: public MosaicInfo
{
public:
  /** @brief Return the X-Trans pattern singleton */
  static const XTransPattern* xtransPattern();

protected:
  XTransPattern();
};

/** @} */

}
}
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0))
  tab-width:2
  c-basic-offset:2
  indent-tabs-mode:nil
  fill-column:80
  End:
*/
