

#include <fstream>
#include <cstdint>
#include <cstring>
#include <deque>
#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include <assert.h>

#include "avidef.h"
#include "common.h"
#include "writer.h"


using std::ofstream;
using std::deque;
using std::memset;


#define STDINDEXSIZE 0x4000
#ifdef WIN32
	#define filepos() tellp().seekpos()
#else
	#define filepos() tellp()
#endif


avilib::AviWriter::AviWriter()
{
	reset();
}

void avilib::AviWriter::reset()
{
	memset(&m_avimHeader, 0x0, sizeof(avilib::AVIMAINHEADER));
	m_avimHeader.fcc = 'hiva';
	m_avimHeader.cb = sizeof( avilib::AVIMAINHEADER ) - 8;
	m_avimHeader.dwFlags = AVIF_HASINDEX/* | AVIF_MUSTUSEINDEX | AVIF_TRUSTCKTYPE*/;
	m_avimHeader.dwStreams = 1;

	memset(&m_avisHeader, 0x0, sizeof(avilib::AVISTREAMHEADER));
	m_avisHeader.fcc = 'hrts';
	m_avisHeader.cb = sizeof( avilib::AVISTREAMHEADER ) - 8;
	m_avisHeader.fccType = 'sdiv';
	m_avisHeader.fccHandler = BI_RGB;
	m_avisHeader.dwScale = 1;
	m_avisHeader.dwQuality = -1;

	memset(&m_bitmapInfo, 0x0, sizeof(avilib::BITMAPINFO));
	m_bitmapInfo.biSize = sizeof( avilib::BITMAPINFO );
	m_bitmapInfo.biPlanes = 1;
	m_bitmapInfo.biBitCount = 24; // ??
	m_bitmapInfo.biCompression = BI_RGB;
}

avilib::AviWriter::~AviWriter()
{
	close();
}


uint32_t NULL32 = 0;

