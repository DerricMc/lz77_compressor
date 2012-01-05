#ifndef __STRUCT_H__
#define __STRUCT_H__
struct length_offset
{
	int32_t length;//The number of bytes compressed
	uint16_t offset;//How far back in sliding window where bytes that match the lookAheadBuffer is located
	bool compare_equal(const length_offset& lo_pair)
{
	return length == lo_pair.length && offset == lo_pair.offset;
}
};

#endif // __STRUCT_H__
