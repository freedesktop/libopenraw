/* -*- tab-width:4; c-basic-offset:4 -*- */
/*
 * libopenraw - neffile.cpp
 *
 * Copyright (C) 2006-2008, 2012-2013 Hubert Figuiere
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


#include <iostream>
#include <vector>

#include <libopenraw/cameraids.h>
#include <libopenraw++/thumbnail.h>
#include <libopenraw++/rawdata.h>

#include "trace.h"
#include "ifd.h"
#include "ifdfilecontainer.h"
#include "ifddir.h"
#include "ifdentry.h"
#include "io/file.h"
#include "huffman.h"
#include "makernotedir.h"
#include "nefdiffiterator.h"
#include "nefcfaiterator.h"
#include "neffile.h"
#include "rawfile_private.h"

using namespace Debug;

namespace OpenRaw {

namespace Internals {

#define OR_MAKE_NIKON_TYPEID(camid) \
    OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_NIKON,camid)

/* taken from dcraw, by default */
static const BuiltinColourMatrix s_matrices[] = {
    { OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D1), 0, 0, /* multiplied by 2.218750, 1.0, 1.148438 */
      { 16772,-4726,-2141,-7611,15713,1972,-2846,3494,9521 } },
    { OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D100), 0, 0,
      { 5902,-933,-782,-8983,16719,2354,-1402,1455,6464 } },
    { OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D1X), 0, 0,
      { 7702,-2245,-975,-9114,17242,1875,-2679,3055,8521 } },
    { OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D200), 0, 0xfbc,
      { 8367,-2248,-763,-8758,16447,2422,-1527,1550,8053 } },
    { OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D2H), 0, 0,
      { 5710,-901,-615,-8594,16617,2024,-2975,4120,6830 } },
    { OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D2X), 0, 0,
      { 10231,-2769,-1255,-8301,15900,2552,-797,680,7148 } },

    { OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D3), 0, 0,
      { 8139,-2171,-663,-8747,16541,2295,-1925,2008,8093 } },
    { OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D3S), 0, 0,
      { 8828,-2406,-694,-4874,12603,2541,-660,1509,7587 } },
    { OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D3X), 0, 0,
      { 7171,-1986,-648,-8085,15555,2718,-2170,2512,7457 } },
    { OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D300), 0, 0,
      { 9030,-1992,-715,-8465,16302,2255,-2689,3217,8069 } },
    { OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D300S), 0, 0,
      { 9030,-1992,-715,-8465,16302,2255,-2689,3217,8069 } },
    { OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D3000), 0, 0,
      { 8736,-2458,-935,-9075,16894,2251,-1354,1242,8263 } },
    { OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D3100), 0, 0,
      { 7911,-2167,-813,-5327,13150,2408,-1288,2483,7968 } },
    // From DNG Converter 7.1-rc
    { OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D3200), 0, 0,
      { 7911, -2167, -813, -5327, 1315, 2408, -1288, 2483, 7968 } },
    // From DNG Converter 7.1-rc
    { OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D4), 0, 0,
      { 8598, -2848, -857, -5618, 13606, 2195, -1002, 1773, 7137 } },
    { OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D40), 0, 0,
      { 6992,-1668,-806,-8138,15748,2543,-874,850,7897 } },
    { OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D40X), 0, 0,
      { 8819,-2543,-911,-9025,16928,2151,-1329,1213,8449 } },
    { OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D50), 0, 0,
      { 7732,-2422,-789,-8238,15884,2498,-859,783,7330 } },
    { OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D5000), 0, 0xf00,
      { 7309,-1403,-519,-8474,16008,2622,-2433,2826,8064 } },
    { OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D5100), 0, 0x3de6,
      { 8198,-2239,-724,-4871,12389,2798,-1043,2050,7181 } },
    { OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D600), 0, 0,
      { 8139,-2171,-663,-8747,16541,2295,-1925,2008,8093 } },
    { OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D70), 0, 0,
      { 7732,-2422,-789,-8238,15884,2498,-859,783,7330 } },
    { OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D70S), 0, 0,
      { 7732,-2422,-789,-8238,15884,2498,-859,783,7330 } },
    { OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D700), 0, 0,
      { 8139,-2171,-663,-8747,16541,2295,-1925,2008,8093 } },
    { OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D7000), 0, 0,
      { 8198,-2239,-724,-4871,12389,2798,-1043,2050,7181 } },
    { OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D80), 0, 0,
      { 8629,-2410,-883,-9055,16940,2171,-1490,1363,8520 } },
    // From DNG Converter 7.1-rc
    { OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D800), 0, 0,
      { 7866, -2108, -555, -4869, 12483, 2681, -1176, 2069, 7501 } },
    // From DNG Converter 7.1-rc
    { OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D800E), 0, 0,
      { 7866, -2108, -555, -4869, 12483, 2681, -1176, 2069, 7501 } },
    { OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D90), 0, 0xf00,
      { 7309,-1403,-519,-8474,16008,2622,-2434,2826,8064 } },
    { OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_COOLPIX_P6000), 0, 0,
      { 9698,-3367,-914,-4706,12584,2368,-837,968,5801 } },
    { OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_COOLPIX_P7000), 0, 0,
      { 11432,-3679,-1111,-3169,11239,2202,-791,1380,4455 } },
    { OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_COOLPIX_P7100), 0, 0,
      { 11053,-4269,-1024,-1976,10182,2088,-526,1263,4469 } },
    { OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_COOLPIX_P7700), 0, 0,
      { 10321,-3920,-931,-2750,11146,1824,-442,1545,5539 } },
    { OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_1_J1), 0, 0,
      { 8994,-2667,-865,-4594,12324,2552,-699,1786,6260 } },
    { OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_1_V1), 0, 0,
      { 8994,-2667,-865,-4594,12324,2552,-699,1786,6260 } },
    { OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_1_V2), 0, 0,
      { 6588,-1305,-693,-3277,10987,2634,-355,2016,5106 } },

    { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } }
};