bool avilib::AviWriter::open( const char *filename )
{
	if( !m_propsSet )
		return false;

	int32_t i_tmp;
	ofstream::streamoff off_tmp;

	_f.open( filename, ofstream::binary|ofstream::trunc );
	if( !_f.good() ) return false;

	//RIFF ('AVI '
	_f.write( "RIFF", 4 );
	pos_RIFFSize = _f.tellp();
	_f.write( (const char *)&NULL32, 4 ); // we'll come back to this later (pos_RIFFSize)
	_f.write( "AVI ", 4 );

	//      LIST ('hdrl'
	_f.write( "LIST", 4 );

	ofstream::streampos pos_hdrlSize = _f.tellp();
	_f.write( (const char *)&NULL32, 4 );

	_f.write( "hdrl", 4 );

	//            'avih'(<Main AVI Header>)
	pos_aviMainHeader = _f.tellp();
	_f.write( (const char *)&m_avimHeader, sizeof(avilib::AVIMAINHEADER) );

	// streams begin
	for (int32_t i_stream=0; i_stream<1; i_stream++)
 	{
		_f.write( "LIST", 4 );
		ofstream::streampos pos_strlSize = _f.tellp();
		_f.write( (const char *)&NULL32, 4 );

		_f.write( "strl", 4 );
		pos_streamHeader[0] = _f.tellp();
		_f.write( (const char *)&m_avisHeader, sizeof(avilib::AVISTREAMHEADER) );

		_f.write( "strf", 4 );
		i_tmp = sizeof(avilib::BITMAPINFO);
		_f.write( (const char *)&i_tmp, 4 );
		_f.write( (const char *)&m_bitmapInfo, sizeof(avilib::tagBITMAPINFOHEADER) );

		if( m_openDML )
		{
			uint32_t num_stdIndexes = 128;
			if( m_reservedFrames != 0 ){
				num_stdIndexes = (m_reservedFrames + ((STDINDEXSIZE - sizeof(avilib::AVISTDINDEX)) / sizeof(avilib::AVISTDINDEX_ENTRY)) - 1) / ((STDINDEXSIZE - sizeof(avilib::AVISTDINDEX)) / sizeof(avilib::AVISTDINDEX_ENTRY));
				if( !num_stdIndexes ){
					m_openDML = false; // just guessing
				}
			} else {
				m_reservedFrames = num_stdIndexes * ((STDINDEXSIZE - sizeof(avilib::AVISTDINDEX)) / sizeof(avilib::AVISTDINDEX_ENTRY));
			}

			if( m_openDML )
			{
				size_t clearsize = sizeof( AVISUPERINDEX ) + num_stdIndexes*sizeof(avilib::AVISUPERINDEX_ENTRY);

				// cf. http://msdn.microsoft.com/en-us/library/windows/desktop/ff625871(v=vs.85).aspx
				m_superIdx.fcc = 'xdni';
				m_superIdx.nEntriesInUse = 1; // runtime adapted
				m_superIdx.cb = (uint32_t)clearsize - 8;
				m_superIdx.wLongsPerEntry = 4;
				m_superIdx.bIndexSubType = 0; //[ AVI_INDEX_2FIELD | 0 ]
				m_superIdx.bIndexType = AVI_INDEX_OF_INDEXES;
				m_superIdx.dwChunkId = generate_fcc( "dc", 0 );
				m_superIdx.dwReserved[0] = 0;
				m_superIdx.dwReserved[1] = 0;
				m_superIdx.dwReserved[2] = 0;

				pos_odmlSuperIdx = _f.tellp();
				uint8_t *p = new uint8_t[clearsize];
				memset(p,0x0,clearsize);
				_f.write( (const char *)p, clearsize );
				delete [] p;
				// super index later -
			}
		}

		// write strl size
		off_tmp = _f.tellp();
		uint32_t ui_strlSize = (uint32_t)(_f.tellp() - pos_strlSize) - 4;
		_f.seekp(pos_strlSize);
		_f.write( (const char *)&ui_strlSize, 4);
		_f.seekp(off_tmp);

		if( m_openDML )
		{
			_f.write( "LIST", 4 );
			i_tmp = 248 + 12;
			_f.write( (const char *)&i_tmp, 4 );
			_f.write( "odml", 4 );
			_f.write( "dmlh", 4 );
			i_tmp = 248;
			_f.write( (const char *)&i_tmp, 4 );
			pos_odmlExt = _f.tellp();
			uint8_t *p = new uint8_t[248];
			avilib::ODMLExtendedAVIHeader odml_ext = { 0 };
			memset(p,0x0,248);
			memcpy(p,&odml_ext,sizeof(avilib::ODMLExtendedAVIHeader));
			_f.write((const char *)p, 248);
			delete [] p;
		}
	} // streams

	// write hdrl size
	off_tmp = _f.tellp();
	uint32_t ui_hdrlSize = (uint32_t)(_f.tellp() - pos_hdrlSize) - 4;
	_f.seekp(pos_hdrlSize);
	_f.write( (const char *)&ui_hdrlSize, 4);
	_f.seekp(off_tmp);


	// movi LIST
	_f.write( "LIST", 4 );
	pos_moviListSize = _f.tellp();
	_f.write( (const char *)&NULL32, 4 ); // later ...
	// m_currMoviListSize = 4; // needed?
	pos_1stMoviStart = _f.tellp();
	_f.write( "movi", 4 );

	m_RIFF_size = (uint32_t)_f.tellp() + sizeof( avilib::AVIOLDINDEX );
	m_totalFrames = 0;
	m_currBaseOff = 0;

	if( m_openDML ) // FIXME: support throughout
	{
		avilib::AVISTDINDEX std_index;
		std_index.fcc = '00xi';
		std_index.wLongsPerEntry = sizeof(avilib::AVISTDINDEX_ENTRY)/sizeof(DWORD);
		std_index.bIndexSubType = 0; // must be 0
		std_index.dwChunkId = generate_fcc( "dc", 0 );
		std_index.qwBaseOffset = 0; // all dwOffsets in aIndex array are relative to this
		std_index.dwReserved = 0; // must be 0
		m_stdIndexes.push_back( std_index );
	}


	return _f.good();
}

