/*
 * libopenraw - ifdentry.hpp
 *
 * Copyright (C) 2006-2020 Hubert Figuière
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

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>

#include <exception>
#include <string>
#include <vector>
#include <memory>

#include "exception.hpp"
#include "trace.hpp"
#include "endianutils.hpp"
#include "rawcontainer.hpp"
#include "ifd.hpp"

namespace OpenRaw {

class MetaValue;

namespace Internals {

/** @addtogroup ifd_parsing
 * @{
 */
class IfdDir;

class IfdEntry;

/** @brief Describe an IFD type */
template <typename T>
struct IfdTypeTrait
{
	static const uint16_t type; /**< the EXIF enum for the type */
	static const size_t   size; /**< the storage size unit in IFD*/
	static T EL(const uint8_t* d, size_t len) noexcept;
	static T BE(const uint8_t* d, size_t len) noexcept;
};


template <>
inline uint8_t IfdTypeTrait<uint8_t>::EL(const uint8_t* b, size_t) noexcept
{
	return *b;
}

template <>
inline uint8_t IfdTypeTrait<uint8_t>::BE(const uint8_t* b, size_t) noexcept
{
	return *b;
}

template <>
inline int8_t IfdTypeTrait<int8_t>::EL(const uint8_t* b, size_t) noexcept
{
	return *b;
}

template <>
inline int8_t IfdTypeTrait<int8_t>::BE(const uint8_t* b, size_t) noexcept
{
	return *b;
}

template <>
inline uint16_t IfdTypeTrait<uint16_t>::EL(const uint8_t* b, size_t) noexcept
{
	return EL16(b);
}

template <>
inline uint16_t IfdTypeTrait<uint16_t>::BE(const uint8_t* b, size_t) noexcept
{
	return BE16(b);
}

template <>
inline int16_t IfdTypeTrait<int16_t>::EL(const uint8_t* b, size_t) noexcept
{
	uint16_t uns = EL16(b);
	return *(int16_t*)&uns;
}

template <>
inline int16_t IfdTypeTrait<int16_t>::BE(const uint8_t* b, size_t) noexcept
{
	uint16_t uns = BE16(b);
	return *(int16_t*)&uns;
}

template <>
inline uint32_t IfdTypeTrait<uint32_t>::EL(const uint8_t* b, size_t) noexcept
{
	return EL32(b);
}

template <>
inline uint32_t IfdTypeTrait<uint32_t>::BE(const uint8_t* b, size_t) noexcept
{
	return BE32(b);
}

template <>
inline int32_t IfdTypeTrait<int32_t>::EL(const uint8_t* b, size_t) noexcept
{
	return EL32(b);
}

template <>
inline int32_t IfdTypeTrait<int32_t>::BE(const uint8_t* b, size_t) noexcept
{
	return BE32(b);
}

template <>
inline std::string IfdTypeTrait<std::string>::EL(const uint8_t* b, size_t len) noexcept
{
  std::string s;
  try {
    s.assign((const char*)b, strnlen((const char*)b, len));
  }
  catch(...) {
  }
  return s;
}

template <>
inline std::string IfdTypeTrait<std::string>::BE(const uint8_t* b, size_t len) noexcept
{
  std::string s;
  try {
    s.assign((const char*)b, strnlen((const char*)b, len));
  }
  catch(...) {
  }
  return s;
}

template <>
inline IFD::ORRational IfdTypeTrait<IFD::ORRational>::EL(const uint8_t* b, size_t) noexcept
{
    IFD::ORRational r;
    r.num = EL32(b);
    r.denom = EL32(b + 4);
    return r;
}

template <>
inline IFD::ORRational IfdTypeTrait<IFD::ORRational>::BE(const uint8_t* b, size_t) noexcept
{
    IFD::ORRational r;
    r.num = BE32(b);
    r.denom = BE32(b + 4);
    return r;
}

template <>
inline IFD::ORSRational IfdTypeTrait<IFD::ORSRational>::EL(const uint8_t* b, size_t) noexcept
{
    IFD::ORSRational r;
    r.num = EL32(b);
    r.denom = EL32(b + 4);
    return r;
}

template <>
inline IFD::ORSRational IfdTypeTrait<IFD::ORSRational>::BE(const uint8_t* b, size_t) noexcept
{
    IFD::ORSRational r;
    r.num = BE32(b);
    r.denom = BE32(b + 4);
    return r;
}

/** @brief An entry in the IfdDir. */
class IfdEntry
{
public:
	/** @brief IfdEntry reference (ie shared pointer) */
	typedef std::shared_ptr<IfdEntry> Ref;

	IfdEntry(uint16_t _id, int16_t _type, int32_t _count, uint32_t _data,
					 const IfdDir& _dir, bool synthetic = false);
	virtual ~IfdEntry();

	int16_t id() const noexcept
		{
			return m_id;
		}
	int16_t type() const noexcept
		{
			return m_type;
		}

	/** @brief The count of items in the entry. */
	uint32_t count() const noexcept
		{
			return m_count;
		}

	/** @brief The offset of the data.
	 *
	 *  It can just be the value if the entry is self contained.
	 */
	off_t offset() noexcept
		{
			if (endian() == RawContainer::ENDIAN_LITTLE) {
				return IfdTypeTrait<uint32_t>::EL((uint8_t*)&m_data, sizeof(uint32_t));
			}
			return IfdTypeTrait<uint32_t>::BE((uint8_t*)&m_data, sizeof(uint32_t));
		}

	RawContainer::EndianType endian() const;

	/** @brief Return the raw data pointer */
	const uint8_t* dataptr() const
		{
			if (m_dataptr == nullptr) {
				return (uint8_t*)&m_data;
			} else {
				return m_dataptr;
			}
		}

	/**
	 * @brief Unit size for type
	 */
	static size_t typeUnitSize(IFD::ExifTagType _type);
	/** @brief Load the data for the entry if m_loaded is false
	 *
	 * If all the data fits in m_data, it will keep it there
	 * it will set m_loaded in case of success.
	 * @param unit_size the size of 1 unit of data
	 * @param offset offset from the begining of the container. Usually 0.
	 * @return true m_loaded
	 */
	bool loadData(size_t unit_size, off_t offset);
	size_t loadDataInto(uint8_t* dataptr, size_t data_size, off_t offset) const;
	void setData(const uint8_t* dataptr, size_t data_size);
private:
	uint16_t m_id;
	uint16_t m_type;
	uint32_t m_count;
	uint32_t m_data; /**< @brief raw data without endian conversion */
	bool m_loaded;
	uint8_t *m_dataptr;
	const IfdDir& m_dir;
	template <typename T> friend struct IfdTypeTrait;

	IfdEntry(const IfdEntry& f) = delete;
	IfdEntry & operator=(const IfdEntry&) = delete;

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