const IfdFile::camera_ids_t NefFile::s_def[] = {
    { "NIKON D1 ", OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D1) },
    { "NIKON D100 ", OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D100) },
    { "NIKON D1X", OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D1X) },
    { "NIKON D200", OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D200) },
    { "NIKON D2H", OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D2H ) },
    { "NIKON D2X", OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D2X ) },
    { "NIKON D3", OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D3) },
    { "NIKON D3S", OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D3S) },
    { "NIKON D3X", OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D3X) },
    { "NIKON D300", OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D300) },
    { "NIKON D300S", OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D300S) },
    { "NIKON D3000", OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D3000) },
    { "NIKON D3100", OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D3100) },
    { "NIKON D3200", OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D3200) },
    { "NIKON D4", OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D4) },
    { "NIKON D40", OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D40) },
    { "NIKON D40X", OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D40X) },
    { "NIKON D50", OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D50) },
    { "NIKON D5000", OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D5000) },
    { "NIKON D5100", OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D5100) },
    { "NIKON D5200", OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D5100) },
    { "NIKON D600", OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D600) },
    { "NIKON D70", OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D70) },
    { "NIKON D70s", OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D70S) },
    { "NIKON D700", OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D700) },
    { "NIKON D7000", OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D7000) },
    { "NIKON D7100", OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D7100) },
    { "NIKON D80", OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D80) },
    { "NIKON D800", OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D800) },
    { "NIKON D800E", OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D800E) },
    { "NIKON D90", OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_D90) },
    { "COOLPIX P6000", OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_COOLPIX_P6000) },
    { "COOLPIX P7000", OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_COOLPIX_P7000) },
    { "COOLPIX P7100", OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_COOLPIX_P7100) },
    { "COOLPIX P7700", OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_COOLPIX_P7700) },
    { "COOLPIX A", OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_COOLPIX_A) },
    { "NIKON 1 J1", OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_1_J1) },
    { "NIKON 1 J2", OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_1_J2) },
    { "NIKON 1 J3", OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_1_J3) },
    { "NIKON 1 V1", OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_1_V1) },
    { "NIKON 1 V2", OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_1_V2) },
    { "NIKON 1 S1", OR_MAKE_NIKON_TYPEID(OR_TYPEID_NIKON_1_S1) },
    { 0, 0 }
};

RawFile *NefFile::factory(IO::Stream* _filename)
{
    return new NefFile(_filename);
}

NefFile::NefFile(IO::Stream* _filename)
    : TiffEpFile(_filename, OR_RAWFILE_TYPE_NEF)
{
    _setIdMap(s_def);
    _setMatrices(s_matrices);
}


NefFile::~NefFile()
{
}

bool NefFile::isCompressed(RawContainer & container, uint32_t offset)
{
    int i;
    uint8_t buf[256];
    size_t real_size = container.fetchData(buf, offset, 
                                           256);
    if(real_size != 256) {
        return true;
    }
    for(i = 15; i < 256; i+= 16) {
        if(buf[i]) {
            Trace(DEBUG1) << "isCompressed: true\n";
            return true;
        }
    }
    Trace(DEBUG1) << "isCompressed: false\n";
    return false;
}

MakerNoteDir::Ref NefFile::_locateMakerNoteIfd()
{
    const IfdDir::Ref & _exifIfd = exifIfd();
    if(!_exifIfd) {
        return MakerNoteDir::Ref();
    }
	
    IfdEntry::Ref maker_ent =
        _exifIfd->getEntry(IFD::EXIF_TAG_MAKER_NOTE);
    if(!maker_ent) {
        return MakerNoteDir::Ref();
    }
	
    uint32_t off = maker_ent->offset();
    uint32_t base = off + 10;
	
    MakerNoteDir::Ref ref(new MakerNoteDir(base + 8, *m_container, base));
    ref->load();
    return ref;
}