bool avilib::AviWriter::close()
{
	if( _f.bad() ) return false;

	_f.seekp(pos_aviMainHeader);
	_f.write( (const char *)&m_avimHeader, sizeof(avilib::AVIMAINHEADER) );

	if( m_openDML )
	{
		// write odml total frame count
		_f.seekp(pos_odmlExt);
		avilib::ODMLExtendedAVIHeader odml_ext = { static_cast<DWORD>(m_totalFrames) };
		_f.write((const char *)&odml_ext, sizeof(avilib::ODMLExtendedAVIHeader));

		// write "00xi"s at the very end
		deque<avilib::AVISUPERINDEX_ENTRY> deq_supIdxEntries;
		for_each( m_stdIndexEntries.begin(), m_stdIndexEntries.end(), [&,this]( const std::pair<int32_t, deque<avilib::AVISTDINDEX_ENTRY>> &pair )
		{
			avilib::AVISTDINDEX &std_index = m_stdIndexes[pair.first];
			const deque<avilib::AVISTDINDEX_ENTRY> &deq_entries = pair.second;

			std_index.bIndexType = AVI_INDEX_OF_CHUNKS;
			std_index.nEntriesInUse = (uint32_t)deq_entries.size();
			std_index.cb = (uint32_t)(sizeof(avilib::AVISTDINDEX)-8 + sizeof(avilib::AVISTDINDEX_ENTRY)*deq_entries.size());

			_f.seekp( 0, ofstream::end );
			uint64_t pos_stdindex = _f.filepos();
			assert(_f.good());

			_f.write( (const char *)&std_index, sizeof(avilib::AVISTDINDEX) );
			
			uint8_t *p = new uint8_t[sizeof(avilib::AVISTDINDEX_ENTRY)*deq_entries.size()], *p0 = p;
			for_each( deq_entries.begin(), deq_entries.end(), [&]( const avilib::AVISTDINDEX_ENTRY &entry ){
				*(avilib::AVISTDINDEX_ENTRY *)p = entry;
				p += sizeof(avilib::AVISTDINDEX_ENTRY);
			});
			_f.write( (const char *)p0, sizeof(avilib::AVISTDINDEX_ENTRY)*deq_entries.size() );
			delete [] p0;

			avilib::AVISUPERINDEX_ENTRY supidx_entry;
			supidx_entry.qwOffset = pos_stdindex;
			supidx_entry.dwSize = std_index.cb+8;
			supidx_entry.dwDuration = static_cast<DWORD>(deq_entries.size()); // frame count (video)
			deq_supIdxEntries.push_back(supidx_entry);
		});

		// write that list to superindex
		_f.seekp( pos_odmlSuperIdx, ofstream::beg );
		_f.write( (const char *)&m_superIdx, sizeof(AVISUPERINDEX) );
		for_each( deq_supIdxEntries.begin(), deq_supIdxEntries.end(), [&]( avilib::AVISUPERINDEX_ENTRY &supidx_entry ){
			_f.write( (const char *)&supidx_entry, sizeof(avilib::AVISUPERINDEX_ENTRY));
		});
	}



	// write stream header
	_f.seekp( pos_streamHeader[0], ofstream::beg );
	_f.write( (const char *)&m_avisHeader, sizeof(avilib::AVISTREAMHEADER) );

	if( m_RIFF_idx == 0 )
	{
		finish_old();
	}
	else
	{
		// write movi size without final "ix00"s
		_f.seekp( 0, ofstream::end );
		uint32_t riffAvixMoviSize = (uint32_t)((uint64_t)_f.filepos() - pos_moviListSize - 4);
		_f.seekp( pos_moviListSize, ofstream::beg );
		_f.write( (const char *)&riffAvixMoviSize, 4 );

		_f.seekp( 0, ofstream::end );
		uint32_t riffAvixSize = (uint32_t)((uint64_t)_f.filepos() - pos_RIFFSize - 4);
		
		_f.seekp( pos_RIFFSize, ofstream::beg );
		_f.write( (const char *)&riffAvixSize, 4 );
	}

	reset();
	_f.close();
	return true;
}

