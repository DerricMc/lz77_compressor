#include "lz77Type10.h"


/*
Paramaters are
infile - Name of the input file
outfile - name of the output file
offset - Offset to start compressing data
length - length of bytes from offset to attempt to compress data

Return 0 on success
*/

lz77Type10::lz77Type10(int ReadAheadBuffer,int MinimumOffset,int SlidingWindow,int MinimumMatch,int BlockSize)
	:lzBase(ReadAheadBuffer,MinimumOffset,SlidingWindow,MinimumMatch,BlockSize)
	{}

enumCompressionResult lz77Type10::Compress(const wxString& inStr,const wxString& outStr,unsigned long offset,unsigned long length)
{
	wxFFile infile,outfile;
	infile.Open(inStr,wxT("rb"));
	infile.Seek(offset);
	byte* filedata=new byte[length];
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
	
	int encodeSize=(length<<8)|(0x10);
	
	outfile.Write(&encodeSize,4);
	
	byte *ptrStart=filedata;
	byte *ptrEnd=(byte*)(filedata+length);
	
	//At most their will be two bytes written if the bytes can be compressed. So if all bytes in the block can be compressed it would take blockSize*2 bytes
	byte *compressedBytes=new byte[m_iBlockSize *2];//Holds the compressed bytes yet to be written
	while( ptrStart< ptrEnd )
	{
		byte num_of_compressed_bytes=0;
		//In Binary represents 1 if byte is compressed or 0 if not compressed
		//For example 01001000 means that the second and fifth byte in the blockSize from the left is compressed
		byte *ptrBytes=compressedBytes;
		for(int i=0;i < m_iBlockSize;i++)
		{
			length_offset searchResult=Search(filedata,ptrStart,ptrEnd);
			//If the number of bytes to be compressed is at least the size of the Minimum match 
			if(searchResult.length >= m_iMIN_MATCH)
			{	//Gotta swap the bytes since system is wii is big endian and most computers are little endian
				unsigned short len_off=wxINT16_SWAP_ON_LE( (((searchResult.length - m_iMIN_MATCH) & 0xF) << 12) | ((searchResult.offset - 1) & 0xFFF) );
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
	int div4;
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
enumCompressionResult lz77Type10::Decompress(const wxString& inStr,const wxString& outStr,unsigned long offset)
{
	wxFFile infile,outfile;
	infile.Open(inStr,wxT("rb"));
	
	infile.Seek(offset);
	
	if(!FileIsCompressed(inStr, 0x10,offset))
	{
		infile.Close();
		return enumCompressionResult::FILE_NOT_COMPRESSED;//Not compressible
	}
	unsigned int filesize=0;
	infile.Read(&filesize,3); //Size of data when it is uncompressed

	long inputsize=infile.Length()-offset-4;
	byte* filedata=new byte[inputsize];
	byte* buffer=filedata;
	size_t bytesRead;
	//Read file from the offset into filedata
	while((bytesRead=infile.Read(buffer,4096))>0){
		buffer+=bytesRead;
	}
	infile.Close();
		
	byte *uncompressedData=new byte[filesize];
	byte *outputPtr=uncompressedData;
	byte *outputEndPtr=uncompressedData+filesize;
	byte *inputPtr=filedata;
	byte *inputEndPtr=filedata+inputsize;
	while(inputPtr<inputEndPtr && outputPtr<outputEndPtr)
	{
	
		byte isCompressed=*inputPtr++;

		for(int i=0;i < m_iBlockSize; i++)
		{
			//Checks to see if the next byte is compressed by looking 
			//at its binary representation - E.g 10010000
			//This says that the first extracted byte and the four extracted byte is compressed
			 if ((isCompressed>>(7-i)) & 0x1) 					
			 {
				 unsigned short len_off;
				 memcpy(&len_off,inputPtr,sizeof(short));
				 inputPtr+=sizeof(short);//Move forward two bytes
				 
				 len_off=wxINT16_SWAP_ON_LE(len_off);
								 
				 //length offset pair has been decoded.
				 length_offset decoding={(len_off>>12)+m_iMIN_MATCH, static_cast<unsigned short>((len_off & 0xFFF) + 1)};
				 

				if((outputPtr - decoding.offset) < uncompressedData){//If the offset to look for uncompressed is passed the current uncompresed data then the data is not compressed
				 	delete []uncompressedData;
				 	delete []filedata;
					uncompressedData=NULL;
				 	filedata=NULL;
				 	return enumCompressionResult::INVALID_COMPRESSED_DATA;
				 }
				for(int j=0;j<decoding.length;++j)
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