::or_error NefFile::_decompressNikonQuantized(RawData & data)
{
    NEFCompressionInfo c;
    if (!_getCompressionCurve(data, c)) {
        Trace(ERROR) << "compression curve not found\n";
        return OR_ERROR_NOT_FOUND;
    }
    const uint32_t rows = data.height();
    const uint32_t raw_columns = data.width();
    
    //FIXME: not always true
    const uint32_t columns = raw_columns - 1;
    
    NefDiffIterator
        diffs(c.huffman, data.data());
    NefCfaIterator iter(diffs, rows, raw_columns, c.vpred);
    
    RawData newData;
    uint16_t *p = (uint16_t *) newData.allocData(rows * columns * 2);
    newData.setDimensions(columns, rows);
    newData.setDataType(OR_DATA_TYPE_RAW);
    uint16_t bpc = data.bpc();
    newData.setBpc(bpc);
    newData.setMax((1 << bpc) - 1);
    newData.setCfaPatternType(data.cfaPattern()->patternType());
	
    for (unsigned int i = 0; i < rows; i++) {
        for (unsigned int j = 0; j < raw_columns; j++) {
            uint16_t t = iter.get();
            if (j < columns) {
                unsigned shift = 16 - data.bpc();
                p[i * columns + j] =  c.curve[t & 0x3fff] << shift;
            }
        }
    }
    
    data.swap(newData);
    return OR_ERROR_NONE;
}

::or_error NefFile::_decompressIfNeeded(RawData & data,
                                        uint32_t options)
{
    uint32_t compression = data.compression();
    if((options & OR_OPTIONS_DONT_DECOMPRESS) ||
       compression == IFD::COMPRESS_NONE) {
        return OR_ERROR_NONE;
    } else if(compression == IFD::COMPRESS_NIKON_QUANTIZED) {
        return _decompressNikonQuantized(data);
    } else {
        return OR_ERROR_INVALID_FORMAT;
    }
}

int NefFile::_getCompressionCurve(RawData & data,  NefFile::NEFCompressionInfo& c)
{
    MakerNoteDir::Ref _makerNoteIfd = makerNoteIfd();
    if(!_makerNoteIfd) {
        Trace(ERROR) << "makernote not found\n";
        return 0;
    }
    IfdEntry::Ref curveEntry = _makerNoteIfd->getEntry(IFD::MNOTE_NIKON_NEFDECODETABLE2);
    if(!curveEntry) {
        Trace(ERROR) << "decode table2 tag not found\n";
        return 0;
    }

    size_t pos = _makerNoteIfd->getMnoteOffset() + curveEntry->offset();

    IO::Stream *file = m_container->file();
    file->seek(pos, SEEK_SET);

    uint8_t header0, header1;
    bool read = m_container->readUInt8(file, header0);
    if(!read) {
        Trace(ERROR) << "Header not found\n";
        return 0;
    }
    read = m_container->readUInt8(file, header1);
    if(!read) {
        Trace(ERROR) << "Header not found\n";
        return 0;
    }

    bool header_ok = false;
    if (header0 == 0x44) {
        if (header1 == 0x10) {
            c.huffman = NefDiffIterator::Lossy12Bit;
            data.setBpc(12);
            header_ok = true;
        } else if (header1 == 0x20) {
            c.huffman = NefDiffIterator::Lossy14Bit;
            data.setBpc(14);
            header_ok = true;
        }
    } else if (header0 == 0x46 && header1 == 0x30) {
        c.huffman = NefDiffIterator::LossLess14Bit;
        data.setBpc(14);
        header_ok = true;
    }
    if (!header_ok) {
        Trace(ERROR) << "Wrong header, found " << header0 << "-"
                     << header1 << "\n";
        return 0;
    }

    int16_t aux;

    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            read = m_container->readInt16(file, aux);
            if(!read) {
                return 0;
            }
            c.vpred[i][j] = aux;
        }
    }

    if (header0 == 0x44) {
        size_t nelems;
        read = m_container->readInt16(file, aux);
        nelems = aux;

        for (size_t i = 0; i < nelems; ++i) {
            read = m_container->readInt16(file, aux);
            if (!read)
                return 0;
            c.curve.push_back(aux);
        }
    } else if (header0 == 0x46 && header1 == 0x30) {
        for (size_t i = 0; i <= 0x3fff; ++i) {
            c.curve.push_back(i);
        }
    }

    return 1;
}

}
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0))
  indent-tabs-mode:nil
  fill-column:80
  End:
*/