void avilib::AviWriter::finish_avix()
{
	uint32_t riffAvixSize, riffAvixMoviSize;
	_f.seekp( 0, ofstream::end );
	riffAvixSize = (uint32_t)((uint64_t)_f.tellp() - pos_RIFFSize - 4);
	riffAvixMoviSize = (uint32_t)((uint64_t)_f.tellp() - pos_moviListSize - 4);

	_f.seekp( pos_RIFFSize, ofstream::beg );
	_f.write( (const char *)&riffAvixSize, 4 );

	_f.seekp( pos_moviListSize, ofstream::beg );
	_f.write( (const char *)&riffAvixMoviSize, 4 );
}


void avilib::AviWriter::finish_old()
{
	// write old index
	// write stream size
	for( int32_t stream_idx = 0; stream_idx < 1; stream_idx++ )
	{
		_f.seekp( 0, ofstream::end );
		uint64_t moviSize = (uint64_t)_f.filepos() - pos_moviListSize - 4;
		_f.seekp( pos_moviListSize, ofstream::beg );
		_f.write( (const char *)&moviSize, 4 );
	}

	_f.seekp( 0, ofstream::end );
	_f.clear();
	uint32_t sz = (uint32_t)(m_oldIndexEntries.size() * sizeof( avilib::AVIOLDINDEX ));
	_f.write( "idx1", 4 );
	_f.write((const char *)&sz,4);

	for_each( m_oldIndexEntries.begin(), m_oldIndexEntries.end(), [&]( avilib::AVIOLDINDEX &entry ){
		_f.write( (const char *)&entry, sizeof(avilib::AVIOLDINDEX) );
	});

	// write file size right after "RIFF"
	_f.seekp( 0, ofstream::end );
	uint32_t s_ = (uint32_t)_f.tellp() - 8;
	_f.seekp( pos_RIFFSize, ofstream::beg );
	_f.write( (const char *)&s_, 4 );
}

bool avilib::AviWriter::write_frame( uint8_t stream_idx, void *data )
{
	if( _f.bad() ) return false;
	char message[32];

	if( m_reservedFrames <= m_totalFrames ) return false;

	uint32_t ui_next_RIFF_size = m_RIFF_size + m_bitmapInfo.biSizeImage + 8;
	if( ui_next_RIFF_size >= m_currMaxRIFFSize )
	{
		if( m_RIFF_idx == 0 ){
			// write old index
			finish_old();
			m_currMaxRIFFSize = 4ULL * 1000*1000*1000;
		} else {
			finish_avix();
		}

		_f.seekp(0,ofstream::end);

		sprintf( message, "#%u: RIFFSize = %u", m_totalFrames, m_RIFF_size);
		log( message );

		m_RIFF_idx++;
		m_RIFF_size = 24;
		_f.write( "RIFF", 4 );
		pos_RIFFSize = _f.filepos();
		_f.write( (const char *)&NULL32, 4 ); // we'll come back to this later (pos_RIFFSize)
		_f.write( "AVIX", 4 );

		_f.write( "LIST", 4 );
		pos_moviListSize = _f.filepos();
		_f.write( (const char *)&NULL32, 4 ); // later ... FIXME
		_f.write( "movi", 4 );
	}

	m_RIFF_size += m_bitmapInfo.biSizeImage + 8;

	if( m_RIFF_idx == 0 )
	{
		m_RIFF_size += sizeof(avilib::AVIOLDINDEX);
		std::ofstream::streamoff framePosforOldIndex = _f.tellp();
		framePosforOldIndex -= pos_1stMoviStart; // relative to first "movi"

		avilib::AVIOLDINDEX entry;
		entry.dwChunkId = generate_fcc( "dc", 0 );
		entry.dwFlags = AVIIF_KEYFRAME;
		entry.dwOffset = (DWORD)framePosforOldIndex;
		entry.dwSize = m_bitmapInfo.biSizeImage;
		m_oldIndexEntries.push_back( entry );

	}

	if( (m_stdIndexEntries[(int32_t)m_stdIndexes.size()-1].size() + 1U) * sizeof(avilib::AVISTDINDEX_ENTRY) + sizeof(avilib::AVISTDINDEX) >= STDINDEXSIZE )
	{
		avilib::AVISTDINDEX std_index;
		std_index.fcc = '00xi';
		std_index.wLongsPerEntry = sizeof(avilib::AVISTDINDEX_ENTRY)/sizeof(DWORD);
		std_index.bIndexSubType = 0; // must be 0
		std_index.dwChunkId = generate_fcc( "dc", 0 );
		std_index.qwBaseOffset = (uint64_t)_f.filepos(); // all dwOffsets in aIndex array are relative to this
		std_index.dwReserved = 0; // must be 0
		m_stdIndexes.push_back( std_index );
		
		m_currBaseOff = std_index.qwBaseOffset;

		m_superIdx.nEntriesInUse++;
	}

	avilib::AVISTDINDEX_ENTRY std_entry = { static_cast<DWORD>((uint64_t)_f.tellp() + 8 - m_currBaseOff), m_bitmapInfo.biSizeImage };
	m_stdIndexEntries[(int32_t)m_stdIndexes.size()-1].push_back( std_entry );

	_f.write( "00dc", 4 );
	_f.write( (const char *)&m_bitmapInfo.biSizeImage, 4 );
	_f.write( (const char *)data, m_bitmapInfo.biSizeImage );


	m_avimHeader.dwTotalFrames++;
	m_avisHeader.dwLength++;
	m_totalFrames++;
	
	return true;
}


