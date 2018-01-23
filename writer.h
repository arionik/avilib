
namespace avilib
{
	class AviWriter : protected LoggingObject
	{
		std::ofstream _f;
		uint64_t pos_RIFFSize;
		uint64_t pos_moviListSize;
		std::ofstream::streamoff pos_1stMoviStart;
		std::ofstream::streamoff pos_odmlExt;
		std::ofstream::streamoff pos_odmlSuperIdx;
		std::ofstream::streamoff pos_aviMainHeader;
		std::map<int32_t, std::ofstream::streamoff> pos_streamHeader;

		avilib::AVISUPERINDEX m_superIdx;
		std::vector<avilib::AVISTDINDEX> m_stdIndexes;
		std::map<int32_t, std::deque<avilib::AVISTDINDEX_ENTRY>> m_stdIndexEntries;

		std::deque<avilib::AVIOLDINDEX> m_oldIndexEntries;

		// AVI header stuff
		avilib::AVIMAINHEADER m_avimHeader;
		avilib::AVISTREAMHEADER m_avisHeader;
		avilib::BITMAPINFO m_bitmapInfo;
		avilib::WAVEFORMATEX m_waveFormat;


		bool m_openDML = true;
		bool m_propsSet = false;
		bool m_opened = false;
		uint32_t m_reservedFrames = 0;

		uint32_t m_RIFF_idx = 0;  // nth RIFF section (OpenDML)
		uint32_t m_RIFF_size = 0; // current RIFF size (OpenDML)
		uint32_t m_totalFrames = 0;
		uint64_t m_currBaseOff = 0;
		uint64_t m_currMaxRIFFSize = 1000*1000*1000;

		void finish_old();
		void finish_avix();
		void reset();

	public:
		AviWriter();
		virtual ~AviWriter();

		bool setVideoProperties( uint8_t stream_idx, int32_t width, int32_t height, uint32_t codec, uint32_t framesize, double rateHz );
		bool setAudioProperties( uint8_t stream_idx, int16_t format, int8_t channels, uint32_t samplesPerSecond, uint32_t avgBytesPerSecond, uint16_t bitsPerSample, uint16_t blockAlign );
		bool setFrameCount( uint8_t stream_idx, uint32_t count );

		bool open( const char *filename );
		bool write_frame( uint8_t stream_idx, void *data );
		bool close();
	};

}


