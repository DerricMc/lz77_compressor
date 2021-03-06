#include "lookupTable.h"
#include "lz77Type10.h"



lz77Type10::lz77Type10(int32_t MinimumOffset, int32_t SlidingWindow, int32_t MinimumMatch, int32_t BlockSize)
	:lzBase(MinimumOffset,SlidingWindow,MinimumMatch,BlockSize)
	{
			//ReadAheadBuffer is normalize between (minumum match) and(minimum match + 15) so that matches fit within
			//4-bits.
			m_iReadAheadBuffer = m_iMIN_MATCH + 0xF;
	}

/*
Paramaters are
infile - Name of the input file
outfile - name of the output file
offset - Offset to start compressing data
length - length of bytes from offset to attempt to compress data

Return SUCCESS on success
*/
enumCompressionResult lz77Type10::Compress(const wxString& inStr,const wxString& outStr,uint64_t offset,uint64_t length)
{
	wxFFile infile,outfile;
	infile.Open(inStr,wxT("rb"));
	infile.Seek(offset);
	uint8_t* filedata=new uint8_t[length];
	size_t bytesRead;
	//Read file from the offset into filedata
	bytesRead=infile.Read(filedata,length);
	infile.Close();
	if(bytesRead!=length){
		delete []filedata;
		filedata=NULL;
		return enumCompressionResult::COULD_NOT_READ_LENGTH_BYTES;
	}


	outfile.Open(outStr, wxT("wb"));
	if(outfile.IsOpened()==false)	{
		delete []filedata;
		filedata=NULL;
		return enumCompressionResult::FILE_NOT_OPENED;
	}
	
	uint32_t encodeSize=(length<<8)|(0x10);
	encodeSize = wxUINT32_SWAP_ON_BE(encodeSize); //File size needs to be written as little endian always
	outfile.Write(&encodeSize,4);
	
	uint8_t *ptrStart=filedata;
	uint8_t *ptrEnd=(uint8_t*)(filedata+length);
	
	//At most their will be two bytes written if the bytes can be compressed. So if all bytes in the block can be compressed it would take blockSize*2 bytes
	uint8_t *compressedBytes=new uint8_t[m_iBlockSize *2];//Holds the compressed bytes yet to be written
	while( ptrStart< ptrEnd )
	{
		uint8_t num_of_compressed_bytes=0;
		//In Binary represents 1 if byte is compressed or 0 if not compressed
		//For example 01001000 means that the second and fifth byte in the blockSize from the left is compressed
		uint8_t *ptrBytes=compressedBytes;
		for(int32_t i=0;i < m_iBlockSize;i++)
		{
			//length_offset searchResult=Search(ptrStart, filedata, ptrEnd);
			length_offset searchResult=lz77Table->search(ptrStart, filedata, ptrEnd);

			//If the number of bytes to be compressed is at least the size of the Minimum match 
			if(searchResult.length >= m_iMIN_MATCH)
			{	//Gotta swap the bytes since system is wii is big endian and most computers are little endian
				uint16_t len_off=wxINT16_SWAP_ON_LE( (((searchResult.length - m_iMIN_MATCH) & 0xF) << 12) | ((searchResult.offset - 1) & 0xFFF) );
				memcpy(ptrBytes,&len_off,sizeof(short));
				ptrBytes+=sizeof(short);
								
				ptrStart+=searchResult.length;
				
				num_of_compressed_bytes |=(1 << (7-i));
				//Stores which of the next 8 bytes is compressed
				//bit 1 for compress and bit 0 for not compressed
			}
			else if(searchResult.length >= 0)
			{
				*ptrBytes++=*ptrStart++;
				
			
			}
			else
				break;
		}
		outfile.Write(&num_of_compressed_bytes,1);
		outfile.Write(compressedBytes,(size_t)(ptrBytes-compressedBytes));

	}
	delete []compressedBytes;
	delete []filedata;
	compressedBytes=NULL;
	filedata=NULL;
	int32_t div4;
	//Add zeros until the file is a multiple of 4
	if((div4=outfile.Tell()%4) !=0 )
		outfile.Write("\0",4-div4);
	outfile.Close();
	return enumCompressionResult::SUCCESS;

}

