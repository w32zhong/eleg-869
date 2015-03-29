#pragma once
#include "PForDeltaDecompressor.hpp"
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <stdio.h>

using namespace std;

class PostingReader
{
public:
	unsigned docCount;
	unsigned totalTF;

	PostingReader(unsigned *buffer,unsigned offset,unsigned bufLength);
	bool nextRecord();
	inline unsigned getTF();
	inline unsigned getDocID();
	inline bool nextBlock();
	inline void jumpToBlock(unsigned b,unsigned docID);
	unsigned getBlock(unsigned theDocIDs[],unsigned theTFs[]);
	inline unsigned getBlockNum() {return blockNum;}
	~PostingReader();
private:
	unsigned *record,*buffer;
	unsigned *AsDocID,*AsTF;
	unsigned *BsDocID,*BsTF;
	unsigned *lengthDocID,*lengthTF;
	unsigned curPos,preDocID;
	unsigned curBlockLength;
	unsigned curBlock;
	unsigned blockNum;
	unsigned docIDs[PostingPerSet*2];
	unsigned TFs[PostingPerSet*2];
	unsigned bufferLength;

	PForDeltaDecompressor * decompressor;
};

PostingReader::PostingReader(unsigned *buf,unsigned offset,unsigned bl)
{
	buffer = buf;
	record = buf + offset;
	bufferLength = bl;
	docCount = *record++;
	totalTF = *record++;
	blockNum = (docCount+PostingPerSet-1)/PostingPerSet;
	//blockNum = bn;
	//cout<<"docCount = "<<docCount<<endl;
	//cout<<"totalTF = "<<totalTF<<endl;

	AsDocID = new unsigned[blockNum];
	AsTF = new unsigned[blockNum];
	BsDocID = new unsigned[blockNum];
	BsTF = new unsigned[blockNum];
	lengthDocID = new unsigned[blockNum];
	lengthTF = new unsigned[blockNum];

	unsigned i;
	for(i=0;i<blockNum;i++)
	{
		unsigned v = *record++;
		AsDocID[i] = (v>>13) & 7;
		BsDocID[i] = (v>>8) & 31;
		lengthDocID[i] = v & 255;
		AsTF[i] = (v>>29) & 7;
		BsTF[i] = (v>>24) & 31;
		lengthTF[i] = (v>>16) & 255;
	}
	curBlock = 0;
	preDocID = 0;
	curPos = PostingPerSet;
	curBlockLength = PostingPerSet;
	decompressor = new PForDeltaDecompressor();
}

bool PostingReader::nextBlock()
{
	//if(curBlock == 350) cout<<"a="<<AsDocID[curBlock]<<"b="<<BsDocID[curBlock]<<" length="<<lengthDocID[curBlock]<<endl;
	if(curBlock>=blockNum)
	{
		curPos=0;
		docIDs[0]=0xFFFFFFFF;
		return false;
	}
	curBlockLength = PostingPerSet;
	if(curBlock == blockNum-1)
	{
		curBlockLength = docCount % PostingPerSet;
		if(curBlockLength == 0) curBlockLength = PostingPerSet;
	}
	decompressor->decompressor(AsDocID[curBlock],BsDocID[curBlock],curBlockLength,lengthDocID[curBlock],preDocID,record,docIDs);
	record += lengthDocID[curBlock];
	preDocID = docIDs[PostingPerSet-1];
	decompressor->decompressor(AsTF[curBlock],BsTF[curBlock],curBlockLength,lengthTF[curBlock],~0,record,TFs);
	record += lengthTF[curBlock];
	curBlock++;
	curPos=0;
	return true;
}

inline bool PostingReader::nextRecord()
{
	if(curPos<curBlockLength-1) {curPos++;return true;}
	else return nextBlock();
}

inline unsigned PostingReader::getDocID() {return docIDs[curPos];}
inline unsigned PostingReader::getTF() {return TFs[curPos];}

