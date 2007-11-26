/*
 * libopenraw - crwfile.cpp
 *
 * Copyright (C) 2006-2007 Hubert Figuiere
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include <algorithm>
#include <boost/bind.hpp>
#include <boost/scoped_ptr.hpp>

#include <libopenraw/libopenraw.h>
#include <libopenraw++/thumbnail.h>
#include <libopenraw++/rawdata.h>

#include "debug.h"
#include "io/file.h"
#include "io/streamclone.h"
#include "io/memstream.h"
#include "crwfile.h"
#include "ciffcontainer.h"
#include "jfifcontainer.h"
#include "crwdecompressor.h"

#include "rawfilefactory.h"

using namespace Debug;
using boost::scoped_ptr;

namespace OpenRaw {

	namespace Internals {

		using namespace CIFF;

		RawFile *CRWFile::factory(const char* _filename)
		{
			return new CRWFile(_filename);
		}

		CRWFile::CRWFile(const char* _filename)
			: RawFile(_filename, OR_RAWFILE_TYPE_CRW),
				m_io(new IO::File(_filename)),
				m_container(new CIFFContainer(m_io)),
				m_x(0), m_y(0)
		{

		}

		CRWFile::~CRWFile()
		{
			delete m_container;
			delete m_io;
		}

		::or_error CRWFile::_enumThumbnailSizes(std::vector<uint32_t> &list)
		{
			::or_error err = OR_ERROR_NOT_FOUND;

			Heap::Ref heap = m_container->heap();
			if(!heap) {
				// this is not a CIFF file.
				return err;
			}
			const RecordEntry::List & records = heap->records();
			RecordEntry::List::const_iterator iter;
			iter = std::find_if(records.begin(), records.end(), boost::bind(
														&RecordEntry::isA, _1, 
														static_cast<uint16_t>(TAG_JPEGIMAGE)));
			if (iter != records.end()) {
				Trace(DEBUG2) << "JPEG @" << (*iter).offset << "\n";
				m_x = m_y = 0;
				
				scoped_ptr<IO::StreamClone> s(new IO::StreamClone(m_io, heap->offset()
																  + (*iter).offset));
				scoped_ptr<JFIFContainer> jfif(new JFIFContainer(s.get(), 0));

				jfif->getDimensions(m_x, m_y);
				Trace(DEBUG1) << "JPEG dimensions x=" << m_x 
											<< " y=" << m_y << "\n";
				list.push_back(std::max(m_x,m_y));
				err = OR_ERROR_NONE;
			}

			return err;
		}

		::or_error CRWFile::_getThumbnail(uint32_t /*size*/, Thumbnail & thumbnail)
		{
			::or_error err = OR_ERROR_NOT_FOUND;
			Heap::Ref heap = m_container->heap();
			if(!heap) {
				// this is not a CIFF file.
				return err;
			}

			const RecordEntry::List & records = heap->records();
			RecordEntry::List::const_iterator iter;
			iter = std::find_if(records.begin(), records.end(), boost::bind(
														&RecordEntry::isA, _1, 
														static_cast<uint16_t>(TAG_JPEGIMAGE)));
			if (iter != records.end()) {
				Trace(DEBUG2) << "JPEG @" << (*iter).offset << "\n";
				size_t byte_size = (*iter).length;
				void *buf = thumbnail.allocData(byte_size);
				size_t real_size = (*iter).fetchData(heap.get(), buf, byte_size);
				if (real_size != byte_size) {
					Trace(WARNING) << "wrong size\n";
				}
				thumbnail.setDimensions(m_x, m_y);
				thumbnail.setDataType(OR_DATA_TYPE_JPEG);
				err = OR_ERROR_NONE;
			}

			return err;
		}

		::or_error CRWFile::_getRawData(RawData & data, uint32_t options)
		{
			::or_error err = OR_ERROR_NOT_FOUND;
			Heap::Ref heap = m_container->heap();
			if(!heap) {
				// this is not a CIFF file.
				return err;
			}

			const RecordEntry::List & records = heap->records();
			RecordEntry::List::const_iterator iter;

			// locate the properties
			iter = std::find_if(records.begin(), records.end(), boost::bind(
														&RecordEntry::isA, _1, 
														static_cast<uint16_t>(TAG_IMAGEPROPS)));
			if (iter == records.end()) {
				Trace(ERROR) << "Couldn't find the image properties.\n";
				return err;
			}
			
			Heap props(iter->offset + heap->offset(), iter->length, m_container);
			const RecordEntry::List & propsRecs = props.records();
			iter = std::find_if(propsRecs.begin(), propsRecs.end(), boost::bind(
														&RecordEntry::isA, _1, 
														static_cast<uint16_t>(TAG_IMAGEINFO)));
			if (iter == propsRecs.end()) {
				Trace(ERROR) << "Couldn't find the image info.\n";
				return err;
			}
			ImageSpec img_spec;
			img_spec.readFrom(iter->offset + props.offset(), m_container);
			uint32_t x, y;
			x = img_spec.imageWidth;
			y = img_spec.imageHeight;



			// locate decoder table
			iter = std::find_if(propsRecs.begin(), propsRecs.end(), boost::bind(
														&RecordEntry::isA, _1, 
														static_cast<uint16_t>(TAG_EXIFINFORMATION)));
			if (iter == propsRecs.end()) {
				Trace(ERROR) << "Couldn't find the Exif information.\n";
				return err;
			}

			Heap exifProps(iter->offset + props.offset(), iter->length, m_container);

			const RecordEntry::List & exifPropsRecs = exifProps.records();
			iter = std::find_if(exifPropsRecs.begin(), exifPropsRecs.end(), 
													boost::bind(
														&RecordEntry::isA, _1, 
														static_cast<uint16_t>(TAG_DECODERTABLE)));
			if (iter == exifPropsRecs.end()) {
				Trace(ERROR) << "Couldn't find the decoder table.\n";
				return err;
			}
			Trace(DEBUG2) << "length = " << iter->length << "\n";
			Trace(DEBUG2) << "offset = " << exifProps.offset() + iter->offset << "\n";
			IO::Stream *file = m_container->file();
			file->seek(exifProps.offset() + iter->offset, SEEK_SET);
			uint32_t decoderTable;
			if(m_container->readUInt32(file, decoderTable)) {
				Trace(DEBUG2) << "decoder table = " << decoderTable << "\n";
			}

			// locate the CFA info
			uint16_t cfa_x, cfa_y;
			iter = std::find_if(exifPropsRecs.begin(), exifPropsRecs.end(), boost::bind(
														&RecordEntry::isA, _1, 
														static_cast<uint16_t>(TAG_SENSORINFO)));
			if (iter == exifPropsRecs.end()) {
				Trace(ERROR) << "Couldn't find the sensor info.\n";
				return err;
			}
			Trace(DEBUG2) << "length = " << iter->length << "\n";
			Trace(DEBUG2) << "offset = " << exifProps.offset() + iter->offset << "\n";

			// go figure what the +2 is. looks like it is the byte #
			file->seek(exifProps.offset() + iter->offset + 2, SEEK_SET);
			if(!(m_container->readUInt16(file, cfa_x) 
					 && m_container->readUInt16(file, cfa_y))) {
				Trace(ERROR) << "Couldn't find the sensor size.\n";
				return err;
			}


			// locate the RAW data
			iter = std::find_if(records.begin(), records.end(), boost::bind(
														&RecordEntry::isA, _1, 
														static_cast<uint16_t>(TAG_RAWIMAGEDATA)));

			if (iter != records.end()) {
				Trace(DEBUG2) << "RAW @" << heap->offset() + (*iter).offset << "\n";
				size_t byte_size = (*iter).length;
				void *buf = data.allocData(byte_size);
				size_t real_size = (*iter).fetchData(heap.get(), buf, byte_size);
				if (real_size != byte_size) {
					Trace(WARNING) << "wrong size\n";
				}
				data.setDimensions(x, y);
				data.setDataType(OR_DATA_TYPE_COMPRESSED_CFA);

				// decompress if we need
				if((options & OR_OPTIONS_DONT_DECOMPRESS) == 0) {
					boost::scoped_ptr<IO::Stream> s(new IO::MemStream(data.data(),
																														data.size()));
					s->open(); // TODO check success
					
				  CrwDecompressor decomp(s.get(), m_container);
					
					decomp.setOutputDimensions(cfa_x, cfa_y);
					decomp.setDecoderTable(decoderTable);
					RawData *dData = decomp.decompress();
					if (dData != NULL) {
						Trace(DEBUG1) << "Out size is " << dData->x() 
													<< "x" << dData->y() << "\n";
						data.swap(*dData);
						delete dData;
					}
					data.setDataType(OR_DATA_TYPE_CFA);
				}
				err = OR_ERROR_NONE;
			}
			return err;
		}

	}
}