/*
	
	inStr- Input file to decompress
	outStr-Output file to decompress
	offset-position in infile to start de-compression
*/
enumCompressionResult lz77Type10::Decompress(const wxString& inStr,const wxString& outStr,uint64_t offset)
{
	wxFFile infile,outfile;
	
	if(!FileIsCompressed(inStr, 0x10,offset))
	{
		return enumCompressionResult::FILE_NOT_COMPRESSED;//Not compressible
	}
	
	infile.Open(inStr,wxT("rb"));
	infile.Seek(offset);
	uint32_t filesize=0;
	infile.Read(&filesize,4); //Size of data when it is uncompressed
	filesize = wxUINT32_SWAP_ON_BE(filesize); //The compressed file has the filesize encoded in little endian
	filesize = filesize >> 8;//first byte is the encode flag

	int64_t inputsize=infile.Length()-offset-4;
	uint8_t* filedata=new uint8_t[inputsize];
	uint8_t* buffer=filedata;
	size_t bytesRead;
	//Read file from the offset into filedata
	while((bytesRead=infile.Read(buffer,4096))>0){
		buffer+=bytesRead;
	}
	infile.Close();
		
	uint8_t *uncompressedData=new uint8_t[filesize];
	uint8_t *outputPtr=uncompressedData;
	uint8_t *outputEndPtr=uncompressedData+filesize;
	uint8_t *inputPtr=filedata;
	uint8_t *inputEndPtr=filedata+inputsize;
	while(inputPtr<inputEndPtr && outputPtr<outputEndPtr)
	{
	
		uint8_t isCompressed=*inputPtr++;

		for(int32_t i=0;i < m_iBlockSize; i++)
		{
			//Checks to see if the next byte is compressed by looking 
			//at its binary representation - E.g 10010000
			//This says that the first extracted byte and the four extracted byte is compressed
			 if ((isCompressed>>(7-i)) & 0x1) 					
			 {
				 uint16_t len_off;
				 memcpy(&len_off,inputPtr,sizeof(uint16_t));
				 inputPtr+=sizeof(uint16_t);//Move forward two bytes
				 
				 len_off=wxINT16_SWAP_ON_LE(len_off);
								 
				 //length offset pair has been decoded.
				 length_offset decoding={(len_off>>12)+m_iMIN_MATCH, static_cast<uint16_t>((len_off & 0xFFF) + 1)};
				 

				if((outputPtr - decoding.offset) < uncompressedData){//If the offset to look for uncompressed is passed the current uncompresed data then the data is not compressed
				 	delete []uncompressedData;
				 	delete []filedata;
					uncompressedData=NULL;
				 	filedata=NULL;
				 	return enumCompressionResult::INVALID_COMPRESSED_DATA;
				 }
				for(int32_t j=0;j<decoding.length;++j)
					outputPtr[j]=(outputPtr-decoding.offset)[j];
				outputPtr+=decoding.length;
			 }
			 else
				*outputPtr++=*inputPtr++;
			 		 
			 if(!(inputPtr<inputEndPtr && outputPtr<outputEndPtr))
			 	break;
			 	
			 
		}
	}
	
	outfile.Open(outStr, wxT("wb"));
	if(outfile.IsOpened()==false){
	
		delete []uncompressedData;
		delete []filedata;
		uncompressedData=NULL;
		filedata=NULL;
		return enumCompressionResult::FILE_NOT_OPENED;
	}
	outfile.Write(uncompressedData,filesize);
	outfile.Close();
	delete []uncompressedData;
	delete []filedata;
	uncompressedData=NULL;
	filedata=NULL;
	return enumCompressionResult::SUCCESS;
	
	
}