bool avilib::AviWriter::setVideoProperties( uint8_t stream_idx, int32_t width, int32_t height, uint32_t codec, uint32_t framesize, double rateHz )
{
	if( m_opened )
		return false;

	uint32_t Tmicr = int32_t( .5 + 1000000. / rateHz );

	m_avimHeader.dwMicroSecPerFrame = Tmicr;
	m_avimHeader.dwMaxBytesPerSec = DWORD( .5 + framesize * rateHz );

	int32_t rate, scale;
	if( avilib::cancel( int32_t( .5 + 1000. * rateHz ), 1000, rate, scale ) )
	{
		m_avisHeader.dwScale = scale;
		m_avisHeader.dwRate = rate;
	}
	else
	{
		m_avisHeader.dwScale = 1;
		m_avisHeader.dwRate = uint32_t( .5 + rateHz );
	}

	m_avimHeader.dwMaxBytesPerSec = framesize * uint32_t( .5 + rateHz );
	m_avimHeader.dwSuggestedBufferSize = framesize;
	m_avimHeader.dwWidth = width;
	m_avimHeader.dwHeight = height;

	m_avisHeader.fccType = 'sdiv';
	m_avisHeader.fccHandler._ = codec;
	m_avisHeader.dwSuggestedBufferSize = framesize;
	m_avisHeader.rcFrame.top = 
	m_avisHeader.rcFrame.left = 0;
	m_avisHeader.rcFrame.bottom = height;
	m_avisHeader.rcFrame.right = width;

	m_bitmapInfo.biWidth = width;
	m_bitmapInfo.biHeight = height;
	m_bitmapInfo.biCompression = codec;
	m_bitmapInfo.biSizeImage = framesize;


	m_propsSet = true;
	return true;
}

bool avilib::AviWriter::setAudioProperties(uint8_t stream_idx, int16_t format, int8_t channels, uint32_t samplesPerSecond, uint32_t avgBytesPerSecond, uint16_t bitsPerSample, uint16_t blockAlign )
{
	if( m_opened )
		return false;

	m_waveFormat.wFormatTag = format;
	m_waveFormat.nChannels = channels;
	m_waveFormat.nSamplesPerSec = channels;
	m_waveFormat.nAvgBytesPerSec = avgBytesPerSecond;
	m_waveFormat.nBlockAlign = blockAlign;
	m_waveFormat.wBitsPerSample = bitsPerSample;
	m_waveFormat.cbSize = 0;

	m_propsSet = true;
	return true;
}

bool avilib::AviWriter::setFrameCount( uint8_t stream_idx, uint32_t count )
{
	if( m_opened )
		return false;

	m_reservedFrames = count;

	return true;
}


