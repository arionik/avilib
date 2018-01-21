
#include <stdint.h>
#ifdef __GNUC__
	#include <cstddef> 
	#include <inttypes.h>
	#define AVI_DLL_DECL
#else
	#ifndef _LIB
		#ifndef AVI_IMPORT
			#define AVI_DLL_DECL __declspec(dllexport)
		#else
			#define AVI_DLL_DECL
		#endif
	#else
		#define AVI_DLL_DECL
	#endif
#endif

typedef struct
{
	uint32_t biSize;
	uint32_t biWidth;
	uint32_t biHeight;
	uint16_t biPlanes;
	uint16_t biBitCount;
	uint32_t biCompression;
	uint32_t biSizeImage;
	uint32_t biXPelsPerMeter;
	uint32_t biYPelsPerMeter;
	uint32_t biClrUsed;
	uint32_t biClrImportant;

} avilib_BITMAPINFO;


typedef struct {

	uint16_t wFormatTag;
	uint16_t nChannels;
	uint32_t nSamplesPerSec;
	uint32_t nAvgBytesPerSec;
	uint16_t nBlockAlign;
	uint16_t wBitsPerSample;
	
} avilib_WAVEFORMATEX;

typedef enum { avilib_UnknownStreamtype, avilib_Video, avilib_Audio } avilib_streamtype_t;


AVI_DLL_DECL void *avireader_create();
AVI_DLL_DECL void avireader_destroy( void *p_context );
AVI_DLL_DECL int32_t avireader_open( void *p_context, const char *filename );
AVI_DLL_DECL int32_t avireader_close( void *p_context );
AVI_DLL_DECL int32_t avireader_read_frame( void *p_context, uint32_t idx, uint32_t stream, void *data );
AVI_DLL_DECL int32_t avireader_get_size( void *p_context, int32_t *width, int32_t *height );
AVI_DLL_DECL int32_t avireader_get_vrate( void *p_context, double *rateHz );
AVI_DLL_DECL int32_t avireader_get_frame_count( void *p_context, uint32_t stream, uint32_t *count );
AVI_DLL_DECL int32_t avireader_get_codec( void *p_context, uint32_t stream, uint32_t *codec );
AVI_DLL_DECL int32_t avireader_get_alloc_size( void *p_context, uint32_t stream, uint32_t *size );
AVI_DLL_DECL int32_t avireader_get_stream_count( void *p_context, uint32_t *count );
AVI_DLL_DECL int32_t avireader_get_vformat( void *p_context, uint32_t stream, avilib_BITMAPINFO *bm_info );
AVI_DLL_DECL int32_t avireader_get_aformat( void *p_context, uint32_t stream, avilib_WAVEFORMATEX *a_info );
AVI_DLL_DECL int32_t avireader_get_stream_type( void *p_context, uint32_t stream, avilib_streamtype_t *type );


AVI_DLL_DECL void *aviwriter_create();
AVI_DLL_DECL void aviwriter_destroy( void *p_context );
AVI_DLL_DECL int32_t aviwriter_open( void *p_context, const char *filename );
AVI_DLL_DECL int32_t aviwriter_close( void *p_context );
AVI_DLL_DECL int32_t aviwriter_write_frame( void *p_context, uint8_t stream_idx, void *data );
AVI_DLL_DECL int32_t aviwriter_set_vprops( void *p_context, uint8_t stream_idx, int32_t width, int32_t height, uint32_t codec, uint32_t framesize, double rateHz );
AVI_DLL_DECL int32_t aviwriter_set_frame_count( void *p_context, uint8_t stream_idx, uint32_t count );



