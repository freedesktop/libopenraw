/* -*- Mode: C++ -*- */
/*
 * libopenraw - file.hpp
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

#ifndef OR_INTERNALS_IO_FILE_H_
#define OR_INTERNALS_IO_FILE_H_

#include <stddef.h>
#include <sys/types.h>

#include <libopenraw/io.h>

#include "stream.hpp"

namespace OpenRaw {
namespace IO {

/** @addtogroup io_internals
 * @{ */

/** @brief File IO stream */
class File : public Stream {
public:
    /** Contruct the file
     * @param filename the full pathname for the file
     */
    File(const char *filename);
    virtual ~File();

    File(const File &f) = delete;
    File &operator=(const File &) = delete;

    // file APIs
    /** @brief Open the file.
     *
     * Return OR_ERROR_ALREADY_OPEN if it is already open.
     * The stream is still usable after that.
     */
    virtual Error open() override;
    /** @inherit */
    virtual int close() override;
    /** @inherit */
    virtual int seek(off_t offset, int whence) override;
    /** @inherit */
    virtual int read(void *buf, size_t count) override;
    virtual off_t filesize() override;
    //virtual void *mmap(size_t l, off_t offset) override;
    //virtual int munmap(void *addr, size_t l) override;

private:
    /** @brief The interface to the C io */
    ::io_methods *m_methods;
    /** @brief The C io file handle */
    ::IOFileRef m_ioRef;
};

/** @} */
}
}

#endif
