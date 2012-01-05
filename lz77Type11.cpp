#include "lz77Type11.h"


/*
Paramaters are
infile - Name of the input file
outfile - name of the output file
offset - Offset to start compressing data
length - length of bytes from offset to attempt to compress data

Return 0 on success
*/

lz77Type11::lz77Type11(int MinimumOffset, int SlidingWindow, int MinimumMatch, int BlockSize)
	:lzBase(MinimumOffset,SlidingWindow,MinimumMatch,BlockSize)
	{
		 m_iReadAheadBuffer=(0xF + 0xFF + 0xFFFF + m_iMIN_MATCH);
	}


enumCompressionResult lz77Type11::Compress(const wxString& inStr,const wxString& outStr,unsigned long offset,unsigned long length)
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
	if (length>0xFFFFFF){// If length is greater than 24 bits or 16 Megs
		uint32_t encodeFlag=0x11;
		encodeFlag = wxUINT32_SWAP_ON_BE(encodeFlag);
		length = wxUINT32_SWAP_ON_BE(length);//Filesize data is little endian
		outfile.Write(&encodeFlag,4);
		outfile.Write(&length,4);
		
	}
	else{
		uint32_t encodeSize=(length<<8)|(0x11);
		encodeSize = wxUINT32_SWAP_ON_BE(encodeSize);
		outfile.Write(&encodeSize,4);
	}

	uint8_t *ptrStart=filedata;
	uint8_t *ptrEnd=(uint8_t*)(filedata+length);
	
	//At most their will be two bytes written if the bytes can be compressed. So if all bytes in the block can be compressed it would take blockSize*2 bytes
	uint8_t *compressedBytes=new uint8_t[m_iBlockSize *2];//Holds the compressed bytes yet to be written
	
	uint8_t maxTwoByteMatch= 0xF+1; 
	uint8_t minThreeByteMatch=maxTwoByteMatch+1;//Minimum Three byte match is maximum TwoByte match + 1
	unsigned short maxThreeByteMatch= 0xFF+minThreeByteMatch;
	unsigned short minFourByteMatch=maxThreeByteMatch+1;//Minimum Four byte match is maximum Three Byte match + 1
	int maxFourByteMatch=0xFFFF+minFourByteMatch;
	/*
	Normaliazation Example: If MIN_MATCH is 3 then 3 gets mapped to 2 and 16 gets mapped to 15. 
	17 gets mapped to 1 and 272 gets mapped to 255
	273 gets mapped to 0 and 65808 gets mapped to 65535
				     
				     
	A two byte match uses 4 bits
	A three byte match uses 8 bits
	A four byte match uses 16 bits
	In each case the offset uses 12 bits 
	In the two byte case the length is normalized so that the first 4 bits are numbers between between 2 and 15
	In the three byte case the first 4 bits are 0000
	In the four byte case the first 4 bits a 0001
	*/		
	while( ptrStart< ptrEnd )
	{
		uint8_t num_of_compressed_bytes=0;
		//In Binary represents 1 if byte is compressed or 0 if not compressed
		//For example 01001000 means that the second and fifth byte in the blockSize from the left is compressed
		uint8_t *ptrBytes=compressedBytes;
		for(int i=0;i < m_iBlockSize;i++)
		{
			length_offset searchResult=Search(filedata,ptrStart,ptrEnd);
			//If the number of bytes to be compressed is at least the size of the Minimum match 
			if(searchResult.length >= m_iMIN_MATCH)
			{	//Gotta swap the bytes since system is wii is big endian and most computers are little endian
				
				if(searchResult.length <= maxTwoByteMatch){
					unsigned short len_off=wxINT16_SWAP_ON_LE
					( 
						(((searchResult.length - 1) & 0xF) << 12) | //Bits 15-12
						((searchResult.offset - 1) & 0xFFF) 		       //Bits 11-0
					);	
					memcpy(ptrBytes,&len_off,2);
					ptrBytes+=2;
				}
				else if(searchResult.length <= maxThreeByteMatch){ 
					unsigned int len_off=wxINT32_SWAP_ON_LE
					(  
						
						(((searchResult.length - minThreeByteMatch) & 0xFF)<< 12) | //Bits 20-12
						((searchResult.offset - 1) & 0xFFF)               	    //Bits 11-0
					);
					memcpy(ptrBytes,(uint8_t*)&len_off+1,3); //Make sure to copy the lower 24 bits. 0x12345678- This statement copies 0x123456
					ptrBytes+=3;
				}
				else if(searchResult.length <= maxFourByteMatch){ 
					unsigned int len_off=wxINT32_SWAP_ON_LE
					(  
						(1<<28) | 						     //Bits 31-28 Flag to say that this is four bytes
						(((searchResult.length - minFourByteMatch) & 0xFFFF)<< 12) | //Bits 28-12
						((searchResult.offset - 1) & 0xFFF)               	     //Bits 11-0
					);
					memcpy(ptrBytes,&len_off,4);
					ptrBytes+=4;
				}
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
enumCompressionResult lz77Type11::Decompress(const wxString& inStr,const wxString& outStr,unsigned long offset)
{
	wxFFile infile,outfile;

	if(!FileIsCompressed(inStr, 0x11,offset))
	{
		return enumCompressionResult::FILE_NOT_COMPRESSED;//Not compressible
	}
	
	infile.Open(inStr,wxT("rb"));
	infile.Seek(offset);
	unsigned int filesize=0;
	infile.Read(&filesize,4);
	filesize = wxUINT32_SWAP_ON_BE(filesize);//The compressed file has the filesize encoded in little endian
	filesize = filesize >> 8;//First byte is the encode flag
	if(filesize==0)//If the filesize var is zero then the true filesize is over 14MB and must be read in from the next 4 bytes
	{
		infile.Read(&filesize,4);
		filesize = wxUINT32_SWAP_ON_BE(filesize);
	}
	
	long inputsize=infile.Length()-offset-4;
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
	
	length_offset decoding;
	uint8_t maxTwoByteMatch= 0xF+1; 
	uint8_t threeByteDenorm=maxTwoByteMatch+1;//Amount to add to length when compression is 3 bytes
	unsigned short maxThreeByteMatch=0xFF+threeByteDenorm;
	unsigned short fourByteDenorm=maxThreeByteMatch+1;
	
	while(inputPtr<inputEndPtr && outputPtr<outputEndPtr)
	{
	
		uint8_t isCompressed=*inputPtr++;

		for(int i=0;i < m_iBlockSize; i++)
		{
			//Checks to see if the next byte is compressed by looking 
			//at its binary representation - E.g 10010000
			//This says that the first extracted byte and the four extracted byte is compressed
			 if ((isCompressed>>(7-i)) & 0x1) 					
			 {
				 uint8_t metaDataSize=*inputPtr >> 4;//Look at the top 4 bits
				 if(metaDataSize >= 2){ //Two Bytes of Length/Offset MetaData
				 	unsigned short len_off=0;
				 	memcpy(&len_off,inputPtr,2);
				 	inputPtr+=2;
				  	len_off=wxINT16_SWAP_ON_LE(len_off);
					decoding.length=(len_off>>12)+1;
					decoding.offset=(len_off & 0xFFF) + 1;
				 }
				 else if (metaDataSize==0){ //Three Bytes of Length/Offset MetaData
				 	unsigned int len_off=0;
				 	memcpy((uint8_t*)&len_off+1,inputPtr,3);
				 	inputPtr+=3;
				  	len_off=wxINT32_SWAP_ON_LE(len_off);
					decoding.length=(len_off>>12)+threeByteDenorm;
					decoding.offset=(len_off & 0xFFF) + 1;
				 }
				 else if(metaDataSize==1){ //Four Bytes of Length/Offset MetaData
				 	unsigned int len_off=0;
				 	memcpy(&len_off,inputPtr,4);
				 	inputPtr+=4;
				  	len_off=wxINT32_SWAP_ON_LE(len_off);
				  	 
					decoding.length=((len_off>>12) & 0xFFFF)+fourByteDenorm; //Gets rid of the Four byte flag
					decoding.offset=(len_off & 0xFFF) + 1;
				 }
				 else{
				 	delete []uncompressedData;
				 	delete []filedata;
					uncompressedData=NULL;
				 	filedata=NULL;
				 	return enumCompressionResult::INVALID_COMPRESSED_DATA;
				 }

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

