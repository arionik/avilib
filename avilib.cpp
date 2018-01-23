
#include <cstdint>
#include <string>
#include <deque>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>

#include "avilib.h"
#include "avidef.h"
#include "common.h"
#include "writer.h"
#include "reader.h"


AVI_DLL_DECL void *avireader_create(){
	return (void *)new avilib::AviReader;
}
AVI_DLL_DECL void avireader_destroy( void *p_context ){
	delete (avilib::AviReader *)p_context;
}
AVI_DLL_DECL int32_t avireader_open( void *p_context, const char *filename ){
	return ((avilib::AviReader *)p_context)->open( filename ) ? 1 : 0;
}
AVI_DLL_DECL int32_t avireader_close( void *p_context ){
	return ((avilib::AviReader *)p_context)->close() ? 1 : 0;
}
AVI_DLL_DECL int32_t avireader_get_stream_count( void *p_context, uint32_t *count ){
	return ((avilib::AviReader *)p_context)->getStreamCount( *count ) ? 1 : 0;
}
AVI_DLL_DECL int32_t avireader_read_frame( void *p_context, uint32_t idx, uint32_t stream, void *data ){
	return ((avilib::AviReader *)p_context)->read_frame( idx, stream, (uint8_t *)data ) ? 1 : 0;
}
AVI_DLL_DECL int32_t avireader_get_size( void *p_context, int32_t *width, int32_t *height ){
	return ((avilib::AviReader *)p_context)->getSize( *width, *height ) ? 1 : 0;
}
AVI_DLL_DECL int32_t avireader_get_vrate( void *p_context, double *rateHz ){
	return ((avilib::AviReader *)p_context)->getFrameRate( *rateHz ) ? 1 : 0;
}
AVI_DLL_DECL int32_t avireader_get_frame_count( void *p_context, uint32_t stream, uint32_t *count ){
	return ((avilib::AviReader *)p_context)->getFrameCount( stream, *count ) ? 1 : 0;
}
AVI_DLL_DECL int32_t avireader_get_codec( void *p_context, uint32_t stream, uint32_t *codec ){
	return ((avilib::AviReader *)p_context)->getCodec( stream, *codec ) ? 1 : 0;
}
AVI_DLL_DECL int32_t avireader_get_alloc_size( void *p_context, uint32_t stream, uint32_t *size ){
	return ((avilib::AviReader *)p_context)->getAllocSize( stream, *size ) ? 1 : 0;
}
AVI_DLL_DECL int32_t avireader_get_vformat( void *p_context, uint32_t stream, avilib_BITMAPINFO *bm_info ){
	return ((avilib::AviReader *)p_context)->getFormat( stream, *bm_info ) ? 1 : 0;
}
AVI_DLL_DECL int32_t avireader_get_aformat( void *p_context, uint32_t stream, avilib_WAVEFORMATEX *a_info ){
	return ((avilib::AviReader *)p_context)->getFormat( stream, *a_info ) ? 1 : 0;
}
AVI_DLL_DECL int32_t avireader_get_stream_type( void *p_context, uint32_t stream, avilib_streamtype_t *type ){
	return ((avilib::AviReader *)p_context)->getStreamType( stream, *type ) ? 1 : 0;
}









AVI_DLL_DECL void *aviwriter_create(){
	return (void *)new avilib::AviWriter;
}
AVI_DLL_DECL void aviwriter_destroy( void *p_context ){
	delete (avilib::AviWriter *)p_context;
}
AVI_DLL_DECL int32_t aviwriter_open( void *p_context, const char *filename ){
	return ((avilib::AviWriter *)p_context)->open( filename ) ? 1 : 0;
}
AVI_DLL_DECL int32_t aviwriter_close( void *p_context ){
	return ((avilib::AviWriter *)p_context)->close() ? 1 : 0;
}
AVI_DLL_DECL int32_t aviwriter_write_frame( void *p_context, uint8_t stream_idx, void *data ){
	return ((avilib::AviWriter *)p_context)->write_frame( stream_idx, data ) ? 1 : 0;
}
AVI_DLL_DECL int32_t aviwriter_set_vprops( void *p_context, uint8_t stream_idx, int32_t width, int32_t height, uint32_t codec, uint32_t framesize, double rateHz ){
	return ((avilib::AviWriter *)p_context)->setVideoProperties( stream_idx, width, height, codec, framesize, rateHz ) ? 1 : 0;
}
AVI_DLL_DECL int32_t aviwriter_set_frame_count( void *p_context, uint8_t stream_idx, uint32_t count ){
	return ((avilib::AviWriter *)p_context)->setFrameCount( stream_idx, count ) ? 1 : 0;
}







int32_t avilib::gcd( int32_t nom, int32_t denom )
{
	if( nom < denom ) {
		int32_t tmp = nom;
		nom = denom;
		denom = tmp;
	}
	if( !(nom % denom) )
		return denom;
	return gcd( denom, nom % denom );
}

bool avilib::cancel( int32_t nom, int32_t denom, int32_t &out_nom, int32_t &out_denom )
{
	if( !denom || !nom ) return false;
	int32_t _gcd = gcd( nom, denom );

	if( _gcd ) {
		out_nom = nom / _gcd;
		out_denom = denom / _gcd;
		return true;
	}

	return false;
}


#include <iostream>
#include <string>

using std::string;
using std::cout;
using std::endl;

void avilib::LoggingObject::log( const string &message ){
	#ifdef DEBUG
	cout << message << endl;
	#endif
	m_messages.push_back(message);
}
bool avilib::LoggingObject::getNextMessage( string &message ){
	if(m_messages.empty()) return false;
	message = m_messages.front();
	m_messages.pop_front();
	return true;
}
void avilib::LoggingObject::clear_log(){
	m_messages.clear();
}




avilib::FOURCC avilib::generate_fcc( std::string id, uint8_t idx )
{
	char buff[3] = {0};
	avilib::FOURCC ret = {0};
	sprintf( buff, "%02i", idx > 99 ? 99 : idx < 0 ? 0 : idx);
	ret.fcc[0] = buff[0];
	ret.fcc[1] = buff[1];
	ret.fcc[2] = id.c_str()[0];
	ret.fcc[3] = id.c_str()[1];
	return ret;
}