inline void PostingReader::jumpToBlock(unsigned b,unsigned docID) 
{
	unsigned i;
	if(b==curBlock-1) return;
	if(b>=curBlock)
	{
		for(;curBlock<b;curBlock++) record+=lengthDocID[curBlock]+lengthTF[curBlock];
	}
	else
	{
		for(;curBlock>b;curBlock--) record-=lengthDocID[curBlock]+lengthTF[curBlock];
	}
	//curPos = curBlockLength;
	preDocID = docID;
	nextBlock();
}

unsigned PostingReader::getBlock(unsigned theDocIDs[],unsigned theTFs[])
{
	unsigned i;
	for(i=0;i<curBlockLength;i++) {theDocIDs[i] = docIDs[i];theTFs[i] = TFs[i];}
	return curBlockLength;
}

PostingReader::~PostingReader()
{
	delete decompressor;
	delete[] AsDocID;
	delete[] AsTF;
	delete[] BsDocID;
	delete[] BsTF;
	delete[] lengthDocID;
	delete[] lengthTF;
	if(munmap((char*)buffer,bufferLength*4) == -1) {cerr<<"unmap error at PostingReader destroying"<<endl;}
}

///////////////////////////////////////////////////////////////////////////////

class PostingListReader
{
public:
	PostingListReader(char* path);
	PostingReader *getPosting(unsigned termID);
	~PostingListReader();
private:
	unsigned termN;
	unsigned pageSize;
	char thePath[256];
	int fileHandleSummary;
	int fileHandle[256];
	unsigned fileSize[256];

	void openNewFile(unsigned fileNum);
};

PostingListReader::PostingListReader(char* path)
{
	strcpy(thePath,path);
	pageSize = sysconf(_SC_PAGESIZE)/4;
	//open the summary file
	char filename[512];
	sprintf(filename,"%s-summary",path);
	fileHandleSummary = open(filename,O_RDONLY);
	if(fileHandleSummary==-1) { cerr<<"Could not read the file "<<filename<<endl;return;}
	
	//initial other things
	unsigned i;
	for(i=0;i<256;i++)
	{
		fileHandle[i]=-1;
	}
}

void PostingListReader::openNewFile(unsigned fileNum)
{
	char filename[512];
	sprintf(filename,"%s-tables/%d",thePath,fileNum);
	fileHandle[fileNum] = open(filename,O_RDONLY);
	if(fileHandle[fileNum] == -1) {cerr<<"Could not read the file "<<filename<<endl;return;}
	struct stat s;
	fstat(fileHandle[fileNum],&s);
	fileSize[fileNum] = s.st_size;
}

PostingReader* PostingListReader::getPosting(unsigned termID)
{
	unsigned* buffer;
	unsigned bufferStart = (termID*2)/pageSize*pageSize;
	unsigned recordPos = termID*2-bufferStart;
	unsigned bufferSize = recordPos+4;
	buffer = (unsigned*)mmap(0,bufferSize*4,PROT_READ,MAP_SHARED,fileHandleSummary,bufferStart*4);
	unsigned pos = buffer[recordPos];
	unsigned fileNum = buffer[recordPos+1] & 255;
	unsigned length = buffer[recordPos+2] - pos;
	//unsigned blockNum = buffer[recordPos+1] >> 8;
	unsigned fileNum2 = buffer[recordPos+3] & 255;
	munmap((char*)buffer,bufferSize*4);
	// open the posting list tables
	if(fileHandle[fileNum] == -1) openNewFile(fileNum);
	bufferStart = pos/pageSize*pageSize;
	bufferSize = pos - bufferStart + length;
	if(fileNum != fileNum2) {bufferSize = fileSize[fileNum]/4 - bufferStart;}
	//cout<<"pos="<<pos<<endl;
	buffer = (unsigned*)mmap(0,bufferSize*4,PROT_READ,MAP_SHARED,fileHandle[fileNum],bufferStart*4);
	PostingReader *PR = new PostingReader(buffer,pos-bufferStart,bufferSize);
	return PR;
}

PostingListReader::~PostingListReader()
{
	unsigned i;
	for(i=0;i<256;i++) if(fileHandle[i]!=-1) close(fileHandle[i]);
	close(fileHandleSummary);
}	
