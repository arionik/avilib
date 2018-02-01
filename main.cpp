
#include "avilib.h"
//or #include <avilib.h>

#ifdef WIN32
	#include <windows.h>
#endif
#include <stdio.h>
#include <stdint.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <regex>

using std::cerr;
using std::endl;
using std::string;
using std::ifstream;
using std::ostringstream;
using std::ofstream;
using std::regex;
using std::smatch;
using std::regex_search;

int main( int argc, char *argv[] )
{
	int32_t w = 0,h = 0;
	uint32_t codec,alloc_size,fcount,streams;
	uint8_t *buffer;

	int32_t maj, min, rev;
	const char *string_ver;
	avilib_version( &maj, &min, &rev, &string_ver );
	cerr << string_ver << endl;

	if( argc < 3 ) {
		cerr << "Usage: aviapp <<input>> <<output>>" << endl;
		exit( 1 );
	}

	string input = argv[ 1 ],
		output = argv[ 2 ];
	bool read = !input.compare( input.size()-3, 3, "avi" );
	
	if( read ){
		
		double rateHz;
		uint32_t read_size;
		avireader_t *p_reader = avireader_create();
		if( !avireader_open(p_reader, input.c_str() ) ) {
			cerr << "Error opening file" << endl;
			return -1;
		}

		avilib_BITMAPINFO bm_info = {0};
		avilib_WAVEFORMATEX wav_info = {0};
		avilib_streamtype_t streamtype = avilib_UnknownStreamtype;
		avilib_streamtype_t desired_streamtype = avilib_Video;

		uint32_t i_stream = ~0u;
		while(streamtype != desired_streamtype)
			avireader_get_stream_type( p_reader, ++i_stream, &streamtype );
		avireader_get_stream_count( p_reader, &streams );
		avireader_get_stream_type( p_reader, i_stream, &streamtype );
		if( streamtype == avilib_Video )
			avireader_get_vformat( p_reader, i_stream, &bm_info );
		else
			avireader_get_aformat( p_reader, i_stream, &wav_info );
		avireader_get_size( p_reader, &w, &h );
		avireader_get_vrate( p_reader, &rateHz );
		avireader_get_codec( p_reader, i_stream, &codec );
		avireader_get_alloc_size( p_reader, i_stream, &alloc_size );
		avireader_get_frame_count( p_reader, i_stream, &fcount );

		ofstream out( output.c_str(), ofstream::binary|ofstream::trunc );
		if( !out.good() ) return -1;

		buffer = new uint8_t[ alloc_size ];
		uint32_t frame_idx = 0;

		while( (read_size = avireader_read_frame( p_reader, frame_idx++, i_stream, buffer )) )
			out.write( (char *)buffer, read_size );

		if( fcount != frame_idx-1 )
			cerr << "Said: " << fcount << ", is: " << frame_idx-1 << endl;

		avireader_destroy( p_reader );

	} else {

		smatch what;
		regex reg_exWH( "([1-9][0-9]{0,5})x([1-9][0-9]{0,5})" );
		if( regex_search( input, what, reg_exWH ) ) {
			w = atoi( what[1].str().c_str() );
			h = atoi( what[2].str().c_str() );
		}
		uint32_t sz = w*h * 3 / 2u;

		// http://www.jmcgowan.com/avicodecs.html
		aviwriter_t *p_writer = aviwriter_create();
		aviwriter_set_vprops(p_writer, 0, w, h, (uint32_t)'024I', sz, 25.);
		aviwriter_set_frame_count(p_writer, 0, 15000);

		if( !aviwriter_open( p_writer, output.c_str() ) ) {
			cerr << "Error opening file" << endl;
			return -1;
		}
		
		ifstream in( input, ifstream::binary );
		if( !in.good() ) return -1;
		
		in.seekg( 0*sz );
		buffer = new uint8_t[ sz ];
		
		for( int32_t i=0; i<15000; i++ ){
			if(in.eof()){
				in.clear();
				in.seekg(0);
			}
			in.read( (char *)buffer, sz );
			aviwriter_write_frame( p_writer, 0, buffer );
		}
		aviwriter_destroy(p_writer);
	}

	delete [] buffer;
	
	return 0;
}
