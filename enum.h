#ifndef __ENUM_H__
#define __ENUM_H__
enum class enumCompressionResult:int
{
			COMPRESSED_FORMAT_NOT_SUPPORTED=-10,
			COULD_NOT_READ_LENGTH_BYTES=-2,
			FILE_NOT_OPENED=-1,
			SUCCESS=0,
			FILE_NOT_COMPRESSED,
			INVALID_COMPRESSED_DATA,
			UNCOMPRESSED_SIZE_INVALID
};
enum class enumCompressionMode:int
{
			compressSelect=0,
			decompressSelect=1
};
enum enumCompressionType:int
{
			BEST_LZ77_TYPE_10=0,
			EXACT_WII_LZ77_TYPE_10,
			WII_LZ77_TYPE_11
};
#endif //__ENUM_H__
