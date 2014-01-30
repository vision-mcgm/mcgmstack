//#ifdef WIN32
	#include <windows.h>
	#include <wingdi.h>
//#endif /* WIN32 */

// Open GL libraries currently used in function _Img::ReadBMP()
// This code should ideally be rewritten
#include <gl\gl.h>			
#include <gl\glu.h>		
#include <glaux.h>
//#include "C:\glut\glut.h"

#pragma comment(lib, "glut32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "glaux.lib")

#include <math.h>
#include <stdio.h>
#include <new>
#include <string>
#include <iostream>

#include <typeinfo>

//#include <vector>
//#include <list>

#include "UCLVisionResearchLabLib.h"

static FILE *pErrorFile=stderr;

void SetUCLVRLErrorFile(FILE *_f)
{
	pErrorFile=_f;
}

FILE *GetUCLVRLErrorFile()
{
	return pErrorFile;
}

////////////////////////////////////////////////////////////////////////////
// VISION LIBRARY IMAGE IMPLEMENTS ABSTRACT IMAGE CLASS_____________________
////////////////////////////////////////////////////////////////////////////
	// Independent image class (i.e. not dependent on 3rd party software libraries)
	// Code specifies I/O, creation, destruction, copying and format conversion of images.
	// Code implements convolution using filters from the AbstractFilter class

_Img::_Img()
{
	m_pData=NULL;
	memset(&m_Hdr,0,sizeof(_ImgHdr));
}

_Img::_Img(	unsigned int w,unsigned int h, 
			unsigned int channels,short int dataType)
{
	m_pData=NULL;
	Create(w,h,channels,dataType);
}

inline unsigned int _Img::DataSz() const{
	return m_Hdr.w*m_Hdr.h*m_Hdr.channels*m_Hdr.depth;
}

bool _Img::Alloc()
{
	unsigned int datasz=DataSz();
	
	if(datasz>0)
	{
		try{
			m_pData=new char[datasz];
		}
		catch( bad_alloc xa )
		{
			fprintf(pErrorFile,"_Img::Alloc() Error allocating memory\n");
			Destroy();
			return false;
		}
	}
	return true;
}

bool _Img::AllocInit()
{
	unsigned int datasz=DataSz();
	
	if(datasz>0)
	{
		try{
			m_pData=new char[datasz];
		}
		catch( bad_alloc xa )
		{
			fprintf(pErrorFile,"_Img::Alloc() Error allocating memory\n");
			Destroy();
			return false;
		}
		memset(m_pData,0,datasz);
	}
	
	return true;
}

_Img::_Img(const _Img &img)
{	
	m_pData=NULL;
	memset(&m_Hdr,0,sizeof(_ImgHdr));
	Copy(img);
}


AbstractImage &_Img::operator=(const AbstractImage &img)
{
	if (this!=&img) 
	{
		Copy(img);
	}
	return *this;
}

_Img::~_Img()
{
	//fprintf(pErrorFile,"before deleting %p", this);

	Destroy();

	//fprintf(pErrorFile,"after deletion"); 
}

// 
// Create/destroy
//

bool _Img::CheckDataType(short int dataType) const
{
	if( dataType!=IMG_UCHAR
		&&
		dataType!=IMG_LONG
		&&
		dataType!=IMG_FLOAT
		)
		return false;

	return true;
}

bool _Img::CalcDepth(unsigned int &depth,short int dataType) const
{
	switch(dataType){
	
	case IMG_UCHAR: {
						depth=1;
					} break;
	case IMG_LONG:	{
						depth=sizeof(long);
					} break;
	case IMG_FLOAT:	{
						depth=sizeof(float);
					} break;
	default:		{
						return false;
					}
	}
	return true;
}

/*
bool _Img::Create(	unsigned int w,unsigned int h,
					unsigned int channels,short int dataType)
{
	if(!CheckDataType(dataType)){
		fprintf(pErrorFile,"_Img::Create(), Invalid data type (%d)\n",dataType);
		Destroy();
		return false;
	}

	unsigned int depth=0;

	if(!CalcDepth(depth,dataType)){
		fprintf(pErrorFile,"_Img::Create(), Error calculating depth\n");
		Destroy();
		return false;
	}
	
	if(IsValid()){
		if(DataSz()==w*h*channels*depth){
			m_Hdr.dataType=dataType;
			memset(m_pData,0,DataSz());
			return true;
		}
		else
			Destroy();
	}

	m_Hdr.w=w;
	m_Hdr.h=h;
	m_Hdr.channels=channels;
	m_Hdr.dataType=dataType;
	m_Hdr.depth=depth;

	if(!AllocInit()){
		fprintf(pErrorFile,"_Img::Create() Error allocating memory\n");
		return false;
	}
	return true;
}
*/
bool _Img::Create(	unsigned int w,unsigned int h,
					unsigned int channels,short int dataType,
					const void *data)
{
	if(!CheckDataType(dataType)){
		fprintf(pErrorFile,"_Img::Create(), Invalid data type %d\n",dataType);
		Destroy();
		return false;
	}

	unsigned int depth=0;

	if(!CalcDepth(depth,dataType)){
		fprintf(pErrorFile,"_Img::Create(), Error calculating depth\n");
		Destroy();
		return false;
	}

	if(IsValid()&&DataSz()!=w*h*channels*depth)
		Destroy();

	m_Hdr.w=w;
	m_Hdr.h=h;
	m_Hdr.channels=channels;
	m_Hdr.depth=depth;
	m_Hdr.dataType=dataType;

	if(!IsValid())
	{
		if(!Alloc()){
			fprintf(pErrorFile,"_Img::Create(), Error allocating memory\n");
			Destroy();
			return false;
		}
	}

	if(data){
		memcpy(m_pData,data,DataSz());
	}
	else{
		memset(m_pData,0,DataSz());
	}
	return true;
}


bool _Img::Copy(const AbstractImage &src)
{
	if (this==&src)
		return false;

	if(!src.IsValid())
	{
		//memcpy(&m_Hdr,&src.m_Hdr,sizeof(_srcHdr));
		Destroy();
		m_Hdr.w=src.Width();
		m_Hdr.h=src.Height();
		m_Hdr.channels=src.Channels();
		m_Hdr.depth=src.Depth();
		m_Hdr.dataType=src.DataType();
		
		m_pData=NULL;
		fprintf(pErrorFile,"_src::Copy() arg Data=NULL\n"); 
		
		return false;
	}

	return Create(src.Width(),src.Height(),src.Channels(),src.DataType(),src.Data());
}

bool _Img::Copy(	const AbstractImage &src,
					unsigned int x,unsigned int y,
					unsigned int xRange,unsigned int yRange)
{
	if(this==&src){
		fprintf(pErrorFile,"_Img::Copy(), this==src\n");
		return false;
	}

	if(!src.IsValid())
	{
		fprintf(pErrorFile,"_Img::Copy(), arg 'src' invalid\n");
		return false;
	}

	if(x>=src.Width()||y>=src.Height())
	{
		fprintf(pErrorFile,
"_Img::Copy(), Error, lower left of copy segment (%d,%d) outside image dimensions (%d*%d)\n",
		x,y,src.Width(),src.Height());
		return false;
	}

	Create(xRange,yRange,src.Channels(),src.DataType());

	unsigned int maxX=x+xRange;
	unsigned int maxY=y+yRange;
		//avoid copy boundary overstepping original image border
	if(maxX>src.Width())
		maxX=src.Width();

	if(maxY>src.Height())
		maxY=src.Height();

	unsigned int bytesPerPixel=Depth()*Channels();
	
	unsigned int srcWidthStep=src.WidthStepBytes();
	unsigned int dstWidthStep=WidthStepBytes();
	unsigned int copySz=(maxX-x)*bytesPerPixel;
	
	char *pSrcEnd=((char*)src.Data())+(maxY*src.WidthStepBytes());
	
	char *pSrcRow=((char*)src.Data())+(y*srcWidthStep)+(x*bytesPerPixel);
	char *pDstRow=(char*)Data();
	
	for(	;
		pSrcRow<pSrcEnd;
		pSrcRow+=srcWidthStep,
		pDstRow+=dstWidthStep)
	{
		memcpy(pDstRow,pSrcRow,copySz);
	}
	
	return true;
}


bool _Img::BlankCopy(const AbstractImage &src)
{
	if (this==&src)
		return false;

	if(!src.IsValid())
	{
		//memcpy(&m_Hdr,&src.m_Hdr,sizeof(_srcHdr));
		Destroy();
		m_Hdr.w=src.Width();
		m_Hdr.h=src.Height();
		m_Hdr.channels=src.Channels();
		m_Hdr.depth=src.Depth();
		m_Hdr.dataType=src.DataType();
		
		m_pData=NULL;
		fprintf(pErrorFile,"_src::BlankCopy() arg Data=NULL\n"); 
		
		return false;
	}

	return Create(src.Width(),src.Height(),src.Channels(),src.DataType());
}


bool _Img::Paste(	const AbstractImage &src,	
					unsigned int x,unsigned int y)
{
	if(!src.IsValid())
	{
		fprintf(pErrorFile,"_Img::Paste(), arg 'src' invalid\n");
		return false;
	}
	
	if(!IsValid()){
		return Copy(src);
	}

	if(!SameChannels(src)||!SameDataType(src))
	{
		fprintf(pErrorFile,"_Img::Paste(), this img and arg 'src' have different formats\n");
		fprintf(pErrorFile,"\t this img:%d channels, data-type label: %d, arg:%d channels, data-type label:%d\n", 
						Channels(),DataType(),src.Channels(),src.DataType());
		return false;
	}
	
	unsigned int copyWidth,copyHeight;
	
	if(x+src.Width()>Width())
	{
		copyWidth=Width()-x;
	}
	else
		copyWidth=src.Width();
		
		
	if(y+src.Height()>Height())
	{
		copyHeight=Height()-y;
	}
	else
		copyHeight=src.Height();

	if(x==0&&y==0&&Width()==src.Width()&&Height()==src.Height())
	{
		return Copy(src);
	}
	else
	{
		char *_i, *pDst;
		unsigned int bytesPerPixel=Channels()*Depth();
		unsigned int dstWidthStepBytes=WidthStepBytes();
		unsigned int srcWidthStep=src.WidthStepBytes();
		unsigned int copyWidthBytes=copyWidth*bytesPerPixel;
		unsigned int i;

		pDst=(char*)Data();

		for( _i=(char*)src.Data(),
			 i = 0;
			 i <copyHeight;
			 _i+=srcWidthStep,
			 ++i )	
		{
			memcpy(pDst+(((y+i)*dstWidthStepBytes)+(x*bytesPerPixel)),_i,copyWidthBytes);
		}
	}
	return true;
}

bool _Img::PasteTile(unsigned int rows,unsigned int cols,const AbstractImage **ppSrcs)
{
	unsigned int sz=rows*cols;
	
	if(sz==0){
		fprintf(stderr,"_Img::PasteTile, rows*cols=0\n");
		return false;
	}

	if(!IsValid())
	{
		fprintf(stderr,"_Img::PasteTile, this image invalid\n");
		return false;
	}

	if(!ppSrcs)
	{
		fprintf(stderr,"_Img::PasteTile, ppSrcs=NULL\n");
		return false;
	}

	if(!*ppSrcs)
	{
		fprintf(stderr,"_Img::PasteTile, *ppSrcs=NULL\n");
		return false;
	}

	unsigned int w=Width()/cols;
	unsigned int h=Height()/rows;
	unsigned int min_x=0, min_y=0;

	//int i=0;

	_Img unit;
	unit.Create(w,h,Channels(),DataType());

	for(int y=0;y<rows;++y)
	{
		for(int x=0;x<cols;++x)
		{
			if(!unit.Convert(ppSrcs[x][y])){
				fprintf(stderr,
			"_Img::GridLayout(), Error converting image to this format\n");
							return false;
			}
			min_x=x*w;
			min_y=Height()-((y+1)*h);
			if(!Paste(unit,min_x,min_y)){
				fprintf(stderr,
			"_Img::PasteTile(), Error pasting converted image into this image\n");
							return false;
			}
		}
	}
	return true;
}

bool _Img::Zero()
{
	if(!IsValid()){
		fprintf(pErrorFile,"_Img::Zero(), this image invalid\n");
		return false;
	}

	memset(m_pData,0,DataSz());
	return true;
}


AbstractImage* _Img::New() const
{
	_Img *pReturn;

	try{
		pReturn=new _Img;
	}
	catch(bad_alloc xa)
	{
		fprintf(pErrorFile,"_Img::New(), Error allocating memory\n");
		pReturn=NULL;
	}

	return pReturn;
}

AbstractImage* _Img::NewCopy() const
{
	_Img *pReturn;

	try{
		pReturn=new _Img(*this);
	}
	catch(bad_alloc xa)
	{
		fprintf(pErrorFile,"_Img::NewCopy(), Error allocating memory\n");
		pReturn=NULL;
	}

	return pReturn;
}

AbstractImage* _Img::NewHeader() const
{
	_Img *pReturn;

	try{
		pReturn=new _Img;
		memcpy(&(pReturn->m_Hdr),&m_Hdr,sizeof(_ImgHdr));
	}
	catch(bad_alloc xa)
	{
		fprintf(pErrorFile,"_Img::NewCopy(), Error allocating memory\n");
		pReturn=NULL;
	}
	return pReturn;
}

AbstractImage* _Img::NewBlankCopy() const
{
	_Img *pReturn;

	try{
		pReturn=new _Img;
		pReturn->Create(m_Hdr.w,m_Hdr.h,m_Hdr.channels,m_Hdr.dataType);
	}
	catch(bad_alloc xa)
	{
		fprintf(pErrorFile,"_Img::NewBlankCopy(), Error allocating memory\n");
		pReturn=NULL;
	}

	return pReturn;
}

AbstractImage* _Img::NewBlankCopy(unsigned int w,unsigned int h) const
{
	_Img *pReturn;

	try{
		pReturn=new _Img;
		pReturn->Create(w,h,m_Hdr.channels,m_Hdr.dataType);
	}
	catch(bad_alloc xa)
	{
		fprintf(pErrorFile,"_Img::NewBlankCopy(), Error allocating memory\n");
		pReturn=NULL;
	}

	return pReturn;
}


	//will resize image using bilinear interpolation, 
	//if shrinking image may be beneficial to blur beforehand
bool _Img::Resize(const AbstractImage &src)
{
	if(this==&src){
		fprintf(pErrorFile,"_Img::Resize(), this==src\n");
		return false;
	}

	if(!src.IsValid()){
		fprintf(pErrorFile,"_Img::Resize(), Arg 'src' invalid\n");
		return false;
	}

	if(!IsValid()){
		return Copy(src);
	}

	if(!SameChannels(src)||!SameDataType(src))
	{
		fprintf(pErrorFile,"_Img::Resize(), this img and arg 'src' have different formats\n");
		fprintf(pErrorFile,"\t this img:%d channels, data-type label: %d, arg:%d channels, data-type label:%d\n", 
						Channels(),DataType(),src.Channels(),src.DataType());
		return false;
	}

	int x,y;
	unsigned int dstW=Width();
	unsigned int dstH=Height();

	float scaleX,scaleY; 

	scaleX=((float)src.Width())/dstW;
	scaleY=((float)src.Height())/dstH;

	switch(DataType())
	{
	case IMG_UCHAR:	{
						unsigned char *pSrcData, *pDstPix;
						pSrcData=(unsigned char*)src.Data();
						pDstPix=(unsigned char*)Data();
						for (y=0;y<dstH;++y){
							for(x=0;x<dstW;++x){
							
								BilinearInterpolatePixel(	pDstPix,//(src.Channels()*((y*dstW)+x)),
															pSrcData,
															src.Channels(),src.Width(),
															src.Height(),
															x*scaleX,y*scaleY);
								pDstPix+=src.Channels();
															
							}
						}
					} break;
	case IMG_LONG:	{
						long *pSrcData, *pDstPix;
						pSrcData=(long*)src.Data();
						pDstPix=(long*)Data();

						for (y=0;y<dstH;++y){
							for(x=0;x<dstW;++x){
								BilinearInterpolatePixel(	pDstPix,//+(src.Channels()*((y*dstW)+x)),
															pSrcData,
															src.Channels(),src.Width(),
															src.Height(),
															x*scaleX,y*scaleY);
								pDstPix+=src.Channels();
							}
						}
					} break;
	case IMG_FLOAT:	{
						float *pSrcData, *pDstPix;
						pSrcData=(float*)src.Data();
						pDstPix=(float*)Data();

						for (y=0;y<dstH;++y){
							for(x=0;x<dstW;++x){	
								BilinearInterpolatePixel(	pDstPix,//+(src.Channels()*((y*dstW)+x)),
															pSrcData,
															src.Channels(),src.Width(),
															src.Height(),
															x*scaleX,y*scaleY);
								pDstPix+=src.Channels();
							}
						}
					} break;
	default:		{
						fprintf(pErrorFile,"_Img::Resize(), unrecognised data-type label (%d)\n",
							DataType());
						return false;
					};
	}

	return true;
}



bool _Img::Convert(const AbstractImage &src)
{
	if (this==&src){
		fprintf(pErrorFile,"_Img::Convert(), this==src\n");
		return false;
	}

	if(!src.IsValid())
	{
		fprintf(pErrorFile,"_Img::Convert(), Arg 'src' invalid\n");
		return false;
	}

	if(!IsValid()||SameType(src))
		return Copy(src);

	_Img temp,temp2;
	const AbstractImage *pSource=&src;

	if(!SameDimensions(*pSource))
	{
		if(SameChannels(*pSource)&&SameDataType(*pSource))
		{
			if(!Resize(*pSource))
			{
				fprintf(pErrorFile,"_Img::Convert(), error resizing image\n");
				return false;
			}
			else
				return true;
		}
		else
		{
				//at the end of this if-else have a temp image with converted dimensions
				//but possibly incorrect channels and datatype
			if(!temp.Create(Width(),Height(),src.Channels(),src.DataType()))
			{
				fprintf(pErrorFile,"_Img::Convert(), error creating temp image\n");
				return false;
			}

			if(!temp.Resize(*pSource))
			{
				fprintf(pErrorFile,"_Img::Convert(), error resizing temp image\n");
				return false;
			}
			
			pSource=&temp;
		}
	}

	if(!SameDataType(*pSource))
	{
		if(SameChannels(*pSource))
		{
			if(!ConvertDataType(*pSource)){
				fprintf(pErrorFile,"_Img::Convert(), Error converting data type\n");
				return false;
			}
			else
				return true;
		}
		else
		{
			temp2.Create(pSource->Width(),pSource->Height(),pSource->Channels(),DataType());
		
			if(!temp2.ConvertDataType(*pSource))
			{
				fprintf(pErrorFile,"_Img::Convert(), Error converting data type\n");
				return false;
			}
			pSource=&temp2;
			//fprintf(stderr,"temp2 %p\n",pSource);
		}
	}
	
	if(pSource->Channels()==3&&Channels()==1)
	{
		if(!ToGreyscale(*pSource)){
			fprintf(pErrorFile,"_Img::Convert(), Error converting image to greyscale\n");
			return false;
		}
	}
	else if(pSource->Channels()==1&&Channels()==3)
	{
		if(!To3ChannelGreyscale(*pSource)){
			fprintf(pErrorFile,"_Img::Convert(), Error converting image to 3 channel greyscale\n");
			return false;
		}
	}
	else
	{
		fprintf(pErrorFile,"_Img::Convert(), Unsupported no. of channels (this=%d), (src=%d)\n",
						Channels(),src.Channels());
		return false;
	}

	temp.Destroy();
	temp2.Destroy();
	

	return true;
}


bool _Img::ConvertDataType(const unsigned char *pData,bool scaleToRange)
{
	unsigned int sz=m_Hdr.w*m_Hdr.h*m_Hdr.channels;
	const unsigned char *_src;
	const unsigned long longRange=4294967295;

	switch(m_Hdr.dataType)
	{
	case IMG_UCHAR:	{
						memcpy(m_pData,pData,DataSz());
					} break;
	
	case IMG_LONG:	{
						if(scaleToRange)
						{
							unsigned char *_dst,*max;
							max=((unsigned char*)m_pData)+sz;
						
							for(_dst=(unsigned char*)m_pData,_src=pData;
								_dst<max;
								++_dst,++_src)
							{
								*_dst=(long) (IMG_LONG_MIN+(longRange*(*_src/255)));
							}
						}
						else
						{
							long *_dst,*max;
							max=((long*)m_pData)+sz;
						 
							for(_dst=(long*)m_pData,_src=pData;
								_dst<max;
								++_dst,++_src)
							{
								*_dst=*_src;
							}
						}
					} break;
	
	case IMG_FLOAT: {
						float *_dst,*max;
						max=((float*)m_pData)+sz;
						for(_dst=(float*)m_pData,_src=pData;
							_dst<max;
							++_dst,++_src)
						{
							*_dst=*_src;
						}
					} break;
	
	default:		{
						fprintf(pErrorFile,"_Img::ConvertDataType(), Data type unrecognised\n");
						return false;
					}
	}
	return true;
}

bool _Img::ConvertDataType(const long *pData,bool scaleToRange)
{
	unsigned int sz=m_Hdr.w*m_Hdr.h*m_Hdr.channels;
	const long *_src;
	const unsigned long srcRange=4294967295;
	
	switch(m_Hdr.dataType)
	{
	case IMG_UCHAR:{
						if(scaleToRange)
						{
							unsigned char *_dst,*max;
							max=((unsigned char*)m_pData)+sz;
						
							for(_dst=(unsigned char*)m_pData,_src=pData;
								_dst<max;
								++_dst,++_src)
							{
								*_dst=(unsigned char) (255*((*_src-IMG_LONG_MIN)/srcRange));
							}
						}
						else
						{
							unsigned char *_dst,*max;
							max=((unsigned char*)m_pData)+sz;
						
							for(_dst=(unsigned char*)m_pData,_src=pData;
								_dst<max;
								++_dst,++_src)
							{
								if(*_src>=255)
									*_dst=255;
								else if(*_src<=0)
									*_dst=0;
								else
									*_dst=(unsigned char)*_src;
							}
						}
					} break;
	
	case IMG_LONG:	{
						memcpy(m_pData,pData,DataSz());
					} break;	
	
	case IMG_FLOAT: {
						float *_dst,*max;
						max=((float*)m_pData)+sz;
						
						for(_dst=(float*)m_pData,_src=pData;
							_dst<max;
							++_dst,++_src)
						{
							*_dst=*_src;
						}
					} break;
	
	default:		{
						fprintf(pErrorFile,"_Img::ConvertDataType(), Data type unrecognised\n");
						return false;
					}
	}
	return true;
}

bool _Img::ConvertDataType(const float *pData)
{
	unsigned int sz=m_Hdr.w*m_Hdr.h*m_Hdr.channels;
	const float *_src;

	switch(m_Hdr.dataType)
	{
	case IMG_UCHAR:	{	
						float val;
						unsigned char *_dst,*max;
						max=((unsigned char*)m_pData)+sz;
						
						for(_dst=(unsigned char*)m_pData,_src=pData;
						_dst<max;
						++_dst,++_src)
						{
							val=*_src;
							if(val>=255)
								*_dst=255;
							else if(val<=0)
								*_dst=0;
							else
								*_dst=(unsigned char)val+0.5;
						}
					} break;
	case IMG_LONG:	{
						long *_dst,*max;
						max=((long*)m_pData)+sz;
						for(_dst=(long*)m_pData,_src=pData;
						_dst<max;
						++_dst,++_src)
						{
							*_dst=(long)*_src+0.5;
						}
					} break;
					
	case IMG_FLOAT: {
						memcpy(m_pData,pData,DataSz());
					} break;
	default:		{
				fprintf(pErrorFile,"_Img::ConvertDataType(), Data type unrecognised\n");
				return false;
					}
	}
	return true;
}

	
bool _Img::ConvertDataType(const AbstractImage &src,bool scaleToRange)
{
	if(this==&src)
		return false;

	if(!src.IsValid()) {
		fprintf(pErrorFile,"_Img::ConvertDataType(), image arg invalid)\n");
		return false;
	}

	if(!SameDimensions(src)){
		fprintf(pErrorFile,
"_Img::ConvertDataType(), Src dimensions (%d*%d) don't match this (%d*%d)\n",
		src.Width(),src.Height(),Width(),Height());
		return false;
	}

	if(!SameChannels(src)){
		fprintf(pErrorFile,
"_Img::ConvertDataType(), Src channels (%d) don't match this (%d)\n",
		src.Channels(),Channels());
		return false;
	}

	if(!IsValid()||SameDataType(src))
		return Copy(src);

	switch(src.DataType())
	{
	case IMG_UCHAR:{
					ConvertDataType((unsigned char*)src.Data(),scaleToRange);
				   }break;
	case IMG_LONG:{
					ConvertDataType((long*)src.Data(),scaleToRange);
				   }break;
	case IMG_FLOAT:{
					ConvertDataType((float*)src.Data());
				   }break;
	default:		{
					fprintf(pErrorFile,"_Img::ConvertDataType(), Data type unrecognised\n");
					return false;
					}
	}
	return true;
}


bool _Img::RgbToBgr()
{
	if(!IsValid())
	{
		fprintf(pErrorFile,"_Img::RgbToBgr(), this is invalid\n");
		return false;
	}

	if(m_Hdr.channels!=3)
	{
		fprintf(pErrorFile,"_Img::RgbToBgr(), this image has %d channels, expecting 3\n",
			m_Hdr.channels);
		return false;
	}

	
	fprintf(pErrorFile,"_Img::RgbToBgr(), to be dealt with\n");

	/*
	char *_to,*_from;
	char *limit=((char*)m_pData)+DataSz();
	unsigned int depth=m_Hdr.depth;
	unsigned int incr=3*depth;

	for(_to=(char*)m_pData,_from=(char*)src.Data();
		_to<limit;
		_to+=incr,_from+=incr)
	{
		memcpy(_to,_from+2,depth);
		memcpy(_to+1,_from+1,depth);
		memcpy(_to+2,_from,depth);
	}
	*/

	return true;
}


bool _Img::ToGreyscale(const AbstractImage &src)
{
	if (this==&src)
		return false;

	if(!src.IsValid())
	{
		Destroy();
		m_pData=NULL;
		fprintf(pErrorFile,"_src::ToGreyscale(), Arg src invalid\n");
		return false;
	}

	if(src.Channels()!=3)
	{
		fprintf(pErrorFile,"_Img::ToGreyscale(), Arg src does not have 3 channels\n");
		return false;
	}

	if(!IsValid()||!SameDimensions(src)||!SameDataType(src)){
		if(!Create(src.Width(),src.Height(),1,src.DataType())){
			fprintf(pErrorFile,"_Img::ToGreyscale(), Unable to create greyscale image\n");
			return false;
		}
	}

	char *_to,*_from;
	char *limit=((char*)m_pData)+DataSz();
	unsigned int depth=m_Hdr.depth;
	unsigned int rgbIncr=3*depth;

	switch(m_Hdr.dataType)
	{
	case IMG_UCHAR:
		{
			for(_to=(char*)m_pData,_from=(char*)src.Data();
			_to<limit;
			_to+=depth,_from+=rgbIncr)
			{
			((unsigned char*)_to)[0]=((((unsigned char*)_from)[0]
									+((unsigned char*)_from)[1]
									+((unsigned char*)_from)[2] )/3.0f)+0.5;
			}
		} break;
	case IMG_LONG:
		{
			for(_to=(char*)m_pData,_from=(char*)src.Data();
			_to<limit;
			_to+=depth,_from+=rgbIncr)
			{
				((long*)_to)[0]=((((long*)_from)[0]
									+((long*)_from)[1]
									+((long*)_from)[2] )/3.0f)+0.5;
			}
		} break;
	case IMG_FLOAT:
		{
			for(_to=(char*)m_pData,_from=(char*)src.Data();
			_to<limit;
			_to+=depth,_from+=rgbIncr)
			{
				((float*)_to)[0]=(((float*)_from)[0]
									+((float*)_from)[1]
									+((float*)_from)[2] )/3.0f;
			}
		} break;
	}
	return true;
}

bool _Img::To3ChannelGreyscale(const AbstractImage &src)
{
	if (this==&src)
		return false;

	if(!src.IsValid())
	{
		Destroy();
		m_pData=NULL;
		fprintf(pErrorFile,"_src::ToGreyscale(), Arg src invalid\n");
		return false;
	}

	if(!IsValid()||!SameDimensions(src)||!SameDataType(src)){
		if(!Create(src.Width(),src.Height(),3,src.DataType())){
			fprintf(pErrorFile,"_Img::To3ChannelGreyscale(), Unable to create greyscale image\n");
			return false;
		}
	}

	if(src.Channels()==1){
		char *_to,*_from;
		char *limit=((char*)m_pData)+DataSz();
		unsigned int depth=m_Hdr.depth;
		unsigned int rgbIncr=3*depth;

		switch(m_Hdr.dataType)
		{
		case IMG_UCHAR:
		{
			for(_to=(char*)m_pData,_from=(char*)src.Data();
			_to<limit;
			_to+=rgbIncr,_from+=depth)
			{
				((unsigned char*)_to)[0]=((unsigned char*)_from)[0];
				((unsigned char*)_to)[1]=((unsigned char*)_from)[0];
				((unsigned char*)_to)[2]=((unsigned char*)_from)[0];
			}
		} break;
		case IMG_LONG:
		{
			for(_to=(char*)m_pData,_from=(char*)src.Data();
			_to<limit;
			_to+=rgbIncr,_from+=depth)
			{
				((long*)_to)[0]=((long*)_from)[0];
				((long*)_to)[1]=((long*)_from)[0];
				((long*)_to)[2]=((long*)_from)[0];
			}
		} break;
		case IMG_FLOAT:
		{
			for(_to=(char*)m_pData,_from=(char*)src.Data();
			_to<limit;
			_to+=rgbIncr,_from+=depth)
			{
				((float*)_to)[0]=((float*)_from)[0];
				((float*)_to)[1]=((float*)_from)[0];
				((float*)_to)[2]=((float*)_from)[0];
			}
		} break;
		}
	}
	else if(src.Channels()==3)
	{
		char *_to,*_from;
		char *limit=((char*)m_pData)+DataSz();
		unsigned int depth=m_Hdr.depth;
		unsigned int rgbIncr=3*depth;

		switch(m_Hdr.dataType)
		{
		case IMG_UCHAR:
		{
			unsigned char val;
			for(_to=(char*)m_pData,_from=(char*)src.Data();
			_to<limit;
			_to+=rgbIncr,_from+=depth)
			{
				val=((((unsigned char*)_from)[0]
									+((unsigned char*)_from)[1]
									+((unsigned char*)_from)[2] )/3.0f)+0.5;
				((unsigned char*)_to)[0]=val;
				((unsigned char*)_to)[1]=val;
				((unsigned char*)_to)[2]=val;
			}
		} break;
		case IMG_LONG:
		{
			long val;
			for(_to=(char*)m_pData,_from=(char*)src.Data();
			_to<limit;
			_to+=rgbIncr,_from+=depth)
			{
				val=((((long*)_from)[0]
									+((long*)_from)[1]
									+((long*)_from)[2] )/3.0f)+0.5;
				((long*)_to)[0]=val;
				((long*)_to)[1]=val;
				((long*)_to)[2]=val;
			}
		} break;
		case IMG_FLOAT:
		{
			float val;
			for(_to=(char*)m_pData,_from=(char*)src.Data();
			_to<limit;
			_to+=rgbIncr,_from+=depth)
			{
				val=(((float*)_from)[0]
									+((float*)_from)[1]
									+((float*)_from)[2] )/3.0f;
				((float*)_to)[0]=val;
				((float*)_to)[1]=val;
				((float*)_to)[2]=val;
			}
		} break;
		}
	}
	return true;
}



static void maxminuc(const unsigned char *pData,
			 unsigned int size,unsigned char *pMax,unsigned char *pMin)
{
	const unsigned char *_i;
	
	*pMax = *pData;
	*pMin = *pData;
	
	for( 	_i=pData+1;
			_i < (pData+size);
			++_i
			)
	{
		if(*pMax<*_i)
		{
			*pMax= *_i;
		}
		else if(*pMin>*_i)
		{
			*pMin=*_i;
		}
	}
}

static void maxmini(const long *pData,
					unsigned int size,long *pMax,long *pMin)
{
	const long *_i;
	
	*pMax = *pData;
	*pMin = *pData;
	
	for( 	_i=pData+1;
			_i < (pData+size);
			++_i
			)
	{
		if(*pMax<*_i)
		{
			*pMax= *_i;
		}
		else if(*pMin>*_i)
		{
			*pMin=*_i;
		}
	}
}



static void maxminf(const float *pData,
			 unsigned int size, float *pMax, float *pMin)
{
	const float *_i;
	
	*pMax = *pData;
	*pMin = *pData;
	
	for( 	_i=pData+1;
			_i < (pData+size);
			++_i
			)
	{
		if(*pMax<*_i)
		{
			*pMax= *_i;
		}
		else if(*pMin>*_i)
		{
			*pMin=*_i;
		}
	}
}

bool _Img::ScaleF(const AbstractImage &src,float min,float range)
{
	if(!src.IsValid()){
		fprintf(pErrorFile,"_Img::ScaleF(), arg 'src' invalid\n");
		return false;
	}
	if(!IsValid()||DataType()!=IMG_FLOAT){
		Create(src.Width(),src.Height(),src.Channels(),IMG_FLOAT);
	}

	float srcMinf,srcRangef;

	switch(src.DataType())
	{
		case IMG_UCHAR:
		{
			unsigned char minuc,maxuc;
			maxminuc((unsigned char*)src.Data(),src.Width()*src.Height()*src.Channels(),&maxuc,&minuc);
			srcMinf=minuc;
			srcRangef=maxuc-srcMinf;

			float scale=range/srcRangef;
			float A=min-(scale*srcMinf);

			float *pLimit=(float*)m_pData+(m_Hdr.w*m_Hdr.h*m_Hdr.channels);

			float *_i;
			unsigned char *_src;
			for(_i=(float*)m_pData,
				_src=(unsigned char*)src.Data();
				_i<pLimit;++_i,++_src )
			{
				// *_i=min+(scale*(*_i-srcMinf));
				*_i=A+(scale* *_src);
			}
		} break;
		case IMG_LONG:
		{
			long mini,maxi;
			maxmini((long*)src.Data(),src.Width()*src.Height()*src.Channels(),&maxi,&mini);
			srcMinf=mini;
			srcRangef=maxi-srcMinf;

			float scale=range/srcRangef;
			float A=min-(scale*srcMinf);

			float *pLimit=(float*)m_pData+(m_Hdr.w*m_Hdr.h*m_Hdr.channels);
			long *_src;
			float *_i;
			for(_i=(float*)m_pData,
				_src=(long*)src.Data();
				_i<pLimit;++_i,++_src )
			{
				// *_i=min+(scale*(*_i-srcMinf));
				*_i=A+(scale* *_src);
			}
		} break;
		case IMG_FLOAT:
		{
			float maxf;
			maxminf((float*)src.Data(),src.Width()*src.Height()*src.Channels(),&maxf,&srcMinf);
			srcRangef=maxf-srcMinf;

			float scale=range/srcRangef;
			float A=min-(scale*srcMinf);

			float *pLimit=(float*)m_pData+(m_Hdr.w*m_Hdr.h*m_Hdr.channels);
			float *_src;
			float *_i;
			for(_i=(float*)m_pData,
				_src=(float*)src.Data();
				_i<pLimit;++_i,++_src )
			{
				// *_i=min+(scale*(*_i-srcMinf));
				*_i=A+(scale* *_src);
			}
		} break;
		default:
		{
			fprintf(pErrorFile,
				"_Img::ScaleF(), unrecognised data type: arg 'src' (label %d)\n",
				src.DataType());
		}
	}
	return true;
}

bool _Img::ScaleF(float min,float range)
{
	if(!IsValid()){
		fprintf(pErrorFile,"_Img::ScaleF(), This image invalid\n");
		return false;
	}
	if(DataType()!=IMG_FLOAT){
		fprintf(pErrorFile,
"CIplImg::ScaleF(), Only float data types supported (data label=%d)\n",DataType());
		return false;
	}

	float srcMinf,srcMaxf,srcRangef;
	
	maxminf((float*)m_pData,m_Hdr.w*m_Hdr.h*m_Hdr.channels,&srcMaxf,&srcMinf);
	srcRangef=srcMaxf-srcMinf;

	float scale=range/srcRangef;
	float A=min-(scale*srcMinf);

	float *pLimit=(float*)m_pData+(m_Hdr.w*m_Hdr.h*m_Hdr.channels);
	for(float *_i=(float*)m_pData;_i<pLimit;++_i )
	{
		// *_i=min+(scale*(*_i-srcMinf));
		*_i=A+(scale* *_i);
	}
	return true;
}


bool _Img::Destroy()
{
	if(m_pData){
		delete [] m_pData;
		m_pData=NULL;
	}
	
	memset(&m_Hdr,0,sizeof(_ImgHdr));
	return true;
}

//
//	Get Image attributes
//

inline unsigned int _Img::Width() const
{
	return m_Hdr.w;	
}

inline unsigned int _Img::WidthStep() const
{
	return Width()*Channels();
}

inline unsigned int _Img::WidthStepBytes() const
{
	switch(m_Hdr.dataType)
	{
	case IMG_UCHAR:{
						return Width()*Channels()*sizeof(unsigned char);
				   } break;
	case IMG_LONG:{
						return Width()*Channels()*sizeof(long);
				   } break;
	case IMG_FLOAT:{
						return Width()*Channels()*sizeof(float);
				   } break;
	default:		{
						fprintf(pErrorFile,"_Img::WidthStepBytes(), Unrecognised data type\n");
						return Width();
					}
	}
}

inline unsigned int _Img::Height() const
{
	return m_Hdr.h;	
}

inline unsigned int _Img::Channels() const
{
	return m_Hdr.channels;	
}

inline unsigned int _Img::Depth() const
{
	return m_Hdr.depth;	
}

inline short int _Img::DataType() const
{
	return m_Hdr.dataType;
}

inline const char* _Img::ClassID() const
{
	return typeid(this).name();
}

inline void* _Img::Header() const{
	return (void*)&m_Hdr;
}
	// returns NULL if image invalid
inline void* _Img::Data() const{
	return m_pData;	
}
	
	// returns NULL if coordinate out of bounds or data unallocated
inline void* _Img::Pixel2(unsigned int x,unsigned int y) const{
	if(x>=m_Hdr.w||y>=m_Hdr.h||!m_pData)
		return NULL;
	else return (char*)m_pData+(m_Hdr.depth*m_Hdr.channels*((y*m_Hdr.w)+x)); 
}


inline void _Img::Pixel(void *pixel,unsigned int x,unsigned int y) const{
	
	switch(m_Hdr.dataType){
	case IMG_UCHAR:
		{
			memcpy(pixel,((unsigned char*)m_pData)+(((y*m_Hdr.w)+x)*m_Hdr.channels),
						m_Hdr.channels*m_Hdr.depth); 
		}break;
	case IMG_LONG:
		{
			memcpy(pixel,((long*)m_pData)+(((y*m_Hdr.w)+x)*m_Hdr.channels),
						m_Hdr.channels*m_Hdr.depth);
		}break;
	case IMG_FLOAT:
		{
			memcpy(pixel,((float*)m_pData)+(((y*m_Hdr.w)+x)*m_Hdr.channels),
						m_Hdr.channels*m_Hdr.depth);
		} break;
	}
}

inline void _Img::Pixel(void *pixel,unsigned int index) const{
	
	switch(m_Hdr.dataType){
	case IMG_UCHAR:
		{
			memcpy(pixel,((unsigned char*)m_pData)+(index*m_Hdr.channels),m_Hdr.channels*m_Hdr.depth); 
		}break;
	case IMG_LONG:
		{
			memcpy(pixel,((long*)m_pData)+(index*m_Hdr.channels),m_Hdr.channels*m_Hdr.depth); 
		}break;
	case IMG_FLOAT:
		{
			memcpy(pixel,((float*)m_pData)+(index*m_Hdr.channels),m_Hdr.channels*m_Hdr.depth); 
		} break;
	}
}

inline void _Img::SetPixel(void *pixel,unsigned int index) const
{
	switch(m_Hdr.dataType){
	case IMG_UCHAR:
		{
			memcpy(((unsigned char*)m_pData)+(index*m_Hdr.channels),pixel,m_Hdr.channels*m_Hdr.depth);
		}break;
	case IMG_LONG:
		{
			memcpy(((long*)m_pData)+(index*m_Hdr.channels),pixel,m_Hdr.channels*m_Hdr.depth);
		}break;
	case IMG_FLOAT:
		{
			memcpy(((float*)m_pData)+(index*m_Hdr.channels),pixel,m_Hdr.channels*m_Hdr.depth);
		} break;
	}
}

inline void _Img::SetPixel(void *pixel,unsigned int x,unsigned int y) const
{
	switch(m_Hdr.dataType){
	case IMG_UCHAR:
		{
			memcpy(((unsigned char*)m_pData)+(((y*m_Hdr.w)+x)*m_Hdr.channels),
				pixel,m_Hdr.channels*m_Hdr.depth);
		}break;
	case IMG_LONG:
		{
			memcpy(((long*)m_pData)+(((y*m_Hdr.w)+x)*m_Hdr.channels),
				pixel,m_Hdr.channels*m_Hdr.depth);
		}break;
	case IMG_FLOAT:
		{
			memcpy(((float*)m_pData)+(((y*m_Hdr.w)+x)*m_Hdr.channels),
				pixel,m_Hdr.channels*m_Hdr.depth);
		} break;
	}
}

inline void* _Img::Pixel(unsigned int x,unsigned int y) const{
	
	switch(m_Hdr.dataType){
	case IMG_UCHAR:
		{
			return ((unsigned char*)m_pData)+(y*m_Hdr.w)+x; 
		}break;
	case IMG_LONG:
		{
			return ((long*)m_pData)+(y*m_Hdr.w)+x; 
		}break;
	case IMG_FLOAT:
		{
			return ((float*)m_pData)+(y*m_Hdr.w)+x; 
		} break;
	}
	return NULL;
}

inline void* _Img::Pixel(unsigned int index) const{
	
	switch(m_Hdr.dataType){
	case IMG_UCHAR:
		{
			return ((unsigned char*)m_pData)+index; 
		}break;
	case IMG_LONG:
		{
			return ((long*)m_pData)+index; 
		}break;
	case IMG_FLOAT:
		{
			return ((float*)m_pData)+index; 
		} break;
	}
	return NULL;
}


//
// Image format checks
//
inline bool _Img::IsValid() const
{
	if(!m_pData)
		return false;

	return true;
}

inline bool _Img::SameDepth(const AbstractImage &img) const
{
	if(Depth()!=img.Depth())
		return false;
	
	return true;
}

inline bool _Img::SameChannels(const AbstractImage &img) const
{
	if(Channels()!=img.Channels())
		return false;
	
	return true;
}

inline bool _Img::SameDimensions(const AbstractImage &img) const
{
	if(Width()!=img.Width()||Height()!=img.Height())
		return false;

	return true;
}

inline bool _Img::SameDataType(const AbstractImage &img) const
{
	if(DataType()==IMG_NULL_DATATYPE
		||img.DataType()==IMG_NULL_DATATYPE
			||DataType()!=img.DataType())
		return false;

	return true;
}

inline bool _Img::SameHdr(const _Img &img) const
{
	if(memcmp(&m_Hdr,&img.m_Hdr,sizeof(_ImgHdr))!=0)
		return false;
	
	return true;
}

inline bool _Img::SameClass(const AbstractImage &img) const
{
	//if(typeid(this)!=typeid(&img))
	if(strcmp(ClassID(),img.ClassID())!=0)
	{
		fprintf(pErrorFile,"_Img::SameClass() %s doesn't equal %s\n",ClassID(),img.ClassID());
		return false;
	}
	
	return true;
}

inline bool _Img::SameType(const AbstractImage &img) const
{
	if(!IsValid()||!img.IsValid())
		return false;
	//if(!SameClass(img))
	//	return false;
	if(!SameDimensions(img))
		return false;
	if(!SameDataType(img))
		return false;
	if(!SameChannels(img))
		return false;
	
	return true;
}

bool _Img::Add(const AbstractImage &img)
{
	if(!SameType(img)){
		fprintf(pErrorFile,"_Img::Add(), Images not same type\n");
		return false;
	}

	unsigned int sz=Width()*Height()*Channels();

	switch(m_Hdr.dataType)
	{
	
	case IMG_UCHAR:{
					unsigned char *_src,*_dst,*max;
					int val;
					max=((unsigned char*)m_pData)+sz;
					for(_dst=(unsigned char*)m_pData,_src=(unsigned char*)img.Data();
						_dst<max;
						++_dst,++_src)
					{
						val=*_src+*_dst;
						*_dst=val>255 ? 255 : val;
					}
			   } break;
	case IMG_LONG: {
					long *_src,*_dst,*max;
					max=((long*)m_pData)+sz;
					for(_dst=(long*)m_pData,_src=(long*)img.Data();
						_dst<max;
						++_dst,++_src)
					{
						*_dst+=*_src;
					}
			  } break;
	case IMG_FLOAT: {
					float *_src,*_dst,*max;
					max=((float*)m_pData)+sz;
					for(_dst=(float*)m_pData,_src=(float*)img.Data();
						_dst<max;
						++_dst,++_src)
					{
						*_dst+=*_src;
					}
				} break;
	default:	{
			fprintf(pErrorFile,"_Img::Add(), Data type unrecognised\n");
			return false;
			}
	}

	return true;
}


bool _Img::AddS(const void *pV)
{
	if(!IsValid()){
		fprintf(pErrorFile,"_Img::Add(), this img not valid\n");
		return false;
	}

	unsigned int sz=Width()*Height()*Channels();

	switch(m_Hdr.dataType)
	{
	
	case IMG_UCHAR:{
					unsigned char v=*((unsigned char*)pV);
					unsigned char *_dst,*max;
					int val;
					max=((unsigned char*)m_pData)+sz;
					for(_dst=(unsigned char*)m_pData;
						_dst<max;
						++_dst)
					{
						val=*_dst+v;
						*_dst=val>255 ? 255 : val;
					}
			   } break;
	case IMG_LONG: {
					long v=*((long*)pV);
					long *_dst,*max;
					max=((long*)m_pData)+sz;
					for(_dst=(long*)m_pData;
						_dst<max;
						++_dst)
					{
						*_dst+=v;
					}
			  } break;
	case IMG_FLOAT: {
					float v=*((float*)pV);
					float *_dst,*max;
					max=((float*)m_pData)+sz;
					for(_dst=(float*)m_pData;
						_dst<max;
						++_dst)
					{
						*_dst+=v;
					}
				} break;
	default:	{
			fprintf(pErrorFile,"_Img::Add(), Data type unrecognised\n");
			return false;
			}
	}

	return true;
}


bool _Img::AddS(const AbstractImage &img,const void *pV)
{
	if(!img.IsValid()){
		fprintf(pErrorFile,"_Img::Add(), arg 'img' not valid\n");
		return false;
	}

	if(!SameType(img)){
		if(!Create(img.Width(),img.Height(),img.Channels(),img.DataType())){
			fprintf(pErrorFile,"_Img::Add(), Error creating this image\n");
			return false;
		}
	}

	unsigned int sz=Width()*Height()*Channels();

	switch(m_Hdr.dataType)
	{
	
	case IMG_UCHAR:{
					unsigned char v=*((unsigned char*)pV);
					unsigned char *_src,*_dst,*max;
					int val;
					max=((unsigned char*)m_pData)+sz;
					for(_dst=(unsigned char*)m_pData,_src=(unsigned char*)img.Data();
						_dst<max;
						++_dst,++_src)
					{
						val=*_src+v;
						*_dst=val>255 ? 255 : val;
					}
			   } break;
	case IMG_LONG: {
					long v=*((long*)pV);
					long *_src,*_dst,*max;
					max=((long*)m_pData)+sz;
					for(_dst=(long*)m_pData,_src=(long*)img.Data();
						_dst<max;
						++_dst,++_src)
					{
						*_dst=*_src+v;
					}
			  } break;
	case IMG_FLOAT: {
					float v=*((float*)pV);
					float *_src,*_dst,*max;
					max=((float*)m_pData)+sz;
					for(_dst=(float*)m_pData,_src=(float*)img.Data();
						_dst<max;
						++_dst,++_src)
					{
						*_dst=*_src+v;
					}
				} break;
	default:	{
			fprintf(pErrorFile,"_Img::Add(), Data type unrecognised\n");
			return false;
			}
	}

	return true;
}

bool _Img::Add(const AbstractImage &img,const AbstractImage &img2)
{
	if(!img.SameType(img2)){
		fprintf(pErrorFile,"_Img::Add(), Images not same type\n");
		return false;
	}
	if(!SameType(img)){
		BlankCopy(img);
	}

	unsigned int sz=Width()*Height()*Channels();

	switch(m_Hdr.dataType)
	{
	
	case IMG_UCHAR:{
					unsigned char *_src,*_src2,*_dst,*max;
					int val;
					max=((unsigned char*)m_pData)+sz;
					for(_dst=(unsigned char*)m_pData,
						_src=(unsigned char*)img.Data(),
						_src2=(unsigned char*)img2.Data();
						_dst<max;
						++_dst,++_src,++_src2)
					{
						val=*_src+*_src2;
						*_dst=val>255 ? 255 : val;
					}
			   } break;
	case IMG_LONG: {
					long *_src,*_src2,*_dst,*max;
					max=((long*)m_pData)+sz;
					for(_dst=(long*)m_pData,
						_src=(long*)img.Data(),
						_src2=(long*)img2.Data();
						_dst<max;
						++_dst,++_src,++_src2)
					{
						*_dst=*_src+*_src2;
					}
			  } break;//////////////////////////////
	case IMG_FLOAT: {
					float *_src,*_src2,*_dst,*max;
					max=((float*)m_pData)+sz;
					for(_dst=(float*)m_pData,
						_src=(float*)img.Data(),
						_src2=(float*)img2.Data();
						_dst<max;
						++_dst,++_src,++_src2)
					{
						*_dst=*_src+*_src2;
					}
				} break;
	default:	{
			fprintf(pErrorFile,"_Img::Add(), Data type unrecognised\n");
			return false;
			}
	}

	return true;
}



bool _Img::Subtract(const AbstractImage &img)
{
	if(!SameType(img)){
		fprintf(pErrorFile,"_Img::Subtract() Images not same type\n");
		return false;
	}

	unsigned int sz=Width()*Height()*Channels();

	switch(m_Hdr.dataType)
	{
	
	case IMG_UCHAR:{
					unsigned char *_src,*_dst,*max;
					int val;
					max=((unsigned char*)m_pData)+sz;
					for(_dst=(unsigned char*)m_pData,_src=(unsigned char*)img.Data();
						_dst<max;
						++_dst,++_src)
					{
						val=*_dst-*_src;
						*_dst=val<0 ? 0 : val;
					}
			   } break;
	case IMG_LONG: {
					long *_src,*_dst,*max;
					max=((long*)m_pData)+sz;
					for(_dst=(long*)m_pData,_src=(long*)img.Data();
						_dst<max;
						++_dst,++_src)
					{
						*_dst-=*_src;
					}
			  } break;
	case IMG_FLOAT: {
					float *_src,*_dst,*max;
					max=((float*)m_pData)+sz;
					for(_dst=(float*)m_pData,_src=(float*)img.Data();
						_dst<max;
						++_dst,++_src)
					{
						*_dst-=*_src;
					}
				} break;
	default:	{
			fprintf(pErrorFile,"_Img::Subtract(), Data type unrecognised\n");
			return false;
			}
	}

	return true;
}

bool _Img::SubtractS(const void *pV)
{
	if(!IsValid()){
		fprintf(pErrorFile,"_Img::Subtract(), this img not valid\n");
		return false;
	}

	unsigned int sz=Width()*Height()*Channels();

	switch(m_Hdr.dataType)
	{
	
	case IMG_UCHAR:{
					unsigned char v=*((unsigned char*)pV);
					unsigned char *_dst,*max;
					int val;
					max=((unsigned char*)m_pData)+sz;
					for(_dst=(unsigned char*)m_pData;
						_dst<max;
						++_dst)
					{
						val=*_dst-v;
						*_dst=val<0 ? 0 : val;
					}
			   } break;
	case IMG_LONG: {
					long v=*((long*)pV);
					long *_dst,*max;
					max=((long*)m_pData)+sz;
					for(_dst=(long*)m_pData;
						_dst<max;
						++_dst)
					{
						*_dst-=v;
					}
			  } break;
	case IMG_FLOAT: {
					float v=*((float*)pV);
					float *_dst,*max;
					max=((float*)m_pData)+sz;
					for(_dst=(float*)m_pData;
						_dst<max;
						++_dst)
					{
						*_dst-=v;
					}
				} break;
	default:	{
			fprintf(pErrorFile,"_Img::Subtract(), Data type unrecognised\n");
			return false;
			}
	}

	return true;
}


bool _Img::SubtractS(const AbstractImage &img,const void *pV)
{
	if(!img.IsValid()){
		fprintf(pErrorFile,"_Img::Subtract(), arg 'img' not valid\n");
		return false;
	}

	if(!SameType(img)){
		if(!Create(img.Width(),img.Height(),img.Channels(),img.DataType())){
			fprintf(pErrorFile,"_Img::Subtract(), Error creating this image\n");
			return false;
		}
	}

	unsigned int sz=Width()*Height()*Channels();

	switch(m_Hdr.dataType)
	{
	
	case IMG_UCHAR:{
					unsigned char v=*((unsigned char*)pV);
					unsigned char *_src,*_dst,*max;
					int val;
					max=((unsigned char*)m_pData)+sz;
					for(_dst=(unsigned char*)m_pData,_src=(unsigned char*)img.Data();
						_dst<max;
						++_dst,++_src)
					{
						val=*_src-v;
						*_dst=val<0 ? 0 : val;
					}
			   } break;
	case IMG_LONG: {
					long v=*((long*)pV);
					long *_src,*_dst,*max;
					max=((long*)m_pData)+sz;
					for(_dst=(long*)m_pData,_src=(long*)img.Data();
						_dst<max;
						++_dst,++_src)
					{
						*_dst=*_src-v;
					}
			  } break;
	case IMG_FLOAT: {
					float v=*((float*)pV);
					float *_src,*_dst,*max;
					max=((float*)m_pData)+sz;
					for(_dst=(float*)m_pData,_src=(float*)img.Data();
						_dst<max;
						++_dst,++_src)
					{
						*_dst=*_src-v;
					}
				} break;
	default:	{
			fprintf(pErrorFile,"_Img::Subtract(), Data type unrecognised\n");
			return false;
			}
	}

	return true;
}

bool _Img::Subtract(const AbstractImage &img,const AbstractImage &img2)
{
	if(!img.SameType(img2)){
		fprintf(pErrorFile,"_Img::Subtract(), Images not same type\n");
		return false;
	}
	if(!SameType(img)){
		BlankCopy(img);
	}

	unsigned int sz=Width()*Height()*Channels();

	switch(m_Hdr.dataType)
	{
	
	case IMG_UCHAR:{
					unsigned char val,*_src,*_src2,*_dst,*max;
					max=((unsigned char*)m_pData)+sz;
					for(_dst=(unsigned char*)m_pData,
						_src=(unsigned char*)img.Data(),
						_src2=(unsigned char*)img2.Data();
						_dst<max;
						++_dst,++_src,++_src2)
					{
						val=*_src-*_src2;
						*_dst=val<0 ? 0 : val;
					}
			   } break;
	case IMG_LONG: {
					long *_src,*_src2,*_dst,*max;
					max=((long*)m_pData)+sz;
					for(_dst=(long*)m_pData,
						_src=(long*)img.Data(),
						_src2=(long*)img2.Data();
						_dst<max;
						++_dst,++_src,++_src2)
					{
						*_dst=*_src-*_src2;
					}
			  } break;
	case IMG_FLOAT: {
					float *_src,*_src2,*_dst,*max;
					max=((float*)m_pData)+sz;
					for(_dst=(float*)m_pData,
						_src=(float*)img.Data(),
						_src2=(float*)img2.Data();
						_dst<max;
						++_dst,++_src,++_src2)
					{
						*_dst=*_src-*_src2;
					}
				} break;
	default:	{
			fprintf(pErrorFile,"_Img::Subtract(), Data type unrecognised\n");
			return false;
			}
	}

	return true;
}


bool _Img::Multiply(const AbstractImage &img)
{
	if(!SameType(img)){
		fprintf(pErrorFile,"_Img::Multiply() Images not same type\n");
		return false;
	}

	unsigned int sz=Width()*Height()*Channels();

	switch(m_Hdr.dataType)
	{
	
	case IMG_UCHAR:{
					unsigned char *_src,*_dst,*max;
					int val;
					max=((unsigned char*)m_pData)+sz;
					for(_dst=(unsigned char*)m_pData,_src=(unsigned char*)img.Data();
						_dst<max;
						++_dst,++_src)
					{
						val=*_src* *_dst;
						*_dst=val>255 ? 255 : val;
					}
			   } break;
	case IMG_LONG: {
					long *_src,*_dst,*max;
					max=((long*)m_pData)+sz;
					for(_dst=(long*)m_pData,_src=(long*)img.Data();
						_dst<max;
						++_dst,++_src)
					{
						*_dst*=*_src;
					}
			  } break;
	case IMG_FLOAT: {
					float *_src,*_dst,*max;
					max=((float*)m_pData)+sz;
					for(_dst=(float*)m_pData,_src=(float*)img.Data();
						_dst<max;
						++_dst,++_src)
					{
						*_dst*=*_src;
					}
				} break;
	default:	{
			fprintf(pErrorFile,"_Img::Multiply(), Data type unrecognised\n");
			return false;
			}
	}

	return true;
}


bool _Img::MultiplyS(const void *pV)
{
	if(!IsValid()){
		fprintf(pErrorFile,"_Img::Multiply(), this img not valid\n");
		return false;
	}

	unsigned int sz=Width()*Height()*Channels();

	switch(m_Hdr.dataType)
	{
	
	case IMG_UCHAR:{
					unsigned char v=*((unsigned char*)pV);
					unsigned char *_dst,*max;
					int val;
					max=((unsigned char*)m_pData)+sz;
					for(_dst=(unsigned char*)m_pData;
						_dst<max;
						++_dst)
					{
						val=*_dst* v;
						*_dst=val>255 ? 255 : val;
					}
			   } break;
	case IMG_LONG: {
					long v=*((long*)pV);
					long *_dst,*max;
					max=((long*)m_pData)+sz;
					for(_dst=(long*)m_pData;
						_dst<max;
						++_dst)
					{
						*_dst*=v;
					}
			  } break;
	case IMG_FLOAT: {
					float v=*((float*)pV);
					float *_dst,*max;
					max=((float*)m_pData)+sz;
					for(_dst=(float*)m_pData;
						_dst<max;
						++_dst)
					{
						*_dst*=v;
					}
				} break;
	default:	{
			fprintf(pErrorFile,"_Img::Multiply(), Data type unrecognised\n");
			return false;
			}
	}

	return true;
}

bool _Img::MultiplyS(const AbstractImage &img,const void *pV)
{
	if(!img.IsValid()){
		fprintf(pErrorFile,"_Img::Multiply(), arg 'img' not valid\n");
		return false;
	}

	if(!SameType(img)){
		if(!BlankCopy(img)){
			fprintf(pErrorFile,"_Img::Multiply(), Error creating this image\n");
			return false;
		}
	}

	unsigned int sz=Width()*Height()*Channels();

	switch(m_Hdr.dataType)
	{
	
	case IMG_UCHAR:{
					unsigned char v=*((unsigned char*)pV);
					unsigned char *_src,*_dst,*max;
					int val;
					max=((unsigned char*)m_pData)+sz;
					for(_dst=(unsigned char*)m_pData,_src=(unsigned char*)img.Data();
						_dst<max;
						++_dst,++_src)
					{
						val=*_src* v;
						*_dst=val>255 ? 255 : val;
					}
			   } break;
	case IMG_LONG: {
					long v=*((long*)pV);
					long *_src,*_dst,*max;
					max=((long*)m_pData)+sz;
					for(_dst=(long*)m_pData,_src=(long*)img.Data();
						_dst<max;
						++_dst,++_src)
					{
						*_dst=*_src*v;
					}
			  } break;
	case IMG_FLOAT: {
					float v=*((float*)pV);
					float *_src,*_dst,*max;
					max=((float*)m_pData)+sz;
					for(_dst=(float*)m_pData,_src=(float*)img.Data();
						_dst<max;
						++_dst,++_src)
					{
						*_dst=*_src*v;
					}
				} break;
	default:	{
			fprintf(pErrorFile,"_Img::Multiply(), Data type unrecognised\n");
			return false;
			}
	}

	return true;
}

bool _Img::Multiply(const AbstractImage &img,const AbstractImage &img2)
{
	if(!img.SameType(img2)){
		fprintf(pErrorFile,"_Img::Multiply(), Images not same type\n");
		return false;
	}
	if(!SameType(img)){
		BlankCopy(img);
	}

	unsigned int sz=Width()*Height()*Channels();

	switch(m_Hdr.dataType)
	{
	
	case IMG_UCHAR:{
					unsigned char *_src,*_src2,*_dst,*max;
					int val;
					max=((unsigned char*)m_pData)+sz;
					for(_dst=(unsigned char*)m_pData,
						_src=(unsigned char*)img.Data(),
						_src2=(unsigned char*)img2.Data();
						_dst<max;
						++_dst,++_src,++_src2)
					{
						val=*_src* *_src2;
						*_dst=val>255 ? 255 : val;
					}
			   } break;
	case IMG_LONG: {
					long *_src,*_src2,*_dst,*max;
					max=((long*)m_pData)+sz;
					for(_dst=(long*)m_pData,
						_src=(long*)img.Data(),
						_src2=(long*)img2.Data();
						_dst<max;
						++_dst,++_src,++_src2)
					{
						*_dst=*_src* *_src2;
					}
			  } break;//////////////////////////////
	case IMG_FLOAT: {
					float *_src,*_src2,*_dst,*max;
					max=((float*)m_pData)+sz;
					for(_dst=(float*)m_pData,
						_src=(float*)img.Data(),
						_src2=(float*)img2.Data();
						_dst<max;
						++_dst,++_src,++_src2)
					{
						*_dst=*_src* *_src2;
					}
				} break;
	default:	{
			fprintf(pErrorFile,"_Img::Multiply(), Data type unrecognised\n");
			return false;
			}
	}

	return true;
}


///////////////////////////////////////////////
//Multiply Accumulate the input into this image
// same as mac1 in the old version
bool _Img::MultAcc(const AbstractImage &img,float v)
{
	if(!img.IsValid()){
		fprintf(pErrorFile,"_Img::MultAcc() arg image invalid\n");
		return false;
	}
	
	if (img.DataType()!=IMG_FLOAT){
		fprintf(pErrorFile,"_Img::MultAcc(), Requires Floating Point Image\n");
		return false;
	}
	
	if (img.Channels()!=1){
		fprintf(pErrorFile,"_Img::MultAcc(), Requires single channel Images\n");
		return false;
	}

	if(!SameType(img)){
		BlankCopy(img);
	}

	if(!SameClass(img)){
		fprintf(pErrorFile,"_Img::MultAcc(), Conversion from different abstract image formats currently unsupported\n");
		return false;
	}

	int x,y;
	float *pDst=(float*)Data();
	float *pSrc=(float*)img.Data();

		//to be safe there should be an offset for both images, 
		// or alternatively ensure that Data() returns the unpadded data for ipl images
	int yoffset=WidthStep();///sizeof(float);
	int pos=0;

	for (y=0;y<Height();y++){
		pos=y*yoffset;

		for (x=0;x<Width();x++){
			pDst[pos]+=pSrc[pos]*v;
			pos++;
		}
	}
	return true;
}

bool _Img::Divide(const AbstractImage &img)
{
	if(!SameType(img)){
		fprintf(pErrorFile,"_Img::Divide() Images not same type\n");
		return false;
	}

	unsigned int sz=Width()*Height()*Channels();

	switch(m_Hdr.dataType)
	{
	
	case IMG_UCHAR:{
					unsigned char val,*_src,*_dst,*max;
					max=((unsigned char*)m_pData)+sz;
					for(_dst=(unsigned char*)m_pData,_src=(unsigned char*)img.Data();
						_dst<max;
						++_dst,++_src)
					{
						val=((float)*_dst)/ *_src;
						*_dst=val<0 ? 0 : val;
					}
			   } break;
	case IMG_LONG: {
					long *_src,*_dst,*max;
					max=((long*)m_pData)+sz;
					for(_dst=(long*)m_pData,_src=(long*)img.Data();
						_dst<max;
						++_dst,++_src)
					{
						*_dst/=((float)*_src);
					}
			  } break;
	case IMG_FLOAT: {
					float *_src,*_dst,*max;
					max=((float*)m_pData)+sz;
					for(_dst=(float*)m_pData,_src=(float*)img.Data();
						_dst<max;
						++_dst,++_src)
					{
						*_dst/=*_src;
					}
				} break;
	default:	{
			fprintf(pErrorFile,"_Img::Divide(), Data type unrecognised\n");
			return false;
			}
	}

	return true;
}


bool _Img::Divide(const AbstractImage &img,float numThresh,float denomThresh)
{
	if(!SameType(img)){
		fprintf(pErrorFile,"_ImgImg::Divide() Images not same type\n");
		return false;
	}

	unsigned int sz=Width()*Height()*Channels();

	switch(m_Hdr.dataType)
	{
	
	case IMG_UCHAR:{
					unsigned char *_src,*_dst,*max;
					
					max=((unsigned char*)Data())+sz;
					for(_dst=(unsigned char*)Data(),_src=(unsigned char*)img.Data();
						_dst<max;
						++_dst,++_src)
					{
							if(*_dst>numThresh)
							{
								if (*_src>denomThresh){
									*_dst=(unsigned char) (0.5+((float)*_dst)/ *_src);
								}
								else
									*_dst=255;
							}
							else
								*_dst=0;
					}
			   } break;
	case IMG_LONG: {
					long *_src,*_dst,*max;
					max=((long*)Data())+sz;
					for(_dst=(long*)Data(),_src=(long*)img.Data();
						_dst<max;
						++_dst,++_src)
					{
						if((float)fabs((float)*_dst)>numThresh)
						{
							if ((float)fabs((float)*_src)>denomThresh){
								*_dst=((float)*_dst)/ *_src;
							}
							else
								*_dst=0; //NB this should be long max just gotta work it out
						}
						else
							*_dst=0;
					}
			  } break;
	case IMG_FLOAT: {
					float *_src,*_dst,*max;
					max=((float*)Data())+sz;
					for(_dst=(float*)Data(),_src=(float*)img.Data();
						_dst<max;
						++_dst,++_src)
					{
						if(fabs(*_dst)>numThresh)
						{
							if (fabs(*_src)>denomThresh){
								*_dst/= *_src;
							}
							else
								*_dst=1.0f/denomThresh; // should be largest
						}
						else
							*_dst=0;
					}
				} break;
	default:	{
					fprintf(pErrorFile,"_Img::Divide(), Data type unrecognised\n");
					return false;
				}
	}

	return true;
}

bool _Img::TestDivideF(	const AbstractImage &num,const AbstractImage &denom,
						float fNumThresh,float fDenThresh)
{
	if(!num.IsValid()){
		fprintf(pErrorFile,"_Img::TestDivideF(), image arg 'num' invalid\n");
		return false;
	}
	if(!num.SameType(denom)){
		fprintf(pErrorFile,"_Img::TestDivideF(), image args 'num' & 'denom' not same type\n");
		return false;
	}
	if(num.DataType()!=IMG_FLOAT){
		fprintf(pErrorFile,"_Img::TestDivideF(), Only float data type supported\n");
		return false;
	}
	if(!SameType(num)){
		if(!BlankCopy(num)){
		fprintf(pErrorFile,"_Img::TestDivideF(), Error creating this image\n");
		return false;
		}
	}
	
	unsigned int sz=Width()*Height()*Channels();

	float *_srcNum,*_srcDenom,*_dst;
	float *limit=((float*)Data())+sz;
	
	for(_dst=(float*)Data(),
		_srcNum=(float*)num.Data(),
		_srcDenom=(float*)denom.Data();
		_dst<limit;
		++_dst,++_srcNum,++_srcDenom)
	{
		if (fabs(*_srcNum)>fNumThresh&&fabs(*_srcDenom)>fDenThresh)
		{
			*_dst=*_srcNum/ *_srcDenom;
		}
		else
		{
			*_dst=0.0f;
		}
	}
	return true;
}


bool _Img::TestInvertF(const AbstractImage &src,float fDenThresh)
{
	if(!src.IsValid()){
		fprintf(pErrorFile,"_Img::TestInvertF(), src image invalid\n");
		return false;
	}

	if(src.DataType()!=IMG_FLOAT){
		fprintf(pErrorFile,"_Img::TestInvertF(), Only float data type supported\n");
		return false;
	}

	if(!SameType(src)){
		if(!BlankCopy(src)){
			fprintf(pErrorFile,"_Img::TestInvertF(), Error creating this image\n");
			return false;
		}
	}

	unsigned int sz=Width()*Height()*Channels();

	float *_srcDenom,*_dst;
	float *limit=((float*)Data())+sz;

	for(_dst=(float*)Data(),
		_srcDenom=(float*)src.Data();
		_dst<limit;
		++_dst,++_srcDenom)
	{
		if (fabs(*_srcDenom)>fDenThresh){
			*_dst=1.0f/(*_srcDenom);
		}
		else{
			*_dst=1.0f/fDenThresh;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////
//Used in McGM if val at index of test image < threshold, output image at corresponding
//index set to zero
bool _Img::ThreshToZeroF(const AbstractImage &test,float fThresh)
{
	if(this==&test){
		fprintf(pErrorFile,"_Img::ThreshToZeroF(), this==test\n");
		return false;
	}

	if(!IsValid()||DataType()!=IMG_FLOAT)
	{
		fprintf(pErrorFile,"_Img::ThreshToZeroF(), this image invalid or not float data type\n");
		return false;
	}
	if(!SameType(test))
	{
		fprintf(pErrorFile,"_Img::ThreshToZeroF(), this image different type to 'test'\n");
		return false;
	}
	
	int x,y;

	float *_t=(float*)test.Data();
	float *_dst=(float*)Data();

	int tOffset=test.WidthStepBytes()/sizeof(float);
	int dstOffset=WidthStepBytes()/sizeof(float);

	for (y=0;y<Height();y++)
	{
		_t=(float*)test.Data();
		_dst=(float*)Data();

		_t+=y*tOffset;
		_dst+=y*dstOffset;

		for (x=0;x<Width();x++)
		{
			if (fabs(*_t)<fThresh)
				*_dst=0;

			_dst++;_t++;
		}
	}
	return true;
}

bool _Img::ThreshToZeroF(const AbstractImage &src,const AbstractImage &test,float fThresh)

{
	if(this==&test||this==&src){
		fprintf(pErrorFile,"_Img::ThreshToZeroF(), this==src or this==test\n");
		return false;
	}

	if(!src.IsValid()||src.DataType()!=IMG_FLOAT)
	{
		fprintf(pErrorFile,"_Img::ThreshToZeroF(), arg 'src' invalid or not float data type\n");
		return false;
	}
	if(!src.SameType(test))
	{
		fprintf(pErrorFile,"_Img::ThreshToZeroF(), arg 'src' different type to 'test'\n");
		return false;
	}
	if(!IsValid()||!SameType(src)){
		if(!BlankCopy(src)){
			fprintf(pErrorFile,"_Img::ThreshToZeroF(), error creating this image\n");	
		}
	}

	int x,y;

	float *_t=(float*)test.Data();
	float *_src=(float*)src.Data();
	float *_dst=(float*)Data();

	int tOffset=test.WidthStepBytes()/sizeof(float);
	int srcOffset=src.WidthStepBytes()/sizeof(float);
	int dstOffset=WidthStepBytes()/sizeof(float);

	for (y=0;y<Height();y++)
	{
		_t=(float*)test.Data();
		_src=(float*)src.Data();
		_dst=(float*)Data();

		_t+=y*tOffset;
		_src+=y*srcOffset;
		_dst+=y*dstOffset;

		for (x=0;x<Width();x++)
		{
			if (fabs(*_t)<fThresh)
					*_dst=0;
			else
				*_dst=*_src;

			_src++;_dst++;_t++;
		}
	}
	return true;
}



bool _Img::Sq()
{
	if(!IsValid()){
		fprintf(pErrorFile,"_Img::Sq(), this img invalid\n");
		return false;
	}	
	
	if(m_Hdr.dataType!=IMG_FLOAT){
		fprintf(pErrorFile,"_Img::Sq(), this img not float (data label=%d)\n",m_Hdr.dataType);
		return false;
	}
	float *limit=((float*)m_pData)+(m_Hdr.w*m_Hdr.h*m_Hdr.channels);

	for(float *_i=(float*)m_pData;_i<limit;++_i){
		*_i*=*_i;
	}
	
	return true;
}

bool _Img::Sq(const AbstractImage &img)
{
	
	if(!img.IsValid()){
		fprintf(pErrorFile,"_Img::Sq(), arg img invalid\n");
		return false;
	}
	if(img.DataType()!=IMG_FLOAT){
		fprintf(pErrorFile,"_Img::Sq(), this img not float (data label=%d)\n",m_Hdr.dataType);
		return false;
	}

	if(!SameType(img)){
		BlankCopy(img);
	}
	
	
	float *_src,*_dst;
	float *limit=((float*)m_pData)+(m_Hdr.w*m_Hdr.h*m_Hdr.channels);

	for(_dst=(float*)m_pData,_src=(float*)img.Data();_dst<limit;++_dst,++_src){
		*_dst=*_src* *_src;
	}
	
	return true;
}



bool _Img::Sqrt()
{
	if(!IsValid()){
		fprintf(pErrorFile,"_Img::Sqrt(), this img invalid\n");
		return false;
	}	
	
	if(m_Hdr.dataType!=IMG_FLOAT){
		fprintf(pErrorFile,"_Img::Sqrt(), this img not float (data label=%d)\n",m_Hdr.dataType);
		return false;
	}
	float *limit=((float*)m_pData)+(m_Hdr.w*m_Hdr.h*m_Hdr.channels);

	for(float *_i=(float*)m_pData;_i<limit;++_i){
		if(*_i<0){
			fprintf(stderr,"_Img::Sqrt(), sqrt() arg -ve, setting to 0\n");
			*_i=0;
		}
		else
			*_i=sqrt(*_i);
	}
	return true;
}

bool _Img::Sqrt(const AbstractImage &img)
{
	if(!img.IsValid()){
		fprintf(pErrorFile,"_Img::Sqrt(), arg img invalid\n");
		return false;
	}
	if(img.DataType()!=IMG_FLOAT){
		fprintf(pErrorFile,"_Img::Sqrt(), this img not float (data label=%d)\n",m_Hdr.dataType);
		return false;
	}

	if(!SameType(img)){
		BlankCopy(img);
	}
	
	float *_src,*_dst;	
	float *limit=((float*)m_pData)+(m_Hdr.w*m_Hdr.h*m_Hdr.channels);

	for(_dst=(float*)m_pData,_src=(float*)img.Data();_dst<limit;++_dst,++_src){
		if(*_src<0){
			*_dst=0;
		}
		else
			*_dst=sqrt(*_src);
	}
	
	return true;
}



bool _Img::Abs()
{
	if(!IsValid()){
		fprintf(pErrorFile,"_Img::Abs(), this img invalid\n");
		return false;
	}	
	
	if(m_Hdr.dataType!=IMG_FLOAT){
		fprintf(pErrorFile,"_Img::Abs(), this img not float (data label=%d)\n",m_Hdr.dataType);
		return false;
	}
	
	float *limit=((float*)m_pData)+(m_Hdr.w*m_Hdr.h*m_Hdr.channels);
	for(float *_i=(float*)m_pData;_i<limit;++_i){
		*_i=fabs(*_i);
	}
	
	return true;
}

bool _Img::Abs(const AbstractImage &img)
{
	if(!img.IsValid()){
		fprintf(pErrorFile,"_Img::Abs(), arg img invalid\n");
		return false;
	}
	if(img.DataType()!=IMG_FLOAT){
		fprintf(pErrorFile,"_Img::Abs(), this img not float (data label=%d)\n",m_Hdr.dataType);
		return false;
	}

	if(!SameType(img)){
		BlankCopy(img);
	}
	
	float *_src,*_dst;
	float *limit=((float*)m_pData)+(m_Hdr.w*m_Hdr.h*m_Hdr.channels);

	for(_dst=(float*)m_pData,_src=(float*)img.Data();_dst<limit;++_dst,++_src){
		*_dst=fabs(*_src);
	}
	
	return true;
}


bool _Img::ATan2(const AbstractImage &XImg,const AbstractImage &YImg)
{
	if(!XImg.IsValid()){
		fprintf(pErrorFile,"_Img::ATan2(), arg XImg invalid\n");
		return false;
	}
	
	if (XImg.DataType()!=IMG_FLOAT){
		fprintf(pErrorFile,"_Img::ATan2(), Requires Floating Point Image\n");
		return false;
	}

	if (Channels()!=1){
		fprintf(pErrorFile,"_Img::ATan2(), Requires single channel Images\n");
		return false;
	}
	if (XImg.Channels()!=1){
		fprintf(pErrorFile,"_Img::ATan2(), Requires single channel Images\n");
		return false;
	}

	if (!XImg.SameType(YImg)){
		fprintf(pErrorFile,"_Img::ATan2(), Images not compatible!\n");
		return false;
	}

	if(!SameType(XImg)){
		BlankCopy(XImg);
	}


	int x,y;
	float *pim=(float*)Data();
	float *pimx=(float*)XImg.Data();
	float *pimy=(float*)YImg.Data();
	int offset=WidthStep();///sizeof(float);
	int offsetx=WidthStep();///sizeof(float);
	int offsety=WidthStep();///sizeof(float);

	for (y=0;y<Height();y++){
		pim=(float*)Data();
		pimx=(float*)XImg.Data();
		pimy=(float*)YImg.Data();
		pim+=y*offset;
		pimx+=y*offsetx;
		pimy+=y*offsety;
		for (x=0;x<Width();x++){
			if (*pimx==0 && *pimy==0)
				*(pim)=0.0f;
			else
				*(pim)=(float)atan2(*(pimx),*(pimy));
			pim++;pimx++;pimy++;
		}
	}
	return true;
}


bool _Img::ErrorCheckConvolve(const AbstractImage &src,const AbstractFilter &f)
{
	if(!src.IsValid()){
		fprintf(pErrorFile,"_Img::Convolve(), Arg 'src' invalid\n");
		return false;
	}

	if(!f.xyIsValid()){
		fprintf(pErrorFile,"_Img::Convolve(), Filter invalid\n");
		return false;
	}

	return true;
}

bool _Img::ErrorCheckTConvolve(const AbstractImage **srcs,
									unsigned int sz,
									const AbstractFilter &f)
{
	if(!f.tIsValid()){
		fprintf(pErrorFile,"_Img::TConvolve(), Temporal filter invalid\n");
		return false;
	}

	if(f.tDataType()!=IMG_FLOAT){
		fprintf(pErrorFile,"_Img::TConvolve(), Currently only floating point filtering supported\n");
		return false;
	}
		
	if(sz<f.tSize()){
		fprintf(pErrorFile,"_Img::TConvolve(), Arg sz (%d) too small for temporal filter (%d)\n",
					sz,f.tSize());
		return false;
	}	

	if(!srcs){
		fprintf(pErrorFile,"_Img::TConvolve(), Arg srcs=NULL\n");
		return false;
	}
	
	bool first=true;
	for(const AbstractImage **i=srcs;i<srcs+sz;++i){
		
		if(!first){
			if(!(*i)->SameType(**srcs)){
				fprintf(pErrorFile,"_Img::TConvolve(), Srcs have differing image formats\n");
				return false;
			}
		}
		
		if(!*i){
			fprintf(pErrorFile,"_Img::TConvolve(), Arg *srcs=NULL\n");
			return false;
		}
		
		if(!(*i)->IsValid()){
			fprintf(pErrorFile,"_Img::TConvolve(), Invalid src\n");
			return false;
		}

		if((*i)->DataType()!=f.tDataType()){
			fprintf(pErrorFile,"_Img::TConvolve(), Src datatype (%d) does not match filter (%d)\n",
								(*i)->DataType(),f.DataType());
			return false;
		}
		first=false;
	}

	return true;
}


bool _Img::Convolve(const AbstractImage &src,const AbstractFilter &f)
{
	if(!ErrorCheckConvolve(src,f))
		return false;
	
	_Img tempsrc;
	const AbstractImage *pSource;

	if(src.DataType()!=f.DataType()){
		tempsrc.Create(src.Width(),src.Height(),src.Channels(),f.DataType());
		tempsrc.ConvertDataType(src);
		pSource=&tempsrc;
	}
	else
		pSource=&src;

	if(!IsValid()||!SameType(*pSource))
		BlankCopy(*pSource);

	if(f.IsSep()){
		_Img midProcess;
		midProcess.BlankCopy(*pSource);
		
		switch(f.DataType())
		{
		case IMG_LONG:	{
					ConvSepi((long*)m_pData,(long*)midProcess.Data(),(long*)pSource->Data(),
								Width(),Height(),Channels(),
								(long*)f.XFilter(),(long*)f.YFilter(),f.Width(),f.Height());
						}break;
		case IMG_FLOAT:	{
					ConvSepf((float*)m_pData,(float*)midProcess.Data(),(float*)pSource->Data(),
								Width(),Height(),Channels(),
								(float*)f.XFilter(),(float*)f.YFilter(),f.Width(),f.Height());
						} break;
		default:
			fprintf(pErrorFile,"_Img::Convolve(), Invalid data type (%d)\n",f.DataType());
			return false;
		}
	}
	else{
		switch(f.DataType())
		{
			case IMG_LONG:{
				Conv2Di((long*)m_pData,(long*)pSource->Data(),Width(),Height(),
					Channels(),(long*)f.Filter(),f.Width(),f.Height());
				   } break;
			case IMG_FLOAT:{
				Conv2Df((float*)m_pData,(float*)pSource->Data(),Width(),Height(),
					Channels(),(float*)f.Filter(),f.Width(),f.Height());
				   } break;
			default:
			fprintf(pErrorFile,"_Img::Convolve(), Invalid data type (%d)\n",f.DataType());
			return false;
		}
	}

	return true;
}


bool _Img::Blur(unsigned int xSize,unsigned int ySize)
{
	if(!IsValid()){
		fprintf(pErrorFile,"_Img::Blur(), this img invalid\n");
		return false;
	}
	if ((xSize<2&&ySize<2)
			||xSize%2==0||ySize%2==0){
		fprintf(pErrorFile,
"_Img::Blur(), kernel arg sizes (%d,%d) incompatible, expecting odd no.s, one of which >2\n",
			xSize,ySize);
		return false;
	}
	switch(DataType())
	{
	case IMG_UCHAR:{
						unsigned char *pBlur;
						try
						{
							pBlur=new unsigned char[DataSz()];
						}
						catch(bad_alloc xa)
						{
							fprintf(pErrorFile,"_Img::Blur(), error allocating memory\n");
							return false;
						}

						if(!Bluruc(pBlur,(unsigned char*)m_pData,m_Hdr.w,m_Hdr.h,m_Hdr.channels,xSize,ySize))
						{
							fprintf(pErrorFile,"_Img::Blur(), error blurring image\n");
							delete [] pBlur;
							return false;
						}	
						delete [] m_pData;
						m_pData=pBlur;
				   } break;
	case IMG_LONG:{
						long *pBlur;
						try
						{
							pBlur=new long[DataSz()];
						}
						catch(bad_alloc xa)
						{
							fprintf(pErrorFile,"_Img::Blur(), error allocating memory\n");
							return false;
						}

						if(!Bluri(pBlur,(long*)m_pData,m_Hdr.w,m_Hdr.h,m_Hdr.channels,xSize,ySize))
						{
							fprintf(pErrorFile,"_Img::Blur(), error blurring image\n");
							delete [] pBlur;
							return false;
						}	
						delete [] m_pData;
						m_pData=pBlur;
				  } break;
	case IMG_FLOAT:{
						float *pBlur;
						try
						{
							pBlur=new float[DataSz()];
						}
						catch(bad_alloc xa)
						{
							fprintf(pErrorFile,"_Img::Blur(), error allocating memory\n");
							return false;
						}

						if(!Blurf(pBlur,(float*)m_pData,m_Hdr.w,m_Hdr.h,m_Hdr.channels,xSize,ySize))
						{
							fprintf(pErrorFile,"_Img::Blur(), error blurring image\n");
							delete [] pBlur;
							return false;
						}	
						delete [] m_pData;
						m_pData=pBlur;
				   } break;
	default:{
					fprintf(stderr,"_Img::Blur(), Datatype (%d) unrecognised\n",DataType());
					return false;
			}
	}
	return true;
}


bool _Img::BlurF(unsigned int size)
{
	if(!IsValid()||DataType()!=IMG_FLOAT){
		fprintf(pErrorFile,"_Img::BlurF(), this img invalid or not floating point \n");
		return false;
	}
	if (size<2||size%2==0){
		fprintf(pErrorFile,"_Img::BlurF(), arg size (%d) incompatible, expecting odd no. >2\n",size);
		return false;
	}

	float *pBlur;
	try
	{
		pBlur=new float[DataSz()];
	}
	catch(bad_alloc xa)
	{
		fprintf(pErrorFile,"_Img::BlurF(), error allocating memory\n");
		return false;
	}


	if(!Blurf(pBlur,(float*)m_pData,m_Hdr.w,m_Hdr.h,m_Hdr.channels,size,size))
	{
		fprintf(pErrorFile,"_Img::BlurF(), error blurring image\n");
		delete [] pBlur;
		return false;
	}	
	delete [] m_pData;
	m_pData=pBlur;

	return true;
}

bool _Img::TConvolve(	const AbstractImage **srcs,
						unsigned int sz,
						const AbstractFilter &f)
{
	if(!ErrorCheckTConvolve(srcs,sz,f))
		return false;

	if(!IsValid()||!SameType(**srcs)){
		if(!BlankCopy(**srcs)){
			fprintf(pErrorFile,"_Img::TConvolve(), Error recreating this image\n");
		}
	}

		//Floating point filtering operation
	_Img temp;
	if(!temp.Copy(*this)){
		fprintf(pErrorFile,"_Img::TConvolve(), Error copying image\n");
	}
		
		//First Value
	MultiplyS(*srcs[0],&((float*)f.TFilter())[0]);

	for (int t=1;t<f.tSize();t++) //tap 0 is always zero so start at 1
	{	
		if (fabs( ((float*)f.TFilter())[t] )>0.0001){
			temp.MultiplyS(*srcs[f.tSize()-t],&((float*)f.TFilter())[t]);
			Add(temp);
		}
	}
	return true;
}

	//uses opengl function to read bmp
bool _Img::ReadBMP(const char *fnm)
{
	Destroy();
	FILE *_f=NULL;

	_f=fopen(fnm,"r");							

	if(!_f)											
	{
		fprintf(pErrorFile,"Img::readbmp(), error reading from file:'%s'\n", fnm);
		return false;
	}
	
	fclose(_f);
	AUX_RGBImageRec *glim=auxDIBImageLoadA(fnm);							
	if(!glim||!glim->data)
	// HACK COS NO GLAUX
	//char *glim=NULL;
	//if(!glim)//||!glim->data)
	{
		fprintf(pErrorFile,"_Img::ReadBMP(), Error loading .bmp from file:'%s'\n",fnm);
		return false;
	}
	
	m_Hdr.channels=3;
	m_Hdr.depth= 1;
	m_Hdr.w=glim->sizeX;
	m_Hdr.h=glim->sizeY;
	m_Hdr.dataType=IMG_UCHAR;
	Alloc();
	memcpy(m_pData,glim->data,m_Hdr.w*m_Hdr.h*m_Hdr.channels);
	
	free(glim->data);					
	free(glim);
	
	return true;
}

bool _Img::WriteBMP(const char *fnm)
{
	BITMAPFILEHEADER fileheader;
	BITMAPINFO *bmpinfo_p;
	BITMAPINFOHEADER infoheader;
	struct RGBQUAD {
		unsigned char r,g,b,a;
	};
	
	if(!IsValid())
	{
		fprintf(pErrorFile,"Img::WriteBMP, Image Invalid"); 
		return false;
	}

	_Img temp;
	const _Img *toPrint_p;
	if(m_Hdr.dataType!=IMG_UCHAR)
	{
		temp.Create(Width(),Height(),Channels(),IMG_UCHAR);
		temp.ConvertDataType(*this);
		toPrint_p=&temp;
	}
	else
		toPrint_p=this;
	
	FILE *out = fopen(fnm, "wb");
	if (out==NULL)
		return false;

	infoheader.biSize=sizeof(BITMAPINFOHEADER);
	infoheader.biWidth=toPrint_p->m_Hdr.w;
	infoheader.biHeight=toPrint_p->m_Hdr.h;
	infoheader.biPlanes=1;			// should always be 1
	infoheader.biBitCount=toPrint_p->m_Hdr.channels*8;
	infoheader.biCompression=0;		// or BI_RGB;
	infoheader.biSizeImage=0;		// zero for uncompressed bitmaps
	infoheader.biXPelsPerMeter=0;
	infoheader.biYPelsPerMeter=0;
	infoheader.biClrUsed=0;			// zero means all colours are used 
	infoheader.biClrImportant=0;	// zero means all colours required
	
	size_t bmpinfosize=sizeof(BITMAPINFOHEADER);

	if (m_Hdr.channels==1)
	{
		bmpinfosize+=256*sizeof(RGBQUAD);
		bmpinfo_p=(BITMAPINFO*) new unsigned char[bmpinfosize];
		RGBQUAD *palette_p = (RGBQUAD *) ((unsigned char*)bmpinfo_p+sizeof(BITMAPINFOHEADER));
		unsigned char *_p = (unsigned char*)palette_p;
		for (int i=0; i<256; i++) {_p[0]=_p[1]=_p[2]=i; _p[3]=0; _p+=4;}
	}
	else 
		bmpinfo_p = (BITMAPINFO*) new unsigned char[bmpinfosize]; 

	bmpinfo_p->bmiHeader = infoheader;

	int linesize=toPrint_p->m_Hdr.w*toPrint_p->m_Hdr.channels;
	int linesizepluspad = linesize;

	while(linesizepluspad%4!=0)
		linesizepluspad++;

	int bytepad=linesizepluspad-linesize;

		//now have info necessary for file header
	fileheader.bfType='MB';
	fileheader.bfOffBits=sizeof(BITMAPFILEHEADER)+bmpinfosize;
	fileheader.bfSize=fileheader.bfOffBits+(((toPrint_p->m_Hdr.w*toPrint_p->m_Hdr.channels)+bytepad)*toPrint_p->m_Hdr.h);
	fileheader.bfReserved1=0;
	fileheader.bfReserved2=0;

	//bmpinfo_p->bmiHeader.biSizeImage=(((hdr.w*hdr.colours)+bytepad)*hdr.h);
	
		//print to file
	fwrite(&fileheader,sizeof(BITMAPFILEHEADER),1,out);
	fwrite(bmpinfo_p,bmpinfosize,1,out);

	unsigned char *dataoffset_p=(unsigned char*)toPrint_p->Data();//+infoheader.biSizeImage;
	int lineloop;
	unsigned char dummy[3]={0,0,0};

	for (lineloop=0;lineloop<infoheader.biHeight;lineloop++){
						//Decrement pointer to previous line
		fwrite(dataoffset_p,1,linesize,out);	//Write a line without Padding
		fwrite(&dummy,1,bytepad,out);			//Write bytepadding to dummy
		dataoffset_p+=linesize;	
	}
	
	fclose(out);

	delete [] bmpinfo_p;
	
	return true;
}


////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//__________________________ IMG SEQ________________________________ 
////////////////////////////////////////////////////////////////////////////


ImgSeq::ImgSeq()
{}

ImgSeq::ImgSeq(const ImgSeq &src)
{
	Copy(src);
}
	//rescales to w and h using gluScaleImage
//ImgSeq( const ImgSeq &imseq,	unsigned int w, unsigned int h );
		//copies a section of the image arg
//ImgSeq( const ImgSeq &imseq,	unsigned int minx, unsigned int miny,
	//								unsigned int w, unsigned int h );

ImgSeq::~ImgSeq()
{
	Destroy();
}
	

ImgSeq &ImgSeq::operator=(const ImgSeq &src)
{
	Copy(src);

	return *this;
}

const AbstractImage& ImgSeq::operator [](unsigned int index) const
{
	if(index>m_Seq.size()){
		fprintf(pErrorFile,"ImgSeq::operator[%d] out of bounds, limit = %d",index,m_Seq.size());		
	}
	return *m_Seq[index];
}

bool ImgSeq::Copy(const ImgSeq &src)
{
	if(this==&src) 
		return false;

	Destroy();
		//issues with non-const iterator
	//vector<AbstractImage*>::iterator _i;
	//_i=src.m_Seq.begin();

	//while(_i!=src.m_Seq.end())
	//{
	//	if(*_i){
	//		m_Seq.push_back((*_i)->NewCopy());
	//	}
	//	else
	//		m_Seq.push_back(NULL);
	//}

	for(int i=0;i<src.Size();++i)
	{
		if(src.m_Seq[i]){
			m_Seq.push_back(src.m_Seq[i]->NewCopy());
		}
		else
			m_Seq.push_back(NULL);
	}
	return true;
}


bool ImgSeq::Append(const ImgSeq &src)
{
	if(this==&src) 
		return false;


		//issues with non-const iterator
	//vector<AbstractImage*>::iterator _i;
	//_i=src.m_Seq.begin();

	//while(_i!=src.m_Seq.end())
	//{
	//	if(*_i){
	//		m_Seq.push_back((*_i)->NewCopy());
	//	}
	//	else
	//		m_Seq.push_back(NULL);
	//}

	for(int i=0;i<src.Size();++i)
	{
		if(src.m_Seq[i]){
			m_Seq.push_back(src.m_Seq[i]->NewCopy());
		}
		else
			m_Seq.push_back(NULL);
	}
	return true;
}

bool ImgSeq::Destroy()
{
	vector<AbstractImage*>::iterator _i;

	_i=m_Seq.begin();

	while(_i!=m_Seq.end()){
		delete *_i;
		*_i=NULL;
		_i++;
	}

	m_Seq.clear();

	return true;
}

unsigned int ImgSeq::Size() const
{
	return m_Seq.size();
}
	
bool ImgSeq::AllSameType() const
{
	if(m_Seq.empty()){
		fprintf(pErrorFile,"Imgseq::sametype(), sequence is empty\n");
		return false;
	}

	
	//vector<AbstractImage*>::iterator _i;
	//_i=(const AbstractImage*) m_Seq.begin();
	//_i++;
	
	//while(_i!=m_Seq.end()){
	//	if(!(*m_Seq[i])->SameType(**m_Seq.begin())
	//		return false;
	//	_i++;
	//}

	for(int i=1;i<m_Seq.size();++i)
	{
		if(!m_Seq[i]->SameType(*m_Seq[0]))
			return false;
	}
	
	return true;
}

bool ImgSeq::Convert(const ImgSeq &src,const AbstractImage &templateImg)
{
	if(this==&src){
		fprintf(pErrorFile,"ImgSeq::Convert(), this =const arg src\n");
		return false;
	}

	if(!templateImg.IsValid()){
		fprintf(pErrorFile,"ImgSeq::Convert(), template invalid\n");
		return false;
	}

	Destroy();

	//vector<AbstractImage*>::iterator _i;

//	_i=src.m_Seq.begin();

	//while(_i!=src.m_Seq.end())

	for(int i=0;i<src.Size();++i)
	{
		if(src.m_Seq[i]){
			m_Seq.push_back(templateImg.NewBlankCopy());
			m_Seq[i]->Convert(*src.m_Seq[i]);
		}
		else
			m_Seq.push_back(NULL);
	}

	return true;
}

const vector <AbstractImage*> &ImgSeq::Seq() const
{
	return m_Seq;
}

void ImgSeq::push_back(const AbstractImage &img)
{
	m_Seq.push_back(img.NewCopy());
}

static string getNumberedFileNm(	const char *pFnmPrefix,
									unsigned int print_i,
									const char*fnmsuffix,
									unsigned int nodigits)
{
	const unsigned int lineLength=100;
	char num[lineLength];
	string fnm="";
	string numformat="";
	unsigned int limit,expon,z;

	if(nodigits<lineLength){
		for(expon=0;expon<=nodigits;++expon)
		{
			limit=pow(10.0,(int)expon+1);
			if(print_i<limit)
			{
				for(z=expon;z<nodigits-1;++z){
					numformat+="0";
				}
			
				numformat+="%d";
				sprintf(num,numformat.c_str(),print_i);
				break;
			}
		}

		if(expon==nodigits+1)
			fprintf(pErrorFile,
		"getNumberedFileNm(), index (%d) too large for %d-digit format\n",
							print_i,nodigits);

		for(z=0;z<nodigits;++z)
			numformat+="9";
	}
	else{
		fprintf(pErrorFile,
		"getNumberedFileNm(), %d digits>fnm string capacity\n",
							nodigits);
	}

	if(pFnmPrefix)
		fnm+=pFnmPrefix;
		
	fnm+=num;

	if(fnmsuffix)
		fnm+=fnmsuffix;

	return fnm;
}

unsigned int ImgSeq::ReadBMP(	const char *pFnmPrefix,
								unsigned int minIndex,unsigned int maxIndex,
								unsigned int increment,unsigned int noDigits,
								const AbstractImage &templateImg
								)
{
	unsigned int fileIndex=0,noRead=0;
	string fnm;
	//char num[100];
	
	fileIndex=minIndex;

	while(fileIndex<=maxIndex)
	{
		fnm=getNumberedFileNm(pFnmPrefix,fileIndex,".bmp",noDigits);

		m_Seq.push_back(templateImg.New());
				
		std::cout << "Loading " << fnm << endl;
		//std::cout << "Loading " << fnm.c_str << endl;
		if(!m_Seq.back()->ReadBMP(fnm.c_str()))
		{
			m_Seq.pop_back();
			break;
		}
		fileIndex+=increment;
		noRead++;
	}
	return noRead;
}

unsigned int ImgSeq::WriteBMP(const char *pFnmPrefix,unsigned int fnmstartindex)
{
	unsigned int print_i;
	vector<AbstractImage*>::iterator _i;
	unsigned int noprinted = 0;

	if(m_Seq.empty())
		return 0;

	_i=m_Seq.begin();
	print_i=fnmstartindex;

	while(_i!=m_Seq.end())
	{
		if( !(*_i)->WriteBMP(getNumberedFileNm(pFnmPrefix,print_i,".bmp",4).c_str()))
		{
			fprintf(pErrorFile,"ImgSeq::WriteBMP(), Error writing img[%d]\n", 
													print_i-fnmstartindex);
		}
		else
			noprinted++;

		_i++; print_i++;
	}
return noprinted;
}


////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//__________________________GLOBAL FUNCTIONS________________________________ 
////////////////////////////////////////////////////////////////////////////

/*
//Implement our own version of the AbstractImage Division operator
void TestDiv(AbstractImage *pImNum,AbstractImage *pImDen,float fNumThresh,float fDenThresh)
{
	if (pImNum->Depth() != IPL_DEPTH_32F){return;}
	if (pImNum->Channels() != 1){fprintf(stdout,"McGM2005::TestDiv - Division Not supported for Channels>1\n");return;}

	if (!pImNum->IsSameTypeAs(pImDen)){fprintf(stdout,"TestDiv() - Images not the same!\n");return;}
	
	int x,y;
	float *num=(float*)pImNum->Data();
	float *den=(float*)pImDen->Data();
	int offset=pImNum->WidthStep()/sizeof(float);

	for (y=0;y<pImNum->Height();y++){
	num=(float*)pImNum->GetpData();
	den=(float*)pImDen->GetpData();
	num+=y*offset;
	den+=y*offset;
		for (x=0;x<pImNum->Width();x++){
			if (fabs(*(num))>fNumThresh){
				if (fabs(*(den))>fDenThresh){
					*(num)/=*(den);
				}
				else
				{
					*(num)=1.0f/fDenThresh;
				}
			}
			else
			{
				*(num)=0.0f;
			}
			num++;den++;
		}
	}
}
*/

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////   INTERPOLATION
///////////////////////////////////////////////////////////////////////////////////////////////////////////////


bool BilinearInterpolatePixel(	unsigned char *pDstPixel,const unsigned char *pSrcImg,
								unsigned int channels,
								unsigned int srcW, unsigned int srcH,
								float x, float y )
{
	float a, b;
	int floor_x, floor_y;

	if(x<=1||y<=1||x>=srcW-1||y>=srcH-1)
		return false;

	floor_x = (int) floor(x);
	floor_y = (int) floor(y);
	a=x-floor_x;
	b=y-floor_y;

	if(channels==1)
	{
		unsigned char top_l, top_r, bot_l, bot_r;

		top_l = *(pSrcImg + ((floor_y+1) * srcW) + floor_x );
		top_r = *(pSrcImg + ((floor_y+1) * srcW) + floor_x+1 );
		bot_l = *(pSrcImg + (floor_y * srcW) + floor_x );
		bot_r = *(pSrcImg + ((floor_y) * srcW) + floor_x+1);

		*pDstPixel=((1-b)*(1-a)*bot_l) + ((1-b)*a*bot_r) + (b*(1-a)*top_l) + (b*a*top_r);
	}
	else if(channels==3)
	{
		struct BILINTERP_RGB{
			unsigned char r,g,b;
		} top_l, top_r, bot_l, bot_r, *pRGBSrc,*pRGBDst;

		pRGBSrc=(BILINTERP_RGB*)pSrcImg;
		pRGBDst=(BILINTERP_RGB*)pDstPixel;

		top_l = *(pRGBSrc + ((floor_y+1) * srcW) + floor_x );
		top_r = *(pRGBSrc + ((floor_y+1) * srcW) + floor_x+1 );
		bot_l = *(pRGBSrc + (floor_y * srcW) + floor_x );
		bot_r = *(pRGBSrc + ((floor_y) * srcW) + floor_x+1);

		pRGBDst->r = ((1-b)*(1-a)*bot_l.r) + ((1-b)*a*bot_r.r) + (b*(1 - a)*top_l.r) + (b*a*top_r.r);
		pRGBDst->g = ((1-b)*(1-a)*bot_l.g) + ((1-b)*a*bot_r.g) + (b*(1 - a)*top_l.g) + (b*a*top_r.g);
		pRGBDst->b = ((1-b)*(1-a)*bot_l.b) + ((1-b)*a*bot_r.b) + (b*(1 - a)*top_l.b) + (b*a*top_r.b);
	}
	else
	{
		unsigned char top_l, top_r, bot_l, bot_r;

		for(int c=0;c<channels;++c)
		{
			top_l = *(pSrcImg + c+(channels*(((floor_y+1)*srcW)+floor_x)));
			top_r = *(pSrcImg + c+(channels*(((floor_y+1)*srcW)+floor_x+1)));
			bot_l = *(pSrcImg + c+(channels*((floor_y*srcW)+floor_x)));
			bot_r = *(pSrcImg + c+(channels*(((floor_y)*srcW)+floor_x+1)));

			pDstPixel[c]=((1-b)*(1-a)*bot_l) + ((1-b)*a*bot_r) + (b*(1-a)*top_l) + (b*a*top_r);
		}
	}
	return true;
}

bool BilinearInterpolatePixel(	long *pDstPixel,const long *pSrcImg,
								unsigned int channels,
								unsigned int srcW, unsigned int srcH,
								float x, float y )
{
	float a, b;
	int floor_x, floor_y;

	if(x<=1||y<=1||x>=srcW-1||y>=srcH-1)
		return false;

	floor_x = (int) floor(x);
	floor_y = (int) floor(y);
	a=x-floor_x;
	b=y-floor_y;

	if(channels==1)
	{
		long top_l, top_r, bot_l, bot_r;

		top_l = *(pSrcImg + ((floor_y+1) * srcW) + floor_x );
		top_r = *(pSrcImg + ((floor_y+1) * srcW) + floor_x+1 );
		bot_l = *(pSrcImg + (floor_y * srcW) + floor_x );
		bot_r = *(pSrcImg + ((floor_y) * srcW) + floor_x+1);

		*pDstPixel=((1-b)*(1-a)*bot_l) + ((1-b)*a*bot_r) + (b*(1-a)*top_l) + (b*a*top_r);
	}
	else if(channels==3)
	{
		struct BILINTERP_RGB{
			long r,g,b;
		} top_l, top_r, bot_l, bot_r, *pRGBSrc,*pRGBDst;

		pRGBSrc=(BILINTERP_RGB*)pSrcImg;
		pRGBDst=(BILINTERP_RGB*)pDstPixel;

		top_l = *(pRGBSrc + ((floor_y+1) * srcW) + floor_x );
		top_r = *(pRGBSrc + ((floor_y+1) * srcW) + floor_x+1 );
		bot_l = *(pRGBSrc + (floor_y * srcW) + floor_x );
		bot_r = *(pRGBSrc + ((floor_y) * srcW) + floor_x+1);

		pRGBDst->r = ((1-b)*(1-a)*bot_l.r) + ((1-b)*a*bot_r.r) + (b*(1 - a)*top_l.r) + (b*a*top_r.r);
		pRGBDst->g = ((1-b)*(1-a)*bot_l.g) + ((1-b)*a*bot_r.g) + (b*(1 - a)*top_l.g) + (b*a*top_r.g);
		pRGBDst->b = ((1-b)*(1-a)*bot_l.b) + ((1-b)*a*bot_r.b) + (b*(1 - a)*top_l.b) + (b*a*top_r.b);
	}
	else
	{
		long top_l, top_r, bot_l, bot_r;

		for(int c=0;c<channels;++c)
		{
			top_l = *(pSrcImg + c+(channels*(((floor_y+1)*srcW)+floor_x)));
			top_r = *(pSrcImg + c+(channels*(((floor_y+1)*srcW)+floor_x+1)));
			bot_l = *(pSrcImg + c+(channels*((floor_y*srcW)+floor_x)));
			bot_r = *(pSrcImg + c+(channels*(((floor_y)*srcW)+floor_x+1)));

			pDstPixel[c]=((1-b)*(1-a)*bot_l) + ((1-b)*a*bot_r) + (b*(1-a)*top_l) + (b*a*top_r);
		}
	}
	return true;
}


bool BilinearInterpolatePixel(	float *pDstPixel,const float *pSrcImg,
								unsigned int channels,
								unsigned int srcW, unsigned int srcH,
								float x, float y )
{
	float a, b;
	int floor_x, floor_y;

	if(x<=1||y<=1||x>=srcW-1||y>=srcH-1)
		return false;

	floor_x = (int) floor(x);
	floor_y = (int) floor(y);
	a=x-floor_x;
	b=y-floor_y;

	if(channels==1)
	{
		float top_l, top_r, bot_l, bot_r;

		top_l = *(pSrcImg + ((floor_y+1) * srcW) + floor_x );
		top_r = *(pSrcImg + ((floor_y+1) * srcW) + floor_x+1 );
		bot_l = *(pSrcImg + (floor_y * srcW) + floor_x );
		bot_r = *(pSrcImg + ((floor_y) * srcW) + floor_x+1);

		*pDstPixel=((1-b)*(1-a)*bot_l) + ((1-b)*a*bot_r) + (b*(1-a)*top_l) + (b*a*top_r);
	}
	else if(channels==3)
	{
		struct BILINTERP_RGB{
			float r,g,b;
		} top_l, top_r, bot_l, bot_r, *pRGBSrc,*pRGBDst;

		pRGBSrc=(BILINTERP_RGB*)pSrcImg;
		pRGBDst=(BILINTERP_RGB*)pDstPixel;

		top_l = *(pRGBSrc + ((floor_y+1) * srcW) + floor_x );
		top_r = *(pRGBSrc + ((floor_y+1) * srcW) + floor_x+1 );
		bot_l = *(pRGBSrc + (floor_y * srcW) + floor_x );
		bot_r = *(pRGBSrc + ((floor_y) * srcW) + floor_x+1);

		pRGBDst->r = ((1-b)*(1-a)*bot_l.r) + ((1-b)*a*bot_r.r) + (b*(1 - a)*top_l.r) + (b*a*top_r.r);
		pRGBDst->g = ((1-b)*(1-a)*bot_l.g) + ((1-b)*a*bot_r.g) + (b*(1 - a)*top_l.g) + (b*a*top_r.g);
		pRGBDst->b = ((1-b)*(1-a)*bot_l.b) + ((1-b)*a*bot_r.b) + (b*(1 - a)*top_l.b) + (b*a*top_r.b);
	}
	else
	{
		float top_l, top_r, bot_l, bot_r;

		for(int c=0;c<channels;++c)
		{
			top_l = *(pSrcImg + c+(channels*(((floor_y+1)*srcW)+floor_x)));
			top_r = *(pSrcImg + c+(channels*(((floor_y+1)*srcW)+floor_x+1)));
			bot_l = *(pSrcImg + c+(channels*((floor_y*srcW)+floor_x)));
			bot_r = *(pSrcImg + c+(channels*(((floor_y)*srcW)+floor_x+1)));

			pDstPixel[c]=((1-b)*(1-a)*bot_l) + ((1-b)*a*bot_r) + (b*(1-a)*top_l) + (b*a*top_r);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////   INNER AND OUTER PRODUCTS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void InnerProd(const AbstractImage& src1x,const AbstractImage& src1y,const AbstractImage& src1t,
			   const AbstractImage& src2x,const AbstractImage& src2y,const AbstractImage& src2t,
			   AbstractImage& dst)
{
	AbstractImage* pTemp = src1x.NewBlankCopy();

	dst.Multiply(src1x,src2x);
	pTemp->Multiply(src1y,src2y);

	dst.Add(*pTemp);

	pTemp->Multiply(src1t,src2t);
	dst.Add(*pTemp);

	//iplMultiply(src1x->GetpImg(), src2x->Getsrcg(), src3->GetpImg());
	//iplMultiply(pIm1y->GetpImg(), pIm2y->GetpImg(), Temp->GetpImg());

	//iplAdd(pIm3->GetpImg(), Temp->GetpImg(), pIm3->GetpImg());

	//iplMultiply(pIm1t->GetpImg(), pIm2t->GetpImg(), Temp->GetpImg());

	//iplAdd(pIm3->GetpImg(), Temp->GetpImg(), pIm3->GetpImg());

	delete pTemp;
}


void OuterProd(	const AbstractImage& src1x,const AbstractImage& src1y,const AbstractImage& src1t,
				const AbstractImage& src2x,const AbstractImage& src2y,const AbstractImage& src2t,
				AbstractImage& dstx, AbstractImage& dsty, AbstractImage& dstt)
{
	AbstractImage* pTemp = src1x.NewBlankCopy();

	dstx.Multiply(src1y,src2t);
	pTemp->Multiply(src1t,src2y);
	dstx.Subtract(dstx,*pTemp);

		// y component
	dsty.Multiply(src2x,src1t);
	pTemp->Multiply(src2t,src1x);
	dsty.Subtract(dsty,*pTemp);

		// t component
	dstt.Multiply(src1x,src2y);
	pTemp->Multiply(src1y,src2x);
	dstt.Subtract(dstt,*pTemp);

	// x component
	//iplMultiply(pIm1y->GetpImg(), pIm2t->GetpImg(), pDstx->GetpImg());
	//iplMultiply(pIm1t->GetpImg(), pIm2y->GetpImg(), Temp->GetpImg());
	//iplSubtract(pDstx->GetpImg(), Temp->GetpImg(), pDstx->GetpImg());

	// y component
	//iplMultiply(pIm2x->GetpImg(), pIm1t->GetpImg(), pDsty->GetpImg());
	//iplMultiply(pIm2t->GetpImg(), pIm1x->GetpImg(), Temp->GetpImg());
	//iplSubtract(pDsty->GetpImg(), Temp->GetpImg(), pDsty->GetpImg());

	// t component
	//iplMultiply(pIm1x->GetpImg(), pIm2y->GetpImg(), pDstt->GetpImg());
	//iplMultiply(pIm1y->GetpImg(), pIm2x->GetpImg(), Temp->GetpImg());
	//iplSubtract(pDstt->GetpImg(), Temp->GetpImg(), pDstt->GetpImg());

	delete pTemp;
	
}

void OuterProdThresh(	const AbstractImage& src1x,const AbstractImage& src1y,const AbstractImage& src1t,
						const AbstractImage& src2x,const AbstractImage& src2y,const AbstractImage& src2t, 
						AbstractImage& dstx, AbstractImage& dsty, AbstractImage& dstt, 
						float fThresh)
{
	static AbstractImage* pTemp1;
	static AbstractImage* pTemp2;
	static AbstractImage* pTemp3;
	static AbstractImage* pTemp4;
	static bool firsttime=true;

	//if(firsttime)
	{
		pTemp1 = src1x.NewBlankCopy();
		pTemp2 = src1x.NewBlankCopy();
		pTemp3 = src1x.NewBlankCopy();
		pTemp4 = src1x.NewBlankCopy();
		firsttime=false;
	}

	InnerProd(src1x,src1y,src1t,src1x,src1y,src1t,*pTemp1);
	InnerProd(src2x,src2y,src2t,src2x,src2y,src2t,*pTemp2);

	//Temp1->sqrt();
	//Temp2->sqrt();

	pTemp3->Multiply(*pTemp1,*pTemp2);

	OuterProd(src1x,src1y,src1t,src2x,src2y,src2t,dstx,dsty,dstt);

	pTemp1->Copy(dstx);
	pTemp2->Copy(dsty);
	pTemp4->Copy(dstt);

	pTemp1->Sq();
	pTemp2->Sq();
	pTemp4->Sq();

	pTemp1->Add(*pTemp2);
	pTemp1->Add(*pTemp4);

	//Temp1->sqrt();

	pTemp1->Divide(*pTemp3,0.000001f,0.000001f);

	//Temp1->sqrt();

	fThresh=fThresh*fThresh;
	dstx.ThreshToZeroF(*pTemp1,fThresh);
	dsty.ThreshToZeroF(*pTemp1,fThresh);
	dstt.ThreshToZeroF(*pTemp1,fThresh);

	
	delete pTemp1;
	delete pTemp2;
	delete pTemp3;
	delete pTemp4;
}


// MATH FUNCS used to generate kernels______________________
// Taken from class IPLFilter
// Should be identical to those in VXFilter
/*
double _Filter::Dolg(unsigned int pos,unsigned int order,double alpha,double tau)
{
	double lvar;
	double lg[3];
	double sf=0.0;

	if (order>2){fprintf(pErrorFile,"Dolg:Invalid Order (>2) (%d)\n",order);return 0.0;}
	
	sf = sqrt(3.14159265358979) * tau * alpha * exp(tau*tau/4.0);		
	lvar = (double) pos;
	if (lvar<0.00001){lvar=0.00001;}

	// log gaussian
		lg[0]=exp(-(log(lvar/alpha)/tau)*(log(lvar/alpha)/tau))/sf;
		
	// 1st diff

		lg[1] = -2.0 * ( (log(lvar/alpha))/(tau*tau*lvar) ) * lg[0];

	//2nd Diff
		lg[2]= lg[1]*(-2.0*(log(lvar/alpha)/(tau*tau*lvar)))- 
			(lg[0]*2.0*(1.0-log(lvar/alpha)))/(tau*tau*lvar*lvar);

	//fprintf(pErrorFile,"Dolg %d tap %d = %f\n",order,pos,lg[order]);
	return lg[order];
}

double _Filter::Gaussian(double pos,double sig){
	double t=(sig*sig)/2.0;
	double norm=sqrt(4.0*3.1415926*t);
	double expval = (pos*pos)/(4*t);
	double filtval = exp(-expval)/norm;
	return filtval;
}

//Used to Generate DOG polynomials
double _Filter::HermiteFunction(double x,double sig,int ord)
{
	if (ord==0){return (1.0);}
	int m,top,pass;
	double ans,p,p2,a,b;
	double s=sig*sig/2.0;
	double dord=(double)ord;
	top=(ord/2) - ( (ord%2)/2);
	ans=0.0;

	for(m=0;m<=top;m++) {
		p=dord-2*(double)m;
		p2=dord-(double)m;
		pass=(ord-2*m);
		a=(2.0*x);
		b=(4.0*s);
		ans= ans+ pow(-1.0,(double)m)*((Herpow(a,p)/Herpow(b,p2) )/(Fac(m)*Fac(pass)));
	}

	ans *= Fac(ord);

	return ans;
}

//Compute power in robust manner
double _Filter::Herpow(double a1,double b1)
{
	double z;
	if(a1==0.0 && b1==0){z=1.0;}
	else {z=pow(a1,b1);}
	return(z);
}
//Binomial Number generation
int _Filter::Binomial(int n,int r)
{
	float dnmr;
	float quo;
	dnmr=(float)(Fac(r)*Fac(n-r));
	quo=((float)Fac(n))/dnmr;
	return((int)quo);
}
//Factorial
int _Filter::Fac(int x)
{
	int ans=1, h;
	if (x==0) {return 1;} 
	if (x==1) {return 1;}
	if (x==2) {return 2;}
	else {
		for (h=x; h>1;h--) ans = ans * h;
		return ans;
	}
}

*/


// 2D CONVOLUTION __________________________________________________________


// Performs 2D convolution with Floating point kernel to produce floating point output.  Output image
// must be reserved before calling this function.
bool Conv2Di(	long *imgOut,const long *imgIn,
				unsigned int imgW,int imgH,unsigned int channels,
				const long *kernel,
				unsigned int kXSize,unsigned int kYSize)
{
	int x,y,kx,ky;

	if(!imgOut||!imgIn||!kernel)
		return false;

	int xend=imgW-kXSize;
	int yend=imgH-kYSize;

	if(xend<0||yend<0)
		return false;
	
	int xstart=0;
	int ystart=0;
	
		// aja, changed offset to xoffset and yoffset
	int xoffset=(int)((kXSize-1)/2.0);
	int yoffset=(int)((kYSize-1)/2.0);
	
			//aja, added if else for speed up
	if(channels==1)
	{
		float sum;
		for (y=ystart;y<yend;y++){
			for (x=xstart;x<xend;x++){
				sum = 0;
				for (ky=0;ky<kYSize;ky++){
					for (kx=0;kx<kXSize;kx++){
									//aja, changed m_nSupport to imgW beloimgW
						sum+=*(imgIn+(((y+ky)*imgW)+(x+kx)))**(kernel+(ky*kXSize)+kx);
				}}
			*(imgOut+(((y+yoffset)*imgW)+(x+xoffset)))=sum;
			}
		}
	}
	else if(channels==3)
	{
		//const float *in_i;
		//float *out_i, kernel_val;
		long accum[3]={0,0,0};
	
		for (y=ystart;y<yend;y++){
			for (x=xstart;x<xend;x++){
				accum[0]=accum[1]=accum[2]=0.0;
				//fprintf(pErrorFile,"%d %d\n",x,y);
				//aja added index
				//out_i=imgOut+((((y+yoffset)*imgW)+(x+xoffset))*channels);
				
				for (ky=0;ky<kYSize;ky++){
					for (kx=0;kx<kXSize;kx++){
						//in_i=imgIn+(((y+ky)*imgW)+(x+kx))*channels;
						//kernel_val=kernel[(ky*kXSize)+kx];
						//
						//aja removed for loop
						//for (c=0;c<channels;c++){				
						//	accum[c]+=in_i[c]*kernel_val;
						//}
					
						//accum[0]+=in_i[0]*kernel_val;
						//accum[1]+=in_i[1]*kernel_val;
						//accum[2]+=in_i[2]*kernel_val;

				//the below seems the fastest option (i.e. faster than above indexing strategy)
				//presumably summat to do with compiler optimisation
					accum[0]+=*(imgIn+(((y+ky)*imgW)+(x+kx))*channels)**(kernel+(ky*kXSize)+kx);
					accum[1]+=*(imgIn+(((y+ky)*imgW)+(x+kx))*channels+1)**(kernel+(ky*kXSize)+kx);
					accum[2]+=*(imgIn+(((y+ky)*imgW)+(x+kx))*channels+2)**(kernel+(ky*kXSize)+kx);
				}}
				*(imgOut+((((y+yoffset)*imgW)+(x+xoffset))*channels))=accum[0];
				*(imgOut+((((y+yoffset)*imgW)+(x+xoffset))*channels)+1)=accum[1];
				*(imgOut+((((y+yoffset)*imgW)+(x+xoffset))*channels)+2)=accum[2];
				//out_i[0]=accum[0];
				//out_i[1]=accum[1];
				//out_i[2]=accum[2];
			}
		}
	}
	else
	{
		unsigned int c=0;
		int accum=0;
	
		for(c=0;c<channels;++c){
			for (y=ystart;y<yend;y++){
				for (x=xstart;x<xend;x++){
					accum=0.0;			
				//out_i=imgOut+((((y+yoffset)*imgW)+(x+xoffset))*channels);
				
					for (ky=0;ky<kYSize;ky++){
						for (kx=0;kx<kXSize;kx++){
							//in_i=imgIn+(((y+ky)*imgW)+(x+kx))*channels;
							//kernel_val=kernel[(ky*kXSize)+kx];
								//aja removed for loop
							//for (c=0;c<channels;c++){				
							//	accum[c]+=in_i[c]*kernel_val;
							//}
							accum+=*(imgIn+(((y+ky)*imgW)+(x+kx))*channels+c)**(kernel+(ky*kXSize)+kx);
						}
					}
					*(imgOut+((((y+yoffset)*imgW)+(x+xoffset))*channels)+c)=accum;
				//out_i[0]=accum[0];
				//out_i[1]=accum[1];
				//out_i[2]=accum[2];
				}
			
			}
		}		
	}
	return true;
}


bool Conv2Df(	float *imgOut,const float *imgIn,
				unsigned int imgW,int imgH,unsigned int channels,
				const float *kernel,
				unsigned int kXSize,unsigned int kYSize)
{
	int x,y,kx,ky;

	if(!imgOut||!imgIn||!kernel)
		return false;

	int xend=imgW-kXSize;
	int yend=imgH-kYSize;

	if(xend<0||yend<0)
		return false;
	
	int xstart=0;
	int ystart=0;
	
		// aja, changed offset to xoffset and yoffset
	int xoffset=(int)((kXSize-1)/2.0);
	int yoffset=(int)((kYSize-1)/2.0);
	
		//aja, added if else for speed up
	if(channels==1)
	{
		float sum;
		for (y=ystart;y<yend;y++){
			for (x=xstart;x<xend;x++){
				sum = 0;
				for (ky=0;ky<kYSize;ky++){
					for (kx=0;kx<kXSize;kx++){
									//aja, changed m_nSupport to imgW beloimgW
						sum+=*(imgIn+(((y+ky)*imgW)+(x+kx)))**(kernel+(ky*kXSize)+kx);
				}}
			*(imgOut+(((y+yoffset)*imgW)+(x+xoffset)))=sum;
			}
		}
	}
	else if(channels==3)
	{
		//const float *in_i;
		//float *out_i, kernel_val;
		double accum[3]={0,0,0};
	
		for (y=ystart;y<yend;y++){
			for (x=xstart;x<xend;x++){
				accum[0]=accum[1]=accum[2]=0.0;
				//fprintf(pErrorFile,"%d %d\n",x,y);
				//aja added index
				//out_i=imgOut+((((y+yoffset)*imgW)+(x+xoffset))*channels);
				
				for (ky=0;ky<kYSize;ky++){
					for (kx=0;kx<kXSize;kx++){
						//in_i=imgIn+(((y+ky)*imgW)+(x+kx))*channels;
						//kernel_val=kernel[(ky*kXSize)+kx];
						//
						//aja removed for loop
						//for (c=0;c<channels;c++){				
						//	accum[c]+=in_i[c]*kernel_val;
						//}
					
						//accum[0]+=in_i[0]*kernel_val;
						//accum[1]+=in_i[1]*kernel_val;
						//accum[2]+=in_i[2]*kernel_val;

				//the below seems the fastest option (i.e. faster than above indexing strategy)
				//presumably summat to do with compiler optimisation
					accum[0]+=*(imgIn+(((y+ky)*imgW)+(x+kx))*channels)**(kernel+(ky*kXSize)+kx);
					accum[1]+=*(imgIn+(((y+ky)*imgW)+(x+kx))*channels+1)**(kernel+(ky*kXSize)+kx);
					accum[2]+=*(imgIn+(((y+ky)*imgW)+(x+kx))*channels+2)**(kernel+(ky*kXSize)+kx);
				}}
				*(imgOut+((((y+yoffset)*imgW)+(x+xoffset))*channels))=accum[0];
				*(imgOut+((((y+yoffset)*imgW)+(x+xoffset))*channels)+1)=accum[1];
				*(imgOut+((((y+yoffset)*imgW)+(x+xoffset))*channels)+2)=accum[2];
				//out_i[0]=accum[0];
				//out_i[1]=accum[1];
				//out_i[2]=accum[2];
			}
		}
	}
	else
	{
		unsigned int c=0;
		double accum=0;
	
		for(c=0;c<channels;++c){
			for (y=ystart;y<yend;y++){
				for (x=xstart;x<xend;x++){
					accum=0.0;			
				//out_i=imgOut+((((y+yoffset)*imgW)+(x+xoffset))*channels);
				
					for (ky=0;ky<kYSize;ky++){
						for (kx=0;kx<kXSize;kx++){
							//in_i=imgIn+(((y+ky)*imgW)+(x+kx))*channels;
							//kernel_val=kernel[(ky*kXSize)+kx];
								//aja removed for loop
							//for (c=0;c<channels;c++){				
							//	accum[c]+=in_i[c]*kernel_val;
							//}
							accum+=*(imgIn+(((y+ky)*imgW)+(x+kx))*channels+c)**(kernel+(ky*kXSize)+kx);
						}
					}
					*(imgOut+((((y+yoffset)*imgW)+(x+xoffset))*channels)+c)=accum;
				//out_i[0]=accum[0];
				//out_i[1]=accum[1];
				//out_i[2]=accum[2];
				}
			
			}
		}		
	}
	return true;
}



bool Bluruc(	unsigned char *imgOut,const unsigned char *imgIn,
				unsigned int imgW,int imgH,unsigned int channels,
				unsigned int kXSize,unsigned int kYSize)
{
	int x,y,kx,ky;

	if(!imgOut||!imgIn)
		return false;

	int xend=imgW-kXSize;
	int yend=imgH-kYSize;

	if(xend<0||yend<0)
		return false;
	
	int xstart=0;
	int ystart=0;
	
		// aja, changed offset to xoffset and yoffset
	int xoffset=(int)((kXSize-1)/2.0);
	int yoffset=(int)((kYSize-1)/2.0);

	float kernelArea=kXSize*kYSize;
	
		//aja, added if else for speed up
	if(channels==1)
	{
		float sum;
		for (y=ystart;y<yend;y++){
			for (x=xstart;x<xend;x++){
				sum = 0;
				for (ky=0;ky<kYSize;ky++){
					for (kx=0;kx<kXSize;kx++){
									//aja, changed m_nSupport to imgW beloimgW
						sum+=*(imgIn+(((y+ky)*imgW)+(x+kx)));
				}}
			*(imgOut+(((y+yoffset)*imgW)+(x+xoffset)))=(sum/kernelArea)+0.5f;
			}
		}
	}
	else if(channels==3)
	{
		//const float *in_i;
		//float *out_i, kernel_val;
		double accum[3]={0,0,0};
	
		for (y=ystart;y<yend;y++){
			for (x=xstart;x<xend;x++){
				accum[0]=accum[1]=accum[2]=0.0;
				//fprintf(pErrorFile,"%d %d\n",x,y);
				//aja added index
				//out_i=imgOut+((((y+yoffset)*imgW)+(x+xoffset))*channels);
				
				for (ky=0;ky<kYSize;ky++){
					for (kx=0;kx<kXSize;kx++){
						//in_i=imgIn+(((y+ky)*imgW)+(x+kx))*channels;
						//kernel_val=kernel[(ky*kXSize)+kx];
						//
						//aja removed for loop
						//for (c=0;c<channels;c++){				
						//	accum[c]+=in_i[c]*kernel_val;
						//}
					
						//accum[0]+=in_i[0]*kernel_val;
						//accum[1]+=in_i[1]*kernel_val;
						//accum[2]+=in_i[2]*kernel_val;

				//the below seems the fastest option (i.e. faster than above indexing strategy)
				//presumably summat to do with compiler optimisation
					accum[0]+=*(imgIn+(((y+ky)*imgW)+(x+kx))*channels);
					accum[1]+=*(imgIn+(((y+ky)*imgW)+(x+kx))*channels+1);
					accum[2]+=*(imgIn+(((y+ky)*imgW)+(x+kx))*channels+2);
				}}
				*(imgOut+((((y+yoffset)*imgW)+(x+xoffset))*channels))=(accum[0]/kernelArea)+0.5f;
				*(imgOut+((((y+yoffset)*imgW)+(x+xoffset))*channels)+1)=(accum[1]/kernelArea)+0.5f;
				*(imgOut+((((y+yoffset)*imgW)+(x+xoffset))*channels)+2)=(accum[2]/kernelArea)+0.5f;
				//out_i[0]=accum[0];
				//out_i[1]=accum[1];
				//out_i[2]=accum[2];
			}
		}
	}
	else
	{
		unsigned int c=0;
		double accum=0;
	
		for(c=0;c<channels;++c){
			for (y=ystart;y<yend;y++){
				for (x=xstart;x<xend;x++){
					accum=0.0;			
				//out_i=imgOut+((((y+yoffset)*imgW)+(x+xoffset))*channels);
				
					for (ky=0;ky<kYSize;ky++){
						for (kx=0;kx<kXSize;kx++){
							//in_i=imgIn+(((y+ky)*imgW)+(x+kx))*channels;
							//kernel_val=kernel[(ky*kXSize)+kx];
								//aja removed for loop
							//for (c=0;c<channels;c++){				
							//	accum[c]+=in_i[c]*kernel_val;
							//}
							accum+=*(imgIn+(((y+ky)*imgW)+(x+kx))*channels+c);
						}
					}
					*(imgOut+((((y+yoffset)*imgW)+(x+xoffset))*channels)+c)=(accum/kernelArea)+0.5f;
				//out_i[0]=accum[0];
				//out_i[1]=accum[1];
				//out_i[2]=accum[2];
				}
			
			}
		}		
	}
	return true;
}


bool Bluri(	long *imgOut,const long *imgIn,
			unsigned int imgW,int imgH,unsigned int channels,
			unsigned int kXSize,unsigned int kYSize)
{
	int x,y,kx,ky;

	if(!imgOut||!imgIn)
		return false;

	int xend=imgW-kXSize;
	int yend=imgH-kYSize;

	if(xend<0||yend<0)
		return false;
	
	int xstart=0;
	int ystart=0;
	
		// aja, changed offset to xoffset and yoffset
	int xoffset=(int)((kXSize-1)/2.0);
	int yoffset=(int)((kYSize-1)/2.0);

	float kernelArea=kXSize*kYSize;
	
		//aja, added if else for speed up
	if(channels==1)
	{
		float sum;
		for (y=ystart;y<yend;y++){
			for (x=xstart;x<xend;x++){
				sum = 0;
				for (ky=0;ky<kYSize;ky++){
					for (kx=0;kx<kXSize;kx++){
									//aja, changed m_nSupport to imgW beloimgW
						sum+=*(imgIn+(((y+ky)*imgW)+(x+kx)));
				}}
			*(imgOut+(((y+yoffset)*imgW)+(x+xoffset)))=(sum/kernelArea)+0.5f;
			}
		}
	}
	else if(channels==3)
	{
		//const float *in_i;
		//float *out_i, kernel_val;
		double accum[3]={0,0,0};
	
		for (y=ystart;y<yend;y++){
			for (x=xstart;x<xend;x++){
				accum[0]=accum[1]=accum[2]=0.0;
				//fprintf(pErrorFile,"%d %d\n",x,y);
				//aja added index
				//out_i=imgOut+((((y+yoffset)*imgW)+(x+xoffset))*channels);
				
				for (ky=0;ky<kYSize;ky++){
					for (kx=0;kx<kXSize;kx++){
						//in_i=imgIn+(((y+ky)*imgW)+(x+kx))*channels;
						//kernel_val=kernel[(ky*kXSize)+kx];
						//
						//aja removed for loop
						//for (c=0;c<channels;c++){				
						//	accum[c]+=in_i[c]*kernel_val;
						//}
					
						//accum[0]+=in_i[0]*kernel_val;
						//accum[1]+=in_i[1]*kernel_val;
						//accum[2]+=in_i[2]*kernel_val;

				//the below seems the fastest option (i.e. faster than above indexing strategy)
				//presumably summat to do with compiler optimisation
					accum[0]+=*(imgIn+(((y+ky)*imgW)+(x+kx))*channels);
					accum[1]+=*(imgIn+(((y+ky)*imgW)+(x+kx))*channels+1);
					accum[2]+=*(imgIn+(((y+ky)*imgW)+(x+kx))*channels+2);
				}}
				*(imgOut+((((y+yoffset)*imgW)+(x+xoffset))*channels))=(accum[0]/kernelArea)+0.5f;
				*(imgOut+((((y+yoffset)*imgW)+(x+xoffset))*channels)+1)=(accum[1]/kernelArea)+0.5f;
				*(imgOut+((((y+yoffset)*imgW)+(x+xoffset))*channels)+2)=(accum[2]/kernelArea)+0.5f;
				//out_i[0]=accum[0];
				//out_i[1]=accum[1];
				//out_i[2]=accum[2];
			}
		}
	}
	else
	{
		unsigned int c=0;
		double accum=0;
	
		for(c=0;c<channels;++c){
			for (y=ystart;y<yend;y++){
				for (x=xstart;x<xend;x++){
					accum=0.0;			
				//out_i=imgOut+((((y+yoffset)*imgW)+(x+xoffset))*channels);
				
					for (ky=0;ky<kYSize;ky++){
						for (kx=0;kx<kXSize;kx++){
							//in_i=imgIn+(((y+ky)*imgW)+(x+kx))*channels;
							//kernel_val=kernel[(ky*kXSize)+kx];
								//aja removed for loop
							//for (c=0;c<channels;c++){				
							//	accum[c]+=in_i[c]*kernel_val;
							//}
							accum+=*(imgIn+(((y+ky)*imgW)+(x+kx))*channels+c);
						}
					}
					*(imgOut+((((y+yoffset)*imgW)+(x+xoffset))*channels)+c)=(accum/kernelArea)+0.5f;
				//out_i[0]=accum[0];
				//out_i[1]=accum[1];
				//out_i[2]=accum[2];
				}
			
			}
		}		
	}
	return true;
}


bool Blurf(	float *imgOut,const float *imgIn,
			unsigned int imgW,int imgH,unsigned int channels,
			unsigned int kXSize,unsigned int kYSize)
{
	int x,y,kx,ky;

	if(!imgOut||!imgIn)
		return false;

	int xend=imgW-kXSize;
	int yend=imgH-kYSize;

	if(xend<0||yend<0)
		return false;
	
	int xstart=0;
	int ystart=0;
	
		// aja, changed offset to xoffset and yoffset
	int xoffset=(int)((kXSize-1)/2.0);
	int yoffset=(int)((kYSize-1)/2.0);

	float kernelArea=kXSize*kYSize;
	
		//aja, added if else for speed up
	if(channels==1)
	{
		float sum;
		for (y=ystart;y<yend;y++){
			for (x=xstart;x<xend;x++){
				sum = 0;
				for (ky=0;ky<kYSize;ky++){
					for (kx=0;kx<kXSize;kx++){
									//aja, changed m_nSupport to imgW beloimgW
						sum+=*(imgIn+(((y+ky)*imgW)+(x+kx)));
				}}
			*(imgOut+(((y+yoffset)*imgW)+(x+xoffset)))=sum/kernelArea;
			}
		}
	}
	else if(channels==3)
	{
		//const float *in_i;
		//float *out_i, kernel_val;
		double accum[3]={0,0,0};
	
		for (y=ystart;y<yend;y++){
			for (x=xstart;x<xend;x++){
				accum[0]=accum[1]=accum[2]=0.0;
				//fprintf(pErrorFile,"%d %d\n",x,y);
				//aja added index
				//out_i=imgOut+((((y+yoffset)*imgW)+(x+xoffset))*channels);
				
				for (ky=0;ky<kYSize;ky++){
					for (kx=0;kx<kXSize;kx++){
						//in_i=imgIn+(((y+ky)*imgW)+(x+kx))*channels;
						//kernel_val=kernel[(ky*kXSize)+kx];
						//
						//aja removed for loop
						//for (c=0;c<channels;c++){				
						//	accum[c]+=in_i[c]*kernel_val;
						//}
					
						//accum[0]+=in_i[0]*kernel_val;
						//accum[1]+=in_i[1]*kernel_val;
						//accum[2]+=in_i[2]*kernel_val;

				//the below seems the fastest option (i.e. faster than above indexing strategy)
				//presumably summat to do with compiler optimisation
					accum[0]+=*(imgIn+(((y+ky)*imgW)+(x+kx))*channels);
					accum[1]+=*(imgIn+(((y+ky)*imgW)+(x+kx))*channels+1);
					accum[2]+=*(imgIn+(((y+ky)*imgW)+(x+kx))*channels+2);
				}}
				*(imgOut+((((y+yoffset)*imgW)+(x+xoffset))*channels))=accum[0]/kernelArea;
				*(imgOut+((((y+yoffset)*imgW)+(x+xoffset))*channels)+1)=accum[1]/kernelArea;
				*(imgOut+((((y+yoffset)*imgW)+(x+xoffset))*channels)+2)=accum[2]/kernelArea;
				//out_i[0]=accum[0];
				//out_i[1]=accum[1];
				//out_i[2]=accum[2];
			}
		}
	}
	else
	{
		unsigned int c=0;
		double accum=0;
	
		for(c=0;c<channels;++c){
			for (y=ystart;y<yend;y++){
				for (x=xstart;x<xend;x++){
					accum=0.0;			
				//out_i=imgOut+((((y+yoffset)*imgW)+(x+xoffset))*channels);
				
					for (ky=0;ky<kYSize;ky++){
						for (kx=0;kx<kXSize;kx++){
							//in_i=imgIn+(((y+ky)*imgW)+(x+kx))*channels;
							//kernel_val=kernel[(ky*kXSize)+kx];
								//aja removed for loop
							//for (c=0;c<channels;c++){				
							//	accum[c]+=in_i[c]*kernel_val;
							//}
							accum+=*(imgIn+(((y+ky)*imgW)+(x+kx))*channels+c);
						}
					}
					*(imgOut+((((y+yoffset)*imgW)+(x+xoffset))*channels)+c)=accum/kernelArea;
				//out_i[0]=accum[0];
				//out_i[1]=accum[1];
				//out_i[2]=accum[2];
				}
			
			}
		}		
	}
	return true;
}


////////////////////////////////////////////////////////////////////////////////
// DOG Filter creation code, C-style

bool CreateDogFilter(	float *dog,
						unsigned int xsize, unsigned int ysize,
						unsigned int xorder, unsigned int yorder,
						double sigma, double angle,
						bool dirSwap)
{
	if(!dog)
	{
		fprintf(pErrorFile,"CreateDogFilter(), arg dog=NULL");
		return false;
	}

	if(xsize%2==0||ysize%2==0)
	{
		fprintf(pErrorFile,"CreateDog(), Filter side arg even");
		return false;
	}

	int cx=(int)((xsize-1)/2.0);
	int cy=(int)((ysize-1)/2.0);

	int kx,ky;//Kernel Tap Values
	double kxc,kyc;//Centralised Values
	double rkxc,rkyc;//Rotated Centralised values
	double dummy=0.0;
	double angle_rads=3.1415926*angle/180.0;
	//double sum=0.0;
	//Create Rotated Kernel

	if(dirSwap){
		for (ky=0;ky<ysize;ky++){
			kyc=(double)(cy-ky);
			for (kx=0;kx<xsize;kx++){
				kxc=(double)(cx-kx);
				rkxc=kxc*cos(angle_rads)+kyc*sin(angle_rads);
				rkyc=kyc*cos(angle_rads)-kxc*sin(angle_rads);
				dummy=(Gaussian(rkyc,sigma)*HermiteFunction(rkyc,sigma,yorder));
				*(dog+ky*xsize+kx)=
					(float)(dummy*Gaussian(rkxc,sigma)*HermiteFunction(rkxc,sigma,xorder));
			//sum+=*(dog+ky*xsize+kx);	
			}
		}
	}
	else{
			for (ky=0;ky<ysize;ky++){
			kyc=(double)(ky-cy);
			for (kx=0;kx<xsize;kx++){
				kxc=(double)(kx-cx);
				rkxc=kxc*cos(angle_rads)+kyc*sin(angle_rads);
				rkyc=kyc*cos(angle_rads)-kxc*sin(angle_rads);
				dummy=(Gaussian(rkyc,sigma)*HermiteFunction(rkyc,sigma,yorder));
				*(dog+ky*xsize+kx)=
					(float)(dummy*Gaussian(rkxc,sigma)*HermiteFunction(rkxc,sigma,xorder));
			//sum+=*(dog+ky*xsize+kx);	
			}
		}
	}
	
	//fprintf(pErrorFile,"Normalised Integral of Gaussian %d %d = %f\n",xorder,yorder,sum);

	return true;
}


bool CreateDogFilterSep(	float *dogx,float *dogy,
							unsigned int xsize, unsigned int ysize,
							unsigned int xorder, unsigned int yorder,
							double sigma,bool dirSwap)
{
	if(!dogx||!dogy)
	{
		fprintf(pErrorFile,"CreateDogFilterSep(), arg dogx or dogy=NULL");
		return false;
	}
	
	if(xsize%2==0||ysize%2==0)
	{
		fprintf(pErrorFile,"CreateDogFilterSep(), Filter side arg even");
		return false;
	}

	int k;double kc;
	int cx=(int)((xsize-1)/2.0);//Centre Positions
	int cy=(int)((ysize-1)/2.0);

	if(dirSwap)
	{
		//double sumx=0.0f,sumy=0.0f;
		//Generate Kernels
		
		for (k=0;k<xsize;k++){
			kc=(double)(cx-k);
			dogx[k]=(float)(Gaussian(kc,sigma)*HermiteFunction(kc,sigma,xorder));
			//sumx+=*(dogx+k);
		}
		
		for (k=0;k<ysize;k++){
			kc=(double)(cy-k);
			dogy[k]=(float)(Gaussian(kc,sigma)*HermiteFunction(kc,sigma,yorder));
			//sumy+=*(dogy+k);
		}
	}
	else
	{
		for (k=0;k<xsize;k++){
			kc=(double)(k-cx);
			dogx[k]=(float)(Gaussian(kc,sigma)*HermiteFunction(kc,sigma,xorder));
			//sumx+=*(dogx+k);
		}
		
		for (k=0;k<ysize;k++){
			kc=(double)(k-cy);
			dogy[k]=(float)(Gaussian(kc,sigma)*HermiteFunction(kc,sigma,yorder));
			//sumy+=*(dogy+k);
		}
	}

	//fprintf(pErrorFile,"Gaussian (dx%d,dy%d) XInt=%3.8f  YInt=%3.8f\n",xorder,yorder,sumx,sumy);

	return true;
}


bool CreateDolgFilterT(	float *tKernel,
						unsigned int tsize, unsigned int order, double alpha, 
						double tau,float multiplier)
{
	if (alpha<0){fprintf(pErrorFile,"CreateDolgFilterT, -ve alpha (%f)\n",alpha);return false;}
	if (tau<0){fprintf(pErrorFile,"CreateDolgFilterT, -ve Tau (%f)\n",tau);return false;}
	if (order>2){fprintf(pErrorFile,"CreateDolgFilterT, Order >2 (%d)\n",order);return false;}
	if (tsize<alpha){fprintf(pErrorFile,"CreateDolgFilterT, Invalid tsize<alpha\n");return false;}

	double sum=0.0;
	int t;

	for (t=0;t<tsize;t++){
		((float*)tKernel)[t]=(float)Dolg(t,order,alpha,tau);
		((float*)tKernel)[t]*=multiplier;
		//fprintf(stdout,"DOLG %d (alpha=%3.2f,tau=%3.3f) Tap %d = %3.4f\n",order,alpha,tau,t,*(m_TKernel+t)); 
		sum+=((float*)tKernel)[t];
	}

	for (t=0;t<tsize;t++){
		if (order!=0){((float*)tKernel)[t]-=(float)(sum/(double)tsize);}
	}

	//fprintf(pErrorFile,"DoLG %d Sum = %f\n",order,sum);

	return true;
}


////////////////////////////////////////////////////////////////////////////////
//Math functions for creating filters

double Dolg(int pos, int order, double alpha, double tau)
{
	double lvar;
	double lg[3];
	double sf=0.0;

	if (pos<0.0){fprintf(pErrorFile,"Dolg:Cannot have Negative Temporal Pos\n");return 0.0;}
	if (order<0 || order >2){fprintf(pErrorFile,"Dolg:Invalid Order (%d)\n",order);return 0.0;}
	
	sf = sqrt(3.14159265358979) * tau * alpha * exp(tau*tau/4.0);		
	lvar = (double) pos;
	if (lvar<0.00001){lvar=0.00001;}

	// log gaussian
		lg[0]=exp(-(log(lvar/alpha)/tau)*(log(lvar/alpha)/tau))/sf;
		
	// 1st diff

		lg[1] = -2.0 * ( (log(lvar/alpha))/(tau*tau*lvar) ) * lg[0];

	//2nd Diff
		lg[2]= lg[1]*(-2.0*(log(lvar/alpha)/(tau*tau*lvar)))- 
			(lg[0]*2.0*(1.0-log(lvar/alpha)))/(tau*tau*lvar*lvar);

	//fprintf(pErrorFile,"Dolg %d tap %d = %f\n",order,pos,lg[order]);
	return lg[order];
}

double Gaussian(double pos,double sig){
	double t=(sig*sig)/2.0;
	double norm=sqrt(4.0*3.1415926*t);
	double expval = (pos*pos)/(4*t);
	double filtval = exp(-expval)/norm;
	return (filtval);
}
//Used to Generate DOG polynomials
double HermiteFunction(double x, double sig, int ord)
{
	if (ord==0){return (1.0);}
	int m,top,pass;
	double ans,p,p2,a,b;
	double s=sig*sig/2.0;
	double dord=(double)ord;
	top=(ord/2) - ( (ord%2)/2);
	ans=0.0;

	for(m=0;m<=top;m++) {
		p=dord-2*(double)m;
		p2=dord-(double)m;
		pass=(ord-2*m);
		a=(2.0*x);
		b=(4.0*s);
		ans= ans+ pow(-1.0,(double)m)*((Herpow(a,p)/Herpow(b,p2) )/(Fac(m)*Fac(pass)));
	}

	ans *= Fac(ord);

	return(ans);
}
//Compute power in robust manner
double Herpow(double a1, double b1)
{
	double z;
	if(a1==0.0 && b1==0){z=1.0;}
	else {z=pow(a1,b1);}
	return(z);
}
//Binomial Number generation
int Binomial(int n,int r)
{
	float dnmr;
	float quo;
	dnmr=(float)(Fac(r)*Fac(n-r));
	quo=((float)Fac(n))/dnmr;
	return((int)quo);
}

int parity(int n)
{
	int r=2*(n%2);
	return (1-r);
}

/*
int CSteerBasis::binomial(int N,int i)
{
	return( fac(N)/(fac(i)*fac(N-i)) );
}


//Binomial
static int binomial(int r,int N)
{
	int b = fac(N)/(fac(r)*fac(N-r));
	return b;
}
*/

//Misc mathematical support functions (global)

/*

int CSteerBasis::fac(int x)
{
	if (x==2) return(2);
	if (x==0) return(1);
	if (x==1) return(1);
	return (x*fac(x-1));
}


//Factorial
static int fac(int x)
{
	int i, h = 1;
	if (x==0) return(1);
	if (x==1) return(1);
	if (x==2) return(2);
	if (x==3) return(6);
	if (x==4) return(24);
	
	else {
		x++;
		for (i=1; i < x ;i++) h *= i;
		return(h);
	}
}
*/

//Factorial
int Fac(int x)
{
	int ans=1, h;
	if (x==0) {return(1);} 
	if (x==1) {return(1);}
	if (x==2) {return(2);}
	else {
		for (h=x; h>1;h--) ans = ans * h;
		return(ans);
	}
}




// C-style convolution code

const int XDIR = 0;
const int YDIR = 1;

static bool ConvSepi(	long *imgOut,const long *imgIn,
						unsigned int imgW,int imgH,unsigned int channels,
						const long *xKernel,const long *yKernel,
						unsigned int kXSize,unsigned int kYSize,
						int dirn);
static bool ConvSepf(	float *imgOut,const float *imgIn,
						unsigned int imgW,int imgH,unsigned int channels,
						const float *xKernel,const float *yKernel,
						unsigned int kXSize,unsigned int kYSize,
						int dirn);

bool ConvSepi(long *imgOut,long *imgMid,const long *imgIn,
			  unsigned int imgW,int imgH,unsigned int channels,
			  const long *xKernel,const long *yKernel,
			  unsigned int kXSize,unsigned int kYSize)
{
	if(!imgOut||!imgMid||!imgIn){
		fprintf(pErrorFile,"GenericImage.cpp, ConvSepi(), Img arg(s)=NULL\n");
		return false;
	}

	if(imgOut==imgMid||imgOut==imgIn||imgMid==imgIn){
		fprintf(pErrorFile,"GenericImage.cpp, ConvSepi(), One or more image arg addresses equal\n");
		return false;
	}

		//check that this deals with 0?
	if(kXSize%2==0||kYSize%2==0)
	{
		fprintf(pErrorFile,
"GenericImage.cpp, ConvSepi(), One or both kernel dimensions even (%d*%d)\n",
				kXSize,kYSize);
		return false;
	}

	if(kXSize>imgW||kYSize>imgH)
	{
		fprintf(pErrorFile,
"GenericImage.cpp, ConvSepi(), Kernel (%d*%d) too large for image (%d*%d)\n",
				kXSize,kYSize,imgW,imgH);
		return false;
	}

	if(!ConvSepi(imgMid,imgIn,imgW,imgH,channels,xKernel,yKernel,kXSize,kYSize,XDIR))
		return false;
	
	return ConvSepi(imgOut,imgMid,imgW,imgH,channels,xKernel,yKernel,kXSize,kYSize,YDIR);
}


bool ConvSepf(float *imgOut,float *imgMid,const float *imgIn,
			  unsigned int imgW,int imgH,unsigned int channels,
			  const float *xKernel,const float *yKernel,
			  unsigned int kXSize,unsigned int kYSize)
{
	if(!imgOut||!imgMid||!imgIn){
		fprintf(pErrorFile,"GenericImage.cpp, ConvSepf(), Img arg(s)=NULL\n");
		return false;
	}

	if(imgOut==imgMid||imgOut==imgIn||imgMid==imgIn){
		fprintf(pErrorFile,"GenericImage.cpp, ConvSepf(), One or more image arg addresses equal\n");
		return false;
	}

		//check that this deals with 0?
	if(kXSize%2==0||kYSize%2==0)
	{
		fprintf(pErrorFile,
"GenericImage.cpp, ConvSepf(), One or both kernel dimensions even (%d*%d)\n",
				kXSize,kYSize);
		return false;
	}

	if(kXSize>imgW||kYSize>imgH)
	{
		fprintf(pErrorFile,
"GenericImage.cpp, ConvSepf(), Kernel (%d*%d) too large for image (%d*%d)\n",
				kXSize,kYSize,imgW,imgH);
		return false;
	}

	if(!ConvSepf(imgMid,imgIn,imgW,imgH,channels,xKernel,yKernel,kXSize,kYSize,XDIR))
		return false;
	
	return ConvSepf(imgOut,imgMid,imgW,imgH,channels,xKernel,yKernel,kXSize,kYSize,YDIR);
}


static bool ConvSepi(	long *imgOut,const long *imgIn,
						unsigned int imgW,int imgH,unsigned int channels,
						const long *xKernel,const long *yKernel,
						unsigned int kXSize,unsigned int kYSize,
						int dirn)
{
	int x,y,c,kx;
	
	if(!imgOut||!imgIn||!xKernel||!yKernel)
		return false;

	if (dirn!=XDIR && dirn!=YDIR)
		return false;

	int xend=imgW-kXSize;
	int yend=imgH-kYSize;

	if(xend<0||yend<0)
		return false;
	
	int xstart=0;
	int ystart=0;

		// aja, changed offset to xoffset and yoffset
	int xoffset=(int)((kXSize-1)/2.0);
	int yoffset=(int)((kYSize-1)/2.0);
	
	int accum=0.0f;
	long y_pos;

	if( channels == 1 )
	{
		if (dirn==0){	//X-CONV
			for (y=0;y<imgH;y++){
				y_pos=y*imgW;
				for (x=xstart;x<xend;x++){
					accum=0.0;
					for (kx=0;kx<kXSize;kx++){
						accum+=(*(imgIn+((y_pos)+(x+kx)))**(xKernel+kx));
					}
					*(imgOut+((y_pos)+(x+xoffset)))=accum;
				}
			}
		}
		
		if (dirn==1){	//Y-CONV
			for (y=ystart;y<yend;y++){
				for (x=0;x<imgW;x++){
					accum=0.0f;
					for (kx=0;kx<kYSize;kx++){
						accum +=(*(imgIn+(((y+kx)*imgW)+x))**(yKernel+kx));
					}				
				*(imgOut+(((y+yoffset)*imgW)+x))=accum;
				}
			}
		}
	}
	else
	{
		if (dirn==0){	//X-CONV
			for (c=0;c<channels;c++){
				for (y=0;y<imgH;y++){
					y_pos=y*imgW;
					for (x=xstart;x<xend;x++){
						accum=0.0;
						for (kx=0;kx<kXSize;kx++){
							accum+=(*(imgIn+((y_pos)+(x+kx))*channels+c)**(xKernel+kx));
						}
						*(imgOut+((y_pos)+(x+xoffset))*channels+c)=accum;
					}
				}
			}
		}
		
		if (dirn==1){	//Y-CONV
			for (c=0;c<channels;c++){
				for (y=ystart;y<yend;y++){
					for (x=0;x<imgW;x++){
						accum=0.0f;
						for (kx=0;kx<kYSize;kx++){
							accum += (*(imgIn+(((y+kx)*imgW)+x)*channels+c)**(yKernel+kx));
						}				
					*(imgOut+(((y+yoffset)*imgW)+x)*channels+c)=accum;
					}
				}
			}
		}
	}

	return true;
}


////////////////////////////////////////////////////////////////////////////
// ConvSep
// im:		Input Floats
// imout:	Output Floats
// dirn:	Direction of convolution... x or y.  (XDIR or YDIR)
// channels:	Number of channels in image
//
// Performs convolution with Floating point kernal to produce floating point output.  Output image
// must be reserved before calling this function.
static bool ConvSepf(	float *imgOut,const float *imgIn,
						unsigned int imgW,int imgH,unsigned int channels,
						const float *xKernel,const float *yKernel,
						unsigned int kXSize,unsigned int kYSize,
						int dirn)
{
	int x,y,c,kx;
	
	if(!imgOut||!imgIn||!xKernel||!yKernel)
		return false;

	if (dirn!=XDIR && dirn!=YDIR)
		return false;

	int xend=imgW-kXSize;
	int yend=imgH-kYSize;

	if(xend<0||yend<0)
		return false;
	
	int xstart=0;
	int ystart=0;

		// aja, changed offset to xoffset and yoffset
	int xoffset=(int)((kXSize-1)/2.0);
	int yoffset=(int)((kYSize-1)/2.0);
	
	float accum=0.0f;
	long y_pos;

	if( channels == 1 )
	{
		if (dirn==0){	//X-CONV
			for (y=0;y<imgH;y++){
				y_pos=y*imgW;
				for (x=xstart;x<xend;x++){
					accum=0.0;
					for (kx=0;kx<kXSize;kx++){
						accum+=(*(imgIn+((y_pos)+(x+kx)))**(xKernel+kx));
					}
					*(imgOut+((y_pos)+(x+xoffset)))=accum;
				}
			}
		}
		
		if (dirn==1){	//Y-CONV
			for (y=ystart;y<yend;y++){
				for (x=0;x<imgW;x++){
					accum=0.0f;
					for (kx=0;kx<kYSize;kx++){
						accum +=(*(imgIn+(((y+kx)*imgW)+x))**(yKernel+kx));
					}				
				*(imgOut+(((y+yoffset)*imgW)+x))=accum;
				}
			}
		}
	}
	else
	{
		if (dirn==0){	//X-CONV
			for (c=0;c<channels;c++){
				for (y=0;y<imgH;y++){
					y_pos=y*imgW;
					for (x=xstart;x<xend;x++){
						accum=0.0;
						for (kx=0;kx<kXSize;kx++){
							accum+=(*(imgIn+((y_pos)+(x+kx))*channels+c)**(xKernel+kx));
						}
						*(imgOut+((y_pos)+(x+xoffset))*channels+c)=accum;
					}
				}
			}
		}
		
		if (dirn==1){	//Y-CONV
			for (c=0;c<channels;c++){
				for (y=ystart;y<yend;y++){
					for (x=0;x<imgW;x++){
						accum=0.0f;
						for (kx=0;kx<kYSize;kx++){
							accum += (*(imgIn+(((y+kx)*imgW)+x)*channels+c)**(yKernel+kx));
						}				
					*(imgOut+(((y+yoffset)*imgW)+x)*channels+c)=accum;
					}
				}
			}
		}
	}

	return true;
}


////////////////////////////////////////////////////////////////////////////
// VISION LIBRARY FILTER IMPLEMENTS ABSTRACT FILTER CLASS___________________
////////////////////////////////////////////////////////////////////////////
	// Independent filter class (i.e. not dependent on 3rd party software libraries)
	// Code specifies creation, destruction and copying of (1) derivative of gaussian
	// filters (2d and separable) for spatial filtering and (2) derivative of log
	// gaussian for temporal filtering
	// NOTE only IMG_LONG and IMG_FLOAT data types supported


_Filter::_Filter()
{
	m_Kernel=NULL;
	m_XKernel=NULL;
	m_YKernel=NULL;
	m_TKernel=NULL;	
	m_pInput=NULL;
	m_pOutput=NULL;
	m_pSepOutput=NULL;

	memset(&m_Hdr,0,sizeof(_FilterHdr));
	m_Hdr.isSep=false;
}


_Filter::_Filter(const _Filter &f)
{
	m_Kernel=NULL;
	m_XKernel=NULL;
	m_YKernel=NULL;
	m_TKernel=NULL;
	m_pInput=NULL;
	m_pOutput=NULL;
	m_pSepOutput=NULL;

	Copy(f);
}


bool _Filter::Create(	unsigned int xsize,unsigned int ysize,
						short int dataType,const void *filter)
{
	if(!CheckDataType(dataType)){
		fprintf(pErrorFile,"_Filter::Create(), Invalid data type (%d)\n",dataType);
		xyDestroy();
		return false;
	}

	unsigned int depth=0;

	if(!CalcDepth(depth,dataType)){
		fprintf(pErrorFile,"_Filter::Create(), Invalid data type\n");
		xyDestroy();
		return false;
	}

	m_Hdr.dataType=dataType;

	if(xyIsValid())
		xyDestroy();
		
	m_Hdr.w=xsize;
	m_Hdr.h=ysize;
	m_Hdr.depth=depth;
	unsigned int sz=xsize*ysize*depth;

	try{
		m_Kernel=new char[sz];
	}
	catch( bad_alloc xa )
	{
		fprintf(pErrorFile,"Filter::Create(), Error allocating memory"); 
		xyDestroy();
		return false;
	}

	if(filter){
		memcpy(m_Kernel,filter,sz);
	}
	else{
		fprintf(pErrorFile,"_Img::Create(), Arg filter=NULL\n");
		memset(m_Kernel,0,sz);
		return false;
	}
	return true;
}


bool _Filter::CreateSep(	unsigned int xsize,unsigned int ysize,
							short int dataType,const void *xFilter,const void *yFilter)
{
	if(!CheckDataType(dataType)){
		fprintf(pErrorFile,"_Filter::CreateSep(), Invalid data type (%d)\n",dataType);
		xyDestroy();
		return false;
	}

	unsigned int depth=0;

	if(!CalcDepth(depth,dataType)){
		fprintf(pErrorFile,"_Filter::CreateSep(), Invalid data type\n");
		xyDestroy();
		return false;
	}

	if(xyIsValid())
		xyDestroy();
		
	m_Hdr.dataType=dataType;
	m_Hdr.w=xsize;
	m_Hdr.h=ysize;
	m_Hdr.depth=depth;
	unsigned int xsz=xsize*depth;
	unsigned int ysz=ysize*depth;

	try{
		if(xFilter){
			m_XKernel=new char[xsz];
			memcpy(m_XKernel,xFilter,xsz);
		}
		else
			m_Hdr.w=0;

		if(yFilter){
			m_YKernel=new char[ysz];
			memcpy(m_YKernel,yFilter,xsz);
		}
		else
			m_Hdr.h=0;
	}
	catch( bad_alloc xa )
	{
		fprintf(pErrorFile,"Filter::CreateSep(), Error allocating memory"); 
		xyDestroy();
		return false;
	}

	return true;
}


bool _Filter::CreateT(unsigned int tsize,short int dataType,const void *tFilter)
{
	if(!CheckDataType(dataType)){
		fprintf(pErrorFile,"_Filter::CreateT(), Invalid data type (%d)\n",dataType);
		tDestroy();
		return false;
	}

	unsigned int depth=0;

	if(!CalcDepth(depth,dataType)){
		fprintf(pErrorFile,"_Filter::CreateT(), Invalid data type\n");
		tDestroy();
		return false;
	}

	if(tIsValid())
		tDestroy();
		
	m_Hdr.tSize=tsize;
	m_Hdr.tDepth=depth;
	m_Hdr.tDataType=dataType;
	unsigned int sz=tsize*depth;

	try{
		m_TKernel=new char[sz];
	}
	catch( bad_alloc xa )
	{
		fprintf(pErrorFile,"Filter::CreateT(), Error allocating memory"); 
		tDestroy();
		return false;
	}

	if(tFilter){
		memcpy(m_TKernel,tFilter,sz);
	}
	else{
		fprintf(pErrorFile,"_Img::CreateT(), Arg filter=NULL\n");
		memset(m_TKernel,0,sz);
		return false;
	}
	return true;
}


bool _Filter::CheckDataType(short int dataType)
{
	if( dataType!=IMG_LONG
		&&
		dataType!=IMG_FLOAT
		){
		return false;
	}

	return true;
}

bool _Filter::CalcDepth(unsigned int &depth,short int dataType)
{
	switch(dataType){
	
	case IMG_LONG:	{
						depth=sizeof(long);
					} break;
	case IMG_FLOAT:	{
						depth=sizeof(float);
					} break;
	default:		{
						return false;
					}
	}
	return true;
}


//CreateDogFilter
//Creates a 2D Kernel of a normalised Dog filter at an arbitrary angle.
//Rotation is clockwise in degrees
bool _Filter::CreateDog(	unsigned int xsize, unsigned int ysize,
							 unsigned int xorder, unsigned int yorder,
							 double sigma, double angle)
{
		//Delete Old Kernel
	if (m_YKernel!=NULL){xyDestroy();}
	if (m_XKernel!=NULL){xyDestroy();}
	if (m_Kernel!=NULL){xyDestroy();}

		//need to check whether has been resized

		//Allocate Kernel
	float *dog=NULL;
	try{
		dog=new float[xsize*ysize];
	}
	catch( bad_alloc xa )
	{
		fprintf(pErrorFile,"Filter::CreateDog(), Error allocating memory"); 
		return false;
	}
	
	if(!CreateDogFilter(dog,xsize,ysize,xorder,yorder,sigma,angle,false))
	{
		fprintf(pErrorFile,"_Filter::CreateDog(), Error creating filters\n");
		return false;
	}
	
	m_Hdr.dataType=IMG_FLOAT;
	m_Hdr.isSep=false;
	m_Hdr.w=xsize; 
	m_Hdr.h=ysize;
	CalcDepth(m_Hdr.depth,IMG_FLOAT);
	
	m_Kernel=dog;
	
	return true;
}


/////////////////////////////////
//Creates a separable Dog filter
bool _Filter::CreateDogSep(	unsigned int xsize, unsigned int ysize,
							unsigned int xorder, unsigned int yorder,
							double sigma)
{
	//Delete Old Kernel
	if (m_YKernel){xyDestroy();}
	if (m_XKernel){xyDestroy();}
	if (m_Kernel){xyDestroy();}

	float *dogx, *dogy;
	try{
		dogx=new float[xsize];
		dogy=new float[ysize];
	}
	catch( bad_alloc xa )
	{
		fprintf(pErrorFile,"_Filter::CreateDogSep(), Error allocating memory"); 
		return false;
	}
	
	if(!CreateDogFilterSep(dogx,dogy,xsize,ysize,xorder,yorder,sigma,false))
	{
		fprintf(pErrorFile,"_Filter::CreateDogSep(), Error creating filters\n");
		return false;
	}

	m_Hdr.dataType=IMG_FLOAT;
	m_Hdr.isSep=true;
	m_Hdr.w=xsize; 
	m_Hdr.h=ysize;
	CalcDepth(m_Hdr.depth,IMG_FLOAT);

	m_XKernel=dogx;
	m_YKernel=dogy;
	
	return true;
}








/*
/////////////////////////////////
//Creates a separable Dog filter
bool _Filter::CreateDogSep(	unsigned int xsize, unsigned int ysize,
									unsigned int xorder, unsigned int yorder,
									double sigma)
{
	//Delete Old Kernel
	if (m_YKernel!=NULL){xyDestroy();}
	if (m_XKernel!=NULL){xyDestroy();}
	if (m_Kernel!=NULL){xyDestroy();}

	//Sanity Checks
	if( xsize % 2 == 0 || ysize % 2 == 0 )
	{
		fprintf(pErrorFile,"Filter::CreateDogSep(), Filter side arg even");
		return false;
	}

	float *dogx, *dogy;
	try{
		dogx=new float[xsize];
		dogy=new float[ysize];
	}
	catch( bad_alloc xa )
	{
		fprintf(pErrorFile,"Filter::CreateDogSep(), Error allocating memory"); 
		return false;
	}
	//double sumx=0.0f,sumy=0.0f;

	//Generate Kernels
	int k;double kc;
	int cx=(int)((xsize-1)/2.0);//Centre Position
	for (k=0;k<xsize;k++){
		kc=(double)(cx-k);
		*(dogx+k)=(float)(Gaussian(kc,sigma)*HermiteFunction(kc,sigma,xorder));
		//sumx+=*(dogx+k);
	}
	int cy=(int)((ysize-1)/2.0);//Centre Position
	for (k=0;k<ysize;k++){
		kc=(double)(cy-k);
		*(dogy+k)=(float)(Gaussian(kc,sigma)*HermiteFunction(kc,sigma,yorder));
		//sumy+=*(dogy+k);
	}

	//fprintf(pErrorFile,"Gaussian (dx%d,dy%d) XInt=%3.8f  YInt=%3.8f\n",xorder,yorder,sumx,sumy);

	m_Hdr.dataType=IMG_FLOAT;
	m_Hdr.isSep=true;
	m_Hdr.w=xsize; 
	m_Hdr.h=ysize;
	CalcDepth(m_Hdr.depth,IMG_FLOAT);

	m_XKernel = dogx;
	m_YKernel = dogy;
	
	return true;
}

*/

bool _Filter::CreateDolgT(			unsigned int tsize, unsigned int order, double alpha, 
									double tau,float multiplier)
{
	//(Re)Allocate Temporal Kernel
	if (m_TKernel)
		tDestroy();

	try{
		m_TKernel=new float[tsize];
		m_Hdr.tSize=tsize;
	}
	catch(bad_alloc xa)
	{
		fprintf(pErrorFile,"_Filter::CreateDolgT(), Cannot Allocate TKernel\n");
		tDestroy();
		return false;
	}

	if(!CreateDolgFilterT((float*)m_TKernel,tsize,order,alpha,tau,multiplier))
	{
		fprintf(pErrorFile,"_Filter::CreateDolgT(), Error creating filter\n");
		return false;
	}
	
	m_Hdr.tDataType=IMG_FLOAT;
	CalcDepth(m_Hdr.tDepth,IMG_FLOAT);
	
	return true;
}
	
bool _Filter::Copy(const AbstractFilter &src)
{
	if (this==&src)
		return false;

	bool status=true;

	if(src.xyIsValid())
	{
		if(src.IsSep())
			status=CreateSep(src.Width(),src.Height(),src.DataType(),
											src.XFilter(),src.YFilter());
		else
			status=Create(src.Width(),src.Height(),src.DataType(),src.Filter());
	}
	else
		xyDestroy();

	if(status==false){
		fprintf(pErrorFile,"_Filter::Copy(), Error copying spatial filter\n");
		return false;
	}
	
	if(src.tIsValid())
		status=CreateT(src.tSize(),src.tDataType(),src.TFilter());
	else
		tDestroy();

	if(status==false){
		fprintf(pErrorFile,"_Filter::Copy(), Error copying temporal filter\n");
		return false;
	}
	
	if(!xyIsValid()&&!tIsValid())
		return false;

	return true;
}

AbstractFilter* _Filter::New() const
{
	_Filter *pReturn;

	try{
		pReturn=new _Filter;
	}
	catch(bad_alloc xa)
	{
		fprintf(pErrorFile,"_Filter::New(), Error allocating memory\n");
		pReturn=NULL;
	}

	return pReturn;
}

AbstractFilter* _Filter::NewCopy() const
{
	_Filter *pReturn;

	try{
		pReturn=new _Filter(*this);
	}
	catch(bad_alloc xa)
	{
		fprintf(pErrorFile,"_Filter::NewCopy(), Error allocating memory\n");
		pReturn=NULL;
	}

	return pReturn;
}

AbstractFilter* _Filter::NewHeader() const
{
	_Filter *pReturn;

	try{
		pReturn=new _Filter;
		memcpy(&(pReturn->m_Hdr),&m_Hdr,sizeof(_FilterHdr));
	}
	catch(bad_alloc xa)
	{
		fprintf(pErrorFile,"_Filter::NewHeader(), Error allocating memory\n");
		pReturn=NULL;
	}
	return pReturn;
}


AbstractFilter& _Filter::operator=(const AbstractFilter &f)
{
	if(this!=&f)
	{
		Copy(f);
	}
	return *this;
}
	
_Filter::~_Filter()
{
	Destroy();
}

//Clean Up//
bool _Filter::Destroy()
{
	if( m_Kernel )
		delete [] m_Kernel;
	if( m_XKernel )
		delete [] m_XKernel;
	if( m_YKernel )
		delete [] m_YKernel;
	if( m_TKernel )
		delete [] m_TKernel;										
	if( m_pSepOutput )
		delete [] m_pSepOutput;
	if( m_pInput )
		delete [] m_pInput;
	if( m_pOutput )
		delete [] m_pOutput;
	
	//Set Pointers back to NULL
	m_Kernel=NULL;
	m_XKernel=NULL;
	m_YKernel=NULL;
	m_TKernel=NULL;
	m_pInput=NULL;
	m_pOutput=NULL;
	m_pSepOutput=NULL;

	memset(&m_Hdr,0,sizeof(_FilterHdr));
	m_Hdr.isSep=false;

	return true;
}

bool _Filter::xyDestroy()
{
	if( m_Kernel )
		delete [] m_Kernel;
	if( m_XKernel )
		delete [] m_XKernel;
	if( m_YKernel )
		delete [] m_YKernel;								
	if( m_pSepOutput )
		delete [] m_pSepOutput;
	
	
	//Set Pointers back to NULL
	m_Kernel=NULL;
	m_XKernel=NULL;
	m_YKernel=NULL;
	m_pSepOutput=NULL;

	m_Hdr.w=m_Hdr.h=0;
	m_Hdr.dataType=IMG_NULL_DATATYPE;
	m_Hdr.depth=0;
	m_Hdr.isSep=false;

	return true;
}

bool _Filter::tDestroy()
{
	if( m_TKernel )
		delete [] m_TKernel;										
	
	m_TKernel=NULL;
	m_Hdr.tSize=0;
	m_Hdr.tDepth=0;
	m_Hdr.tDataType=IMG_NULL_DATATYPE;

	return true;
}


inline unsigned int _Filter::Width() const
{
	return m_Hdr.w;	
}

inline unsigned int _Filter::Height() const
{
	return m_Hdr.h;
}

inline unsigned int _Filter::tSize() const
{
	return m_Hdr.tSize;
}

inline const void* _Filter::Filter() const
{
	return 	m_Kernel;
}

inline const void* _Filter::XFilter() const
{
	return m_XKernel;
}

inline const void* _Filter::YFilter() const
{
	return m_YKernel;
}

inline const void* _Filter::TFilter() const
{
	return m_TKernel;
}
	
inline bool _Filter::IsSep() const
{
	return m_Hdr.isSep;
}

inline short int _Filter::DataType() const
{
	return m_Hdr.dataType;
}

inline short int _Filter::tDataType() const
{
	return m_Hdr.tDataType;
}

inline bool _Filter::xyIsValid() const
{
	if(IsSep()){
		if(!m_XKernel||!m_YKernel)
		return false;
	}
	else
		if(!m_Kernel)
			return false;
	
	return true;
}

inline bool _Filter::tIsValid() const
{
	if(!m_TKernel)
		return false;

	return true;
}

inline const char* _Filter::ClassID() const
{
	return typeid(this).name();
}

inline bool _Filter::SameClass(const AbstractFilter &f) const
{
	if(typeid(this)!=typeid(&f))
		return false;
	
	return true;
}


#ifdef _UCLVisionResearchLabLib_H_IPL_INCLUDED_
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// IPL Filter
////////////////////////////////////////////////////////////////////////////

/*
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
*/
//
//#pragma message (CIPLF_VERSION_S)
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CIplFilter::CIplFilter()
{
	m_Kernel=NULL;
	m_XKernel=NULL;
	m_YKernel=NULL;
	m_TKernel=NULL;	

	memset(&m_Hdr,0,sizeof(CIplFilterHdr));
	m_Hdr.isSep=false;
}


CIplFilter::CIplFilter(const CIplFilter &f)
{
	m_Kernel=NULL;
	m_XKernel=NULL;
	m_YKernel=NULL;
	m_TKernel=NULL;
	Copy(f);
}

bool CIplFilter::Create(	unsigned int xsize,unsigned int ysize,
							short int dataType,const void *filter)
{
	if(!CheckDataType(dataType)){
		fprintf(pErrorFile,"CIplFilter::Create(), Invalid data type (%d)\n",dataType);
		xyDestroy();
		return false;
	}

	unsigned int depth=0;

	if(!CalcDepth(depth,dataType)){
		fprintf(pErrorFile,"CIplFilter::Create(), Invalid data type\n");
		xyDestroy();
		return false;
	}

	if(!filter){
		fprintf(pErrorFile,"CIplFilter::Create(), arg 'filter'=NULL\n");
		xyDestroy();
		return false;
	}

	m_Hdr.dataType=dataType;

	if(xyIsValid())
		xyDestroy();
		
	m_Hdr.w=xsize;
	m_Hdr.h=ysize;
	m_Hdr.depth=depth;
	m_Hdr.isSep=false;
	
	int cx=(int)((xsize-1)/2.0);
	int cy=(int)((ysize-1)/2.0);

	if(dataType==IMG_FLOAT){
		m_Kernel=(void*)iplCreateConvKernelFP(xsize,ysize,cx,cy,(float*)filter);
	}
	else{
		m_Kernel=(void*)iplCreateConvKernel(xsize,ysize,cx,cy,(int*)filter,0);
	}
	return true;
}

bool CIplFilter::CreateInt(	unsigned int xsize,unsigned int ysize,
							const long *filter,int shift)
{
	unsigned int depth=0;

	if(!CalcDepth(depth,IMG_LONG)){
		fprintf(pErrorFile,"CIplFilter::Create(), Invalid data type\n");
		xyDestroy();
		return false;
	}

	if(!filter){
		fprintf(pErrorFile,"CIplFilter::Create(), arg 'filter'=NULL\n");
		xyDestroy();
		return false;
	}

	m_Hdr.dataType=IMG_LONG;

	if(xyIsValid())
		xyDestroy();
		
	m_Hdr.w=xsize;
	m_Hdr.h=ysize;
	m_Hdr.depth=depth;
	m_Hdr.isSep=false;
	
	int cx=(int)((xsize-1)/2.0);
	int cy=(int)((ysize-1)/2.0);

	m_Kernel=(void*)iplCreateConvKernel(xsize,ysize,cx,cy,(int*)filter,shift);
	
	return true;
}


bool CIplFilter::CreateSep(	unsigned int xsize,unsigned int ysize,
							short int dataType,const void *xFilter,const void *yFilter)
{
	if(!CheckDataType(dataType)){
		fprintf(pErrorFile,"CIplFilter::CreateSep(), Invalid data type (%d)\n",dataType);
		Destroy();
		return false;
	}

	unsigned int depth=0;

	if(!CalcDepth(depth,dataType)){
		fprintf(pErrorFile,"CIplFilter::CreateSep(), Invalid data type\n");
		xyDestroy();
		return false;
	}

	if(!xFilter&&!yFilter){
		fprintf(pErrorFile,"CIplFilter::Create(), arg 'xFilter' and 'yFilter'=NULL\n");
		xyDestroy();
		return false;
	}

	if(xyIsValid())
		xyDestroy();
		
	m_Hdr.w=xsize;
	m_Hdr.h=ysize;
	m_Hdr.depth=depth;
	m_Hdr.dataType=dataType;
	m_Hdr.isSep=true;
	
	int cx=(int)((xsize-1)/2.0);
	int cy=(int)((ysize-1)/2.0);
	
	
	if(dataType==IMG_FLOAT){
		if(xFilter){
			m_XKernel=(void*)iplCreateConvKernelFP(xsize,1,cx,0,(float*)xFilter);
		}
		else
			m_Hdr.w=0;
		
		if(yFilter){
			m_YKernel=(void*)iplCreateConvKernelFP(1,ysize,0,cy,(float*)yFilter);
		}
		else
			m_Hdr.h=0;
	}
	else{
		if(xFilter){
			m_XKernel=(void*)iplCreateConvKernel(xsize,1,cx,0,(int*)xFilter,0);
		}
		else
			m_Hdr.w=0;

		if(yFilter){
			m_YKernel=(void*)iplCreateConvKernel(1,ysize,0,cy,(int*)yFilter,0);
		}
		else
			m_Hdr.h=0;
	}

	return true;
}

bool CIplFilter::CreateSepInt(	unsigned int xsize,unsigned int ysize,
								const long *xFilter,const long *yFilter,
								int xShift,int yShift)
{
	unsigned int depth=0;

	if(!CalcDepth(depth,IMG_LONG)){
		fprintf(pErrorFile,"CIplFilter::CreateSep(), Invalid data type\n");
		xyDestroy();
		return false;
	}

	if(!xFilter&&!yFilter){
		fprintf(pErrorFile,"CIplFilter::Create(), arg 'xFilter' and 'yFilter'=NULL\n");
		xyDestroy();
		return false;
	}

	if(xyIsValid())
		xyDestroy();
		
	m_Hdr.w=xsize;
	m_Hdr.h=ysize;
	m_Hdr.depth=depth;
	m_Hdr.dataType=IMG_LONG;
	m_Hdr.isSep=true;
	
	int cx=(int)((xsize-1)/2.0);
	int cy=(int)((ysize-1)/2.0);
	
	m_XKernel=(void*)iplCreateConvKernel(xsize,1,cx,0,(int*)xFilter,xShift);
	m_YKernel=(void*)iplCreateConvKernel(1,ysize,0,cy,(int*)yFilter,yShift);

	if(!m_YKernel||!m_XKernel) return false;

	return true;
}


bool CIplFilter::CreateT(unsigned int tsize,short int dataType,const void *tFilter)
{
	if(!CheckDataType(dataType)){
		fprintf(pErrorFile,"CIplFilter::CreateT(), Invalid data type (%d)\n",dataType);
		Destroy();
		return false;
	}

	unsigned int depth=0;

	if(!CalcDepth(depth,dataType)){
		fprintf(pErrorFile,"CIplFilter::CreateT(), Invalid data type\n");
		Destroy();
		return false;
	}

	if(tIsValid())
		tDestroy();
	
	m_Hdr.tDataType=IMG_FLOAT;
	m_Hdr.tSize=tsize;
	m_Hdr.tDepth=depth;
	unsigned int sz=tsize*depth;

	try{
		m_TKernel=new char[sz];
	}
	catch( bad_alloc xa )
	{
		fprintf(pErrorFile,"Filter::CreateT(), Error allocating memory"); 
		tDestroy();
		return false;
	}

	if(tFilter){
		memcpy(m_TKernel,tFilter,sz);
	}
	else{
		fprintf(pErrorFile,"_Img::CreateT(), Arg filter=NULL\n");
		memset(m_TKernel,0,sz);
		return false;
	}
	return true;
}



//Creates a numerical difference filter.
//Faster than a Dog but less accurate
bool CIplFilter::CreateNumDiffFilterX(int xorder)
{
	if (xorder<1 || xorder>2 ){fprintf(pErrorFile,"CreateNumDiffFilterX: Invalid Order\n");return false;}
	//Delete Old Kernel
	xyDestroy();
	fprintf(pErrorFile,
"CIplFilter::CreateNumDiffFilterX(), Can only have either X or Y seperable filter, check this out\n");
	
	int dfilt1[3]={-1,0,1};
	int dfilt2[5]={1,-8,0,8,-1};
	if (xorder==1)	
		m_XKernel=(void*)iplCreateConvKernel(3,1,1,0,dfilt1,1);
	else if (xorder==2)	
		m_XKernel=(void*)iplCreateConvKernel(5,1,2,0,dfilt2,3);

	if(!m_XKernel) return false;

	m_YKernel=NULL;
	m_Hdr.isSep=true;
	m_Hdr.dataType=IMG_LONG;
	return true;	
}
//Creates a numerical difference filter.
//Faster than a Dog but less accurate
bool CIplFilter::CreateNumDiffFilterY(int yorder)
{
	if (yorder<1 || yorder>2 ){fprintf(pErrorFile,"CreateNumDiffFilterY: Invalid Order\n");return false;}
	//Delete Old Kernel
	xyDestroy();
	fprintf(pErrorFile,
"CIplFilter::CreateNumDiffFilterY(), Can only have either an X or Y seperable filter, check this out\n");
	
	int dfilt1[3]={-1,0,1};
	int dfilt2[5]={1,-8,0,8,-1};
	
	if (yorder==1)	
		m_YKernel=(void*)iplCreateConvKernel(1,3,0,1,dfilt1,1);
	else if (yorder==2)	
		m_YKernel=(void*)iplCreateConvKernel(1,5,0,2,dfilt2,3);

	if(!m_YKernel) return false;
	
	m_XKernel=NULL;
	m_Hdr.isSep=true;
	m_Hdr.dataType=IMG_LONG;

	return true;	
}

bool CIplFilter::CheckDataType(short int dataType) const
{
	if( dataType!=IMG_LONG
		&&
		dataType!=IMG_FLOAT
		){
		return false;
	}

	return true;
}

bool CIplFilter::CalcDepth(unsigned int &depth,short int dataType) const
{
	switch(dataType){
	
	case IMG_LONG:	{
						depth=sizeof(long);
					} break;
	case IMG_FLOAT:	{
						depth=sizeof(float);
					} break;
	default:		{
						return false;
					}
	}
	return true;
}


//CreateDogFilter
//Creates a 2D Kernel of a normalised Dog filter at an arbitrary angle.
//Rotation is clockwise in degrees
bool CIplFilter::CreateDog(unsigned int xsize, unsigned int ysize,
							 unsigned int xorder, unsigned int yorder,
							 double sigma, double angle)
{
		//Delete Old Kernel
	if (m_YKernel!=NULL){xyDestroy();}
	if (m_XKernel!=NULL){xyDestroy();}
	if (m_Kernel!=NULL){xyDestroy();}

		//need to check whether has been resized

		//Allocate Kernel
	float *dog;
	try{
		dog=new float[xsize*ysize];
	}
	catch( bad_alloc xa )
	{
		fprintf(pErrorFile,"CIplFilter::CreateDog(), Error allocating memory"); 
		return false;
	}

	//int cx=(int)((xsize-1)/2.0);
	//int cy=(int)((ysize-1)/2.0);

	//int kx,ky;//Kernel Tap Values
	//double kxc,kyc;//Centralised Values
	//double rkxc,rkyc;//Rotated Centralised values
	//double dummy=0.0;
	//double angle_rads=3.1415926*angle/180.0;
	//double sum=0.0;
	//Create Rotated Kernel
	//	for (ky=0;ky<ysize;ky++){
	//		kyc=(double)(cy-ky);
	//		for (kx=0;kx<xsize;kx++){
	//			kxc=(double)(cx-kx);
	//			rkxc=kxc*cos(angle_rads)+kyc*sin(angle_rads);
	//			rkyc=kyc*cos(angle_rads)-kxc*sin(angle_rads);
	//			dummy=(Gaussian(rkyc,sigma)*HermiteFunction(rkyc,sigma,yorder));
	//			*(dog+ky*xsize+kx)=
	//				(float)(dummy*Gaussian(rkxc,sigma)*HermiteFunction(rkxc,sigma,xorder));
	//			//sum+=*(dog+ky*xsize+kx);	
	//		}
	//	}


	if(!CreateDogFilter(dog,xsize,ysize,xorder,yorder,sigma,angle,true))
	{
		fprintf(pErrorFile,"CIplFilter::CreateDog(), Error creating filters\n");
		return false;
	}
	//fprintf(pErrorFile,"Normalised Integral of Gaussian %d %d = %f\n",xorder,yorder,sum);

	m_Hdr.dataType=IMG_FLOAT;
	m_Hdr.isSep=false;
	m_Hdr.w=xsize; 
	m_Hdr.h=ysize;
	
	CalcDepth(m_Hdr.depth,IMG_FLOAT);
	m_Kernel=(void*)iplCreateConvKernelFP(xsize,ysize,(xsize-1)/2.0,(ysize-1)/2.0,dog);
	
	if(!m_Kernel) return false;
	
	return true;
}


/////////////////////////////////
//Creates a separable Dog filter
bool CIplFilter::CreateDogSep(	unsigned int xsize, unsigned int ysize,
										unsigned int xorder, unsigned int yorder,
										double sigma)
{
	//Delete Old Kernel
	if (m_YKernel!=NULL){xyDestroy();}
	if (m_XKernel!=NULL){xyDestroy();}
	if (m_Kernel!=NULL){xyDestroy();}

	
	//if( xsize % 2 == 0 || ysize % 2 == 0 )
	//{
//		fprintf(pErrorFile,"Filter::CreateDogSep(), Filter side arg even");
	//	return false;
	//}

	float *dogx, *dogy;
	try{
		dogx=new float[xsize];
		dogy=new float[ysize];
	}
	catch( bad_alloc xa )
	{
		fprintf(pErrorFile,"CIplFilter::CreateDogSep(), Error allocating memory"); 
		return false;
	}
	//double sumx=0.0f,sumy=0.0f;


	if(!CreateDogFilterSep(dogx,dogy,xsize,ysize,xorder,yorder,sigma,true))
	{
		fprintf(pErrorFile,"CIplFilter::CreateDogSep(), Error creating filters\n");
		return false;
	}

/*

	//Generate Kernels
	int k;double kc;
	int cx=(int)((xsize-1)/2.0);//Centre Position
	for (k=0;k<xsize;k++){
		kc=(double)(cx-k);
		*(dogx+k)=(float)(Gaussian(kc,sigma)*HermiteFunction(kc,sigma,xorder));
		//sumx+=*(dogx+k);
	}
	int cy=(int)((ysize-1)/2.0);//Centre Position
	for (k=0;k<ysize;k++){
		kc=(double)(cy-k);
		*(dogy+k)=(float)(Gaussian(kc,sigma)*HermiteFunction(kc,sigma,yorder));
		//sumy+=*(dogy+k);
	}
	//fprintf(pErrorFile,"Gaussian (dx%d,dy%d) XInt=%3.8f  YInt=%3.8f\n",xorder,yorder,sumx,sumy);
*/
	m_Hdr.dataType=IMG_FLOAT;
	m_Hdr.isSep=true;
	m_Hdr.w=xsize; 
	m_Hdr.h=ysize;
	CalcDepth(m_Hdr.depth,IMG_FLOAT);

	m_Hdr.isSep=true;
	
	m_XKernel=(void*)iplCreateConvKernelFP(xsize,1,(xsize-1)/2.0,0,dogx);
	m_YKernel=(void*)iplCreateConvKernelFP(1,ysize,0,(ysize-1)/2.0,dogy);

	//Clean Up
	delete[] dogx;
	delete[] dogy;

	if(!m_XKernel||!m_YKernel) return false;

	return true;
}


bool CIplFilter::CreateDolgT(	unsigned int tsize, unsigned int order, double alpha, 
								double tau,float multiplier)
{
	//(Re)Allocate Temporal Kernel
	if (m_TKernel)
		tDestroy();

	try{
		m_TKernel=new float[tsize];
		m_Hdr.tSize=tsize;
	}
	catch(bad_alloc xa)
	{
		fprintf(pErrorFile,"CIplFilter::CreateDolgT(), Cannot Allocate TKernel\n");
		tDestroy();
		return false;
	}

	if(!CreateDolgFilterT((float*)m_TKernel,tsize,order,alpha,tau,multiplier))
	{
		fprintf(pErrorFile,"CIplFilter::CreateDolgT(), Error creating filter\n");
		return false;
	}
	
	m_Hdr.tDataType=IMG_FLOAT;
	CalcDepth(m_Hdr.tDepth,IMG_FLOAT);
	
	return true;
}

bool CIplFilter::Copy(const AbstractFilter &src)
{
	if (this==&src)
		return false;

	bool status=true;

	if(src.xyIsValid())
	{
		if(src.IsSep())
			status=CreateSep(src.Width(),src.Height(),src.DataType(),
											src.XFilter(),src.YFilter());
		else
			status=Create(src.Width(),src.Height(),src.DataType(),src.Filter());
	}
	else
		xyDestroy();

	if(status==false){
		fprintf(pErrorFile,"CIplFilter::Copy(), Error copying spatial filter\n");
		return false;
	}
	
	if(src.tIsValid())
		status=CreateT(src.tSize(),src.tDataType(),src.TFilter());
	else
		tDestroy();

	if(status==false){
		fprintf(pErrorFile,"CIplFilter::Copy(), Error copying temporal filter\n");
		return false;
	}
	
	if(!xyIsValid()&&!tIsValid())
		return false;

	return true;
}

AbstractFilter* CIplFilter::New() const
{
	CIplFilter *pReturn;

	try{
		pReturn=new CIplFilter;
	}
	catch(bad_alloc xa)
	{
		fprintf(pErrorFile,"CIplFilter::New(), Error allocating memory\n");
		pReturn=NULL;
	}

	return pReturn;
}

AbstractFilter* CIplFilter::NewCopy() const
{
	CIplFilter *pReturn;

	try{
		pReturn=new CIplFilter(*this);
	}
	catch(bad_alloc xa)
	{
		fprintf(pErrorFile,"CIplFilter::NewCopy(), Error allocating memory\n");
		pReturn=NULL;
	}

	return pReturn;
}

AbstractFilter* CIplFilter::NewHeader() const
{
	CIplFilter *pReturn;

	try{
		pReturn=new CIplFilter;
		memcpy(&(pReturn->m_Hdr),&m_Hdr,sizeof(CIplFilterHdr));
	}
	catch(bad_alloc xa)
	{
		fprintf(pErrorFile,"CIplFilter::NewHeader(), Error allocating memory\n");
		pReturn=NULL;
	}
	return pReturn;
}


AbstractFilter& CIplFilter::operator=(const AbstractFilter &f)
{
	if(this!=&f)
	{
		Copy(f);
	}
	return *this;
}
	
CIplFilter::~CIplFilter()
{
	Destroy();
}

//Clean Up//
bool CIplFilter::Destroy()
{
	if(m_Hdr.dataType==IMG_FLOAT){
		if (m_Kernel)
			iplDeleteConvKernelFP((IplConvKernelFP*)m_Kernel);
		if (m_XKernel)
			iplDeleteConvKernelFP((IplConvKernelFP*)m_XKernel);
		if (m_YKernel)
			iplDeleteConvKernelFP((IplConvKernelFP*)m_YKernel);
	}
	else{//Integer Kernel
		if (m_Kernel)
			iplDeleteConvKernel((IplConvKernel*)m_Kernel);
		if (m_XKernel!=NULL)
			iplDeleteConvKernel((IplConvKernel*)m_XKernel);
		if (m_YKernel!=NULL)
			iplDeleteConvKernel((IplConvKernel*)m_YKernel);
	}

	if (m_TKernel)
		delete[] m_TKernel;

	//Set Pointers back to NULL
	m_Kernel=NULL;
	m_XKernel=NULL;
	m_YKernel=NULL;
	m_TKernel=NULL;
	
	memset(&m_Hdr,0,sizeof(CIplFilterHdr));
	m_Hdr.isSep=false;

	return true;
}

bool CIplFilter::xyDestroy()
{
	if(m_Hdr.dataType==IMG_FLOAT){
		if (m_Kernel)
			iplDeleteConvKernelFP((IplConvKernelFP*)m_Kernel);
		if (m_XKernel)
			iplDeleteConvKernelFP((IplConvKernelFP*)m_XKernel);
		if (m_YKernel)
			iplDeleteConvKernelFP((IplConvKernelFP*)m_YKernel);
	}
	else{//Integer Kernel
		if (m_Kernel)
			iplDeleteConvKernel((IplConvKernel*)m_Kernel);
		if (m_XKernel!=NULL)
			iplDeleteConvKernel((IplConvKernel*)m_XKernel);
		if (m_YKernel!=NULL)
			iplDeleteConvKernel((IplConvKernel*)m_YKernel);
	}
	
	//Set Pointers back to NULL
	m_Kernel=NULL;
	m_XKernel=NULL;
	m_YKernel=NULL;

	m_Hdr.w=m_Hdr.h=0;
	m_Hdr.dataType=IMG_NULL_DATATYPE;
	m_Hdr.depth=0;
	m_Hdr.isSep=false;

	return true;
}

bool CIplFilter::tDestroy()
{
	if( m_TKernel )
		delete [] m_TKernel;										
	
	m_TKernel=NULL;
	m_Hdr.tSize=0;
	m_Hdr.tDepth=0;
	m_Hdr.tDataType=IMG_NULL_DATATYPE;

	return true;
}


inline unsigned int CIplFilter::Width() const
{
	return m_Hdr.w;	
}

inline unsigned int CIplFilter::Height() const
{
	return m_Hdr.h;
}

inline unsigned int CIplFilter::tSize() const
{
	return m_Hdr.tSize;
}

inline const void* CIplFilter::Filter() const
{
	if(m_Kernel){
		if(m_Hdr.dataType==IMG_LONG)
			return ((IplConvKernel*)(m_Kernel))->values;
		else if(m_Hdr.dataType==IMG_FLOAT)
			return ((IplConvKernelFP*)(m_Kernel))->values;
	}	
	
	return NULL;
}

inline const void* CIplFilter::XFilter() const
{
	if(m_XKernel){
		if(m_Hdr.dataType==IMG_LONG)
			return ((IplConvKernel*)(m_XKernel))->values;
		else if(m_Hdr.dataType==IMG_FLOAT)
			return ((IplConvKernelFP*)(m_XKernel))->values;
	}	
	
	return NULL;
}

inline const void* CIplFilter::YFilter() const
{
	if(m_YKernel){
		if(m_Hdr.dataType==IMG_LONG)
			return ((IplConvKernel*)(m_YKernel))->values;
		else if(m_Hdr.dataType==IMG_FLOAT)
			return ((IplConvKernelFP*)(m_YKernel))->values;
	}	
	
	return NULL;
}

inline const void* CIplFilter::TFilter() const
{
	return m_TKernel;
}
	

inline const void *CIplFilter::iplFilter() const
{
	return m_Kernel;
}

inline const void *CIplFilter::iplXFilter() const
{
	return m_XKernel;
}

inline const void *CIplFilter::iplYFilter() const
{
	return m_YKernel;
}

inline bool CIplFilter::IsSep() const
{
	return m_Hdr.isSep;
}

inline short int CIplFilter::DataType() const
{
	return m_Hdr.dataType;
}

inline short int CIplFilter::tDataType() const
{
	return m_Hdr.tDataType;
}

inline bool CIplFilter::xyIsValid() const
{
	if(IsSep()){
		if(!m_XKernel||!m_YKernel)
		return false;
	}
	else
		if(!m_Kernel)
			return false;
	
	return true;
}

inline bool CIplFilter::tIsValid() const
{
	if(!m_TKernel)
		return false;

	return true;
}

inline const char* CIplFilter::ClassID() const
{
	return typeid(this).name();
}

inline bool CIplFilter::SameClass(const AbstractFilter &f) const
{
	if(typeid(this)!=typeid(&f))
		return false;
	
	return true;
}

/*
CIplFilter::CIplFilter(const CIplFilter &f)
{
	try{
		m_nTKernelSize=f.m_nTKernelSize;

		if(f.m_bKernelIsSep )
		{
			int nCols=0, nRows=0, anchorX=0, anchorY=0;

			m_bKernelIsSep=true;	
			m_Kernel=NULL;

			if(f.m_bKernelIsFP)
			{
				float *values;

				m_bKernelIsFP=true;

				if(f.m_XKernel){
					values=new float[((IplConvKernel*)f.m_XKernel)->nCols];
					iplGetConvKernelFP((IplConvKernelFP*)f.m_XKernel,&nCols,&nRows,&anchorX,&anchorY,&values);
					m_XKernel=(void*)iplCreateConvKernelFP(nCols,nRows,anchorX,anchorY,values);
					delete []values;
				}
				if(f.m_YKernel){
					values=new float[((IplConvKernel*)f.m_YKernel)->nRows];
					iplGetConvKernelFP((IplConvKernelFP*)f.m_YKernel,&nCols,&nRows,&anchorX,&anchorY,&values);
					m_YKernel=(void*)iplCreateConvKernelFP(nCols,nRows,anchorX,anchorY,values);
					delete []values;
				}
			}
			else
			{
				int* values;
				int nShiftR=0;

				m_bKernelIsFP=false;

				if(f.m_XKernel){
					values=new int[((IplConvKernel*)f.m_XKernel)->nCols];
					iplGetConvKernel((IplConvKernel*)f.m_XKernel,&nCols,&nRows,&anchorX,&anchorY,&values,&nShiftR);
					m_XKernel=(void*)iplCreateConvKernel(nCols,nRows,anchorX,anchorY,values,nShiftR);
					delete []values;
				}
				if(f.m_YKernel){
					values=new int[((IplConvKernel*)f.m_YKernel)->nRows];
					iplGetConvKernel((IplConvKernel*)f.m_YKernel,&nCols,&nRows,&anchorX,&anchorY,&values,&nShiftR);
					m_YKernel=(void*)iplCreateConvKernel(nCols,nRows,anchorX,anchorY,values,nShiftR);
					delete []values;
				}
			}
		}
		else
		{
			int nCols=0, nRows=0, anchorX=0, anchorY=0;

			m_bKernelIsSep=false;	
			m_XKernel=NULL;
			m_YKernel=NULL;

			if(f.m_bKernelIsFP)
			{
				float* values;

				m_bKernelIsFP=true;

				if(f.m_Kernel){
					values=new float[((IplConvKernel*)f.m_Kernel)->nCols*((IplConvKernel*)f.m_Kernel)->nRows];
					iplGetConvKernelFP((IplConvKernelFP*)f.m_Kernel,&nCols,&nRows,&anchorX,&anchorY,&values);
					m_Kernel=(void*)iplCreateConvKernelFP(nCols,nRows,anchorX,anchorY,values);
					delete []values;
				}
			}
			else
			{
				int *values;
				int nShiftR=0;

				m_bKernelIsFP=false;

				if(f.m_Kernel){
					values=new int[((IplConvKernel*)f.m_Kernel)->nCols*((IplConvKernel*)f.m_Kernel)->nRows];
					iplGetConvKernel((IplConvKernel*)f.m_Kernel,&nCols,&nRows,&anchorX,
															&anchorY,&values,&nShiftR);
					m_Kernel=(void*)iplCreateConvKernel(nCols,nRows,anchorX,anchorY,values,nShiftR);
					delete []values;
				}
			}

			m_nTKernelSize=f.m_nTKernelSize;
			if(f.m_TKernel)
			{
				try{
					m_TKernel=new float[m_nTKernelSize];
					memcpy(m_TKernel,f.m_TKernel,m_nTKernelSize*sizeof(float));
				}
				catch(bad_alloc xa)
				{
					fprintf(pErrorFile,"CIplFilter::operator=, error allocating memory");
				}
			}
			else
				m_TKernel=NULL;
		}
	}
	catch(bad_alloc xa)
	{
		fprintf(pErrorFile,"CIplFilter::operator=, error allocating memory");
	}
}

CIplFilter &CIplFilter::operator=(const CIplFilter &f)
{
if(this!=&f)
{
	try{
		if(m_Kernel||m_XKernel||m_YKernel||m_TKernel)
			Destroy();

		if(f.m_bKernelIsSep )
		{
			int nCols=0, nRows=0, anchorX=0, anchorY=0;

			m_bKernelIsSep=true;	
			m_Kernel=NULL;

			if(f.m_bKernelIsFP)
			{
				float *values;

				m_bKernelIsFP=true;

				if(f.m_XKernel){
					values=new float[((IplConvKernel*)f.m_XKernel)->nCols];
					iplGetConvKernelFP((IplConvKernelFP*)f.m_XKernel,&nCols,&nRows,&anchorX,&anchorY,&values);
					m_XKernel=(void*)iplCreateConvKernelFP(nCols,nRows,anchorX,anchorY,values);
					delete []values;
				}
				if(f.m_YKernel){
					values=new float[((IplConvKernel*)f.m_YKernel)->nRows];
					iplGetConvKernelFP((IplConvKernelFP*)f.m_YKernel,&nCols,&nRows,&anchorX,&anchorY,&values);
					m_YKernel=(void*)iplCreateConvKernelFP(nCols,nRows,anchorX,anchorY,values);
					delete []values;
				}
			}
			else
			{
				int* values;
				int nShiftR=0;

				m_bKernelIsFP=false;

				if(f.m_XKernel){
					values=new int[((IplConvKernel*)f.m_XKernel)->nCols];
					iplGetConvKernel((IplConvKernel*)f.m_XKernel,&nCols,&nRows,&anchorX,&anchorY,&values,&nShiftR);
					m_XKernel=(void*)iplCreateConvKernel(nCols,nRows,anchorX,anchorY,values,nShiftR);
					delete []values;
				}
				if(f.m_YKernel){
					values=new int[((IplConvKernel*)f.m_YKernel)->nRows];
					iplGetConvKernel((IplConvKernel*)f.m_YKernel,&nCols,&nRows,&anchorX,&anchorY,&values,&nShiftR);
					m_YKernel=(void*)iplCreateConvKernel(nCols,nRows,anchorX,anchorY,values,nShiftR);
					delete []values;
				}
			}
		}
		else
		{
			int nCols=0, nRows=0, anchorX=0, anchorY=0;

			m_bKernelIsSep=false;	
			m_XKernel=NULL;
			m_YKernel=NULL;

			if(f.m_bKernelIsFP)
			{
				float* values;

				m_bKernelIsFP=true;

				if(f.m_Kernel){
					values=new float[((IplConvKernel*)f.m_Kernel)->nCols*((IplConvKernel*)f.m_Kernel)->nRows];
					iplGetConvKernelFP((IplConvKernelFP*)f.m_Kernel,&nCols,&nRows,&anchorX,&anchorY,&values);
					m_Kernel=(void*)iplCreateConvKernelFP(nCols,nRows,anchorX,anchorY,values);
					delete []values;
				}
			}
			else
			{
				int *values;
				int nShiftR=0;

				m_bKernelIsFP=false;

				if(f.m_Kernel){
					values=new int[((IplConvKernel*)f.m_Kernel)->nCols*((IplConvKernel*)f.m_Kernel)->nRows];
					iplGetConvKernel((IplConvKernel*)f.m_Kernel,&nCols,&nRows,&anchorX,
															&anchorY,&values,&nShiftR);
					m_Kernel=(void*)iplCreateConvKernel(nCols,nRows,anchorX,anchorY,values,nShiftR);
					delete []values;
				}
			}

			m_nTKernelSize=f.m_nTKernelSize;

			if(f.m_TKernel)
			{
				try{
					m_TKernel=new float[m_nTKernelSize];
					memcpy(m_TKernel,f.m_TKernel,m_nTKernelSize*sizeof(float));
				}
				catch(bad_alloc xa)
				{
					fprintf(pErrorFile,"CIplFilter::operator=, error allocating memory");
				}
			}
			else
				m_TKernel=NULL;
		}
	}
	catch(bad_alloc xa)
	{
		fprintf(pErrorFile,"CIplFilter::operator=, error allocating memory");
	}
}
	return *this;
};
*/
/*
CIplFilter::~CIplFilter()
{
	Destroy();
}

//Clean Up//
bool CIplFilter::Destroy(void)
{
	//There are different Deallocation routines depending on type
	if (m_bKernelIsFP){//Floating Point Kernel
		if (m_Kernel!=NULL){iplDeleteConvKernelFP((IplConvKernelFP*)m_Kernel);}
		if (m_XKernel!=NULL){iplDeleteConvKernelFP((IplConvKernelFP*)m_XKernel);}
		if (m_YKernel!=NULL){iplDeleteConvKernelFP((IplConvKernelFP*)m_YKernel);}
	}
	else{//Integer Kernel
		if (m_Kernel!=NULL){iplDeleteConvKernel((IplConvKernel*)m_Kernel);}
		if (m_XKernel!=NULL){iplDeleteConvKernel((IplConvKernel*)m_XKernel);}
		if (m_YKernel!=NULL){iplDeleteConvKernel((IplConvKernel*)m_YKernel);}
	}

	if (m_TKernel!=NULL){delete[] m_TKernel;}

	//Set Pointers back to NULL
	m_Kernel=NULL;
	m_XKernel=NULL;
	m_YKernel=NULL;
	m_TKernel=NULL;
	m_bKernelIsFP=false;
	m_bKernelIsSep=false;
	m_nTKernelSize=0;
	return true;
}


*/


///////////////////////////////////////////
//QuantiseFilter
//Turn a floating point filter into an integer version
//for faster execution.
///////////////////////////////////////////
/*
bool CIplFilter::QuantiseFilter(int bitdepth)
{
	if (bitdepth<3 || bitdepth>16){fprintf(pErrorFile,"QuantiseFilter: Invalid Bitdepth\n");
		return false;}
	if (m_Hdr.dataType!=IMG_FLOAT){fprintf(pErrorFile,"QuantiseFilter: Filter is not FP\n");
		return false;}
	
	IplConvKernelFP *Kernel[2]={NULL,NULL};
	void **pKernel[2];	
	bool isSep;					//is it separable
	int xs,ys,k;				//size of filter
	int cx,cy;					//centre of filter
	int nfilts;					//number of filters (1 for 2D, 2 for sep)
	float *fVals;				//Pointer to old Float Kernel Values	
	float scale=(float)(1<<bitdepth);//Target Integral of Filter
	int *iVals;					//Pointer to New Integer Kernel Values
	
	//Setup depends on whether we have 2D or Separable Filter
	if (m_Hdr.isSep)
	{
		Kernel[0]=(IplConvKernelFP*)m_XKernel;
		Kernel[1]=(IplConvKernelFP*)m_YKernel;
		pKernel[0]=&m_XKernel;
		pKernel[1]=&m_YKernel;
		isSep=true;
		nfilts=2;
	}
	else 
	{
		Kernel[0]=(IplConvKernelFP*)m_Kernel;
		Kernel[1]=NULL;
		pKernel[0]=&m_Kernel;
		nfilts=1;
		isSep=false;
	}
	
	//Do single 2D filter or cycle through the two separable ones
	for (int n=0;n<nfilts;n++){
		if (Kernel[n]!=NULL){

		xs=Kernel[n]->nCols;  ys=Kernel[n]->nRows;
		cx=Kernel[n]->anchorX; cy=Kernel[n]->anchorY;
		fVals=Kernel[n]->values;//Floating point values
		iVals=new int[xs*ys];	//Integer Values
		
		for (k=0;k<xs*ys;k++){
			*(iVals+k)=(int)(*(fVals+k)*scale);
		}

		//make new integer kernel
		iplDeleteConvKernel((IplConvKernel*)(*pKernel[n]));
		*pKernel[n]=(void*)iplCreateConvKernel(xs,ys,cx,cy,iVals,bitdepth);
		m_Hdr.dataType=IMG_LONG;
		m_Hdr.isSep=isSep;
		delete[] iVals;
		}
	}

	return true;
}
*/
/*
//////////////////////////////////////////////////////////
//Print - Output the filter tap values to TRACE or file
//
void CIplFilter::Print(const char *Filename)
{
	FILE *fout=NULL;
	char text[500];
	float* fkernel;
	int* ikernel;

	if (Filename!=NULL){
		fout=fopen(Filename,"wt");
		if (fout==NULL){fprintf(pErrorFile,"Unable to open filter output file\n");}
	}
	
	int xp,yp;
	
	if (m_bKernelIsSep){	//Separable filters
		sprintf(text,"Separable Filter\nTap,Value\n");
		if (fout==NULL){fprintf(stdout,text);}else{fprintf(fout,"%s",text);}
		if (m_bKernelIsFP){//Floating point filters
			int xs=((IplConvKernelFP*)(m_XKernel))->nCols;
			int ys=((IplConvKernelFP*)(m_YKernel))->nRows;
			for (xp=0;xp<xs;xp++){
				fkernel=((IplConvKernelFP*)(m_XKernel))->values;
				sprintf(text,"%d,%3.3f\n",xp,*(fkernel+xp));	
				if (fout==NULL){fprintf(stdout,text);}else{fprintf(fout,"%s",text);}
			}
			sprintf(text,"\n");
			for (yp=0;yp<ys;yp++){
				fkernel=((IplConvKernelFP*)(m_YKernel))->values;
				sprintf(text,"%d,%3.3f\n",yp,*(fkernel+yp));	
				if (fout==NULL){fprintf(stdout,text);}else{fprintf(fout,"%s",text);}
			}
		}
		else {	//Integer filters
			sprintf(text,"RShift=%d\n",((IplConvKernel*)(m_XKernel))->nShiftR);
			if (fout==NULL){fprintf(stdout,text);}else{fprintf(fout,"%s",text);}
			int xs=((IplConvKernel*)(m_XKernel))->nCols;
			int ys=((IplConvKernel*)(m_YKernel))->nRows;
			for (xp=0;xp<xs;xp++){
				ikernel=((IplConvKernel*)(m_XKernel))->values;
				sprintf(text,"%d,%d\n",xp,*(ikernel+xp));	
				if (fout==NULL){fprintf(stdout,text);}else{fprintf(fout,"%s",text);}
			}
			sprintf(text,"\nRShift=%d\n",((IplConvKernel*)(m_YKernel))->nShiftR);
			if (fout==NULL){fprintf(stdout,text);}else{fprintf(fout,"%s",text);}
			for (yp=0;yp<ys;yp++){
				ikernel=((IplConvKernel*)(m_YKernel))->values;
				sprintf(text,"%d,%d\n",yp,*(ikernel+yp));	
				if (fout==NULL){fprintf(stdout,text);}else{fprintf(fout,"%s",text);}
			}
		}
	}
	else	//2D Fitlers
	{
		sprintf(text,"2D Filter\nTap,Value\n");
		if (fout==NULL){fprintf(stdout,text);}else{fprintf(fout,"%s",text);}
		if (m_bKernelIsFP){//Floating point filters
			int xs=((IplConvKernelFP*)(m_Kernel))->nCols;
			int ys=((IplConvKernelFP*)(m_Kernel))->nRows;
			for (yp=0;yp<ys;yp++){
				for (xp=0;xp<xs;xp++){
					fkernel=((IplConvKernelFP*)(m_Kernel))->values;
					sprintf(text,"%3.3f,",*(fkernel+xp+yp*xs));	
					if (fout==NULL){fprintf(stdout,text);}else{fprintf(fout,"%s",text);}
				}
				
				sprintf(text,"\n");
				if (fout==NULL){fprintf(stdout,text);}else{fprintf(fout,"%s",text);}
			}
			
		}
		else {	//Integer filters
			sprintf(text,"RShift=%d\n",((IplConvKernel*)(m_Kernel))->nShiftR);
			int xs=((IplConvKernelFP*)(m_Kernel))->nCols;
			int ys=((IplConvKernelFP*)(m_Kernel))->nRows;
			for (yp=0;yp<ys;yp++){
				for (xp=0;xp<xs;xp++){
					ikernel=((IplConvKernel*)(m_Kernel))->values;
					sprintf(text,"%d,",*(ikernel+xp+yp*xs));	
					if (fout==NULL){fprintf(stdout,text);}else{fprintf(fout,"%s",text);}
				}
				
				sprintf(text,"\n");
				if (fout==NULL){fprintf(stdout,text);}else{fprintf(fout,"%s",text);}
			}
		}
	}
	
	if (Filename!=NULL && fout!=NULL){fclose(fout);}
}

  */
  /*
///////////////////////////////////////////
//Convolution
///////////////////////////////////////////

//Single Public convolution routine.  Choose Automatically
//the correct convolution routine based on the kernel
bool CIplFilter::Convolve(IplImage *src, IplImage *dst)
{
	if (src==NULL || dst==NULL){fprintf(pErrorFile,"ERROR - CIplFilter::Convolve.  src or dst ==NULL\n");return false;}
	if (m_bKernelIsSep)
	{
		if (m_XKernel==NULL && m_YKernel==NULL){fprintf(pErrorFile,"ERROR - CIplFilter::Convolve.  Sep Kernel NULL\n");return false;}
		if (m_bKernelIsFP)
		{
			return ConvolveSep2DFP(src,dst);
		}
		else
		{
			return ConvolveSep2D(src,dst);
		}
	}
	else
	{	
		if (m_Kernel==NULL){fprintf(pErrorFile,"ERROR - CIplFilter::Convolve.  Kernel==NULL\n");return false;}
		if (m_bKernelIsFP)
		{
			return Convolve2DFP(src,dst);
		}
		else
		{
			return Convolve2D(src,dst);
		}
	}
	//aja
	//ASSERT(true);
}

bool CIplImg::ConvolveSep2DFP(IplImage *src,IplImage *dst)
{
	//Check Inputs of correct type
	if (src->depth!=IPL_DEPTH_32F){fprintf(pErrorFile,"Convolve Failed: Input is not IPL_DEPTH_32F\n");return false;}
	if (dst->depth!=IPL_DEPTH_32F){fprintf(pErrorFile,"Convolve Failed: Output is not IPL_DEPTH_32F\n");return false;}
	iplConvolveSep2DFP(src,dst,(IplConvKernelFP*)m_XKernel,(IplConvKernelFP*)m_YKernel);
	return (true);
}

bool CIplImg::ConvolveSep2D(IplImage *src, IplImage *dst)
{
	//Check Inputs of correct type
	if (src->depth==IPL_DEPTH_32F){fprintf(pErrorFile,"Convolve Failed: Input is IPL_DEPTH_32F\n");return false;}
	if (dst->depth==IPL_DEPTH_32F){fprintf(pErrorFile,"Convolve Failed: Ouput is IPL_DEPTH_32F\n");return false;}
	iplConvolveSep2D(src,dst,(IplConvKernel*)m_XKernel,(IplConvKernel*)m_YKernel);
	return (true);
}

bool CIplImg::Convolve2DFP(IplImage *src, IplImage *dst)
{
	//Check Inputs of correct type
	if (src->depth!=IPL_DEPTH_32F){fprintf(pErrorFile,"Convolve Failed: Input is not IPL_DEPTH_32F\n");return false;}
	if (dst->depth!=IPL_DEPTH_32F){fprintf(pErrorFile,"Convolve Failed: Output is not IPL_DEPTH_32F\n");return false;}
	IplConvKernelFP *k=(IplConvKernelFP*)m_Kernel;
	iplConvolve2DFP(src,dst,&k,1,IPL_SUM);
	return (true);
}
bool CIplmg::Convolve2D(IplImage *src, IplImage *dst)
{
	//Check Inputs of correct type
	if (src->depth==IPL_DEPTH_32F){fprintf(pErrorFile,"Convolve Failed: Input is IPL_DEPTH_32F\n");return false;}
	if (dst->depth==IPL_DEPTH_32F){fprintf(pErrorFile,"Convolve Failed: Output is IPL_DEPTH_32F\n");return false;}
	IplConvKernel *k=(IplConvKernel*)m_Kernel;
	iplConvolve2D(src,dst,&k,1,IPL_SUM);
	return (true);
}


//Convolve a sequence of images with temporal filter
bool CIplImg::TConvolve(IplImage **src,IplImage *dst)
{
	if (m_nTKernelSize==0){fprintf(pErrorFile,"Error:TConvolve - Kernelsize=0\n");return false;}
	if (m_TKernel==NULL){fprintf(pErrorFile,"Error:TConvolve - Kernel=NULL\n");return false;}
	if (*(src)==NULL){fprintf(pErrorFile,"Error:src==NULL\n");}
	if (dst==NULL){fprintf(pErrorFile,"Error:dst==NULL\n");}
	if ((*(src))->depth != dst->depth){fprintf(pErrorFile,"Error:Failed - Src Depth != Dest Depth\n");return false;}
	
	//Floating point filtering operation
	if ((*(src))->depth == IPL_DEPTH_32F && dst->depth==IPL_DEPTH_32F){
		//Make Temp Img
		IplImage *temp=iplCloneImage(*src);
		
		//First Value
		iplMultiplySFP(*src, dst, *m_TKernel);

		for (int t=1;t<m_nTKernelSize;t++){	//tap 0 is always zero so start at 1
			if (fabs(*(m_TKernel+t))>0.0001){
				if (*(src+m_nTKernelSize-t)!=NULL){
					iplMultiplySFP(*(src+m_nTKernelSize-t), temp, *(m_TKernel+t));
					iplAdd(temp,dst,dst);
				}
				else{
					fprintf(pErrorFile,"Error:src frame==NULL\n");
				}
			}
		}
		iplDeallocate(temp,IPL_IMAGE_ALL);
		return true;
	}
	else{//Integer operation
		//Make Temp Img 
		IplImage *temp=iplCloneImage(dst);
		
		//First Value
		iplMultiplyS(*(src), dst, (int)(*m_TKernel));
		for (int t=1;t<m_nTKernelSize;t++){	//tap 0 is always zero so start at 1 
			if (*(src+m_nTKernelSize-t)!=NULL){
				iplMultiplyS(*(src+m_nTKernelSize-t), temp, (int)(*(m_TKernel+t)));
				iplAdd(temp,dst,dst);
			}
			else{
				fprintf(pErrorFile,"Error:src frame==NULL\n");
			}
		}

		iplDeallocate(temp,IPL_IMAGE_ALL);
		return true;
	}
	return false;
}

////////////////////////////////////
//Operator * denotes convolution
////////////////////////////////////
/*
CIplImage * operator*(CIplFilter &filter, CIplImage &image)
{
	CIplImage *result = image.BlankCopy();
	filter.Convolve(image.pImg ,result->pImg);
	return result;
}
*/

/*
//Other Filters
////////////////////
bool CIplFilter::Median(IplImage *src,IplImage *dst,int size)
{
	if (src==NULL || dst==NULL){return false;}
	if (size<2 || size>100){return false;}
	int c=(int)((size-1)/2.0);
	iplMedianFilter(src,dst,size,size,c,c);
	return true;	
}
bool CIplFilter::Average(IplImage *src,IplImage *dst,int size)
{
	if (src==NULL || dst==NULL){return false;}
	if (size<2 || size>100){return false;}
	int c=(int)((size-1)/2.0);
	iplBlur(src,dst,size,size,c,c);
	return true;
}
bool CIplFilter::Blur(IplImage *src,IplImage *dst)
{
	if (src==NULL || dst==NULL){return false;}
	iplFixedFilter(src,dst,IPL_GAUSSIAN_3x3);
	return true;
}


////////////////////////
//Add Gaussian Noise
bool CIplFilter::AddNoise(IplImage *src,double stddev)
{
	if (src==NULL){return false;}
	unsigned int seed=(unsigned int)rand();
	IplNoiseParam np;
	if (src->depth==IPL_DEPTH_32F){
		iplNoiseGaussianInitFP(&np,seed,0.0f,(float)stddev);
		iplNoiseImage(src,&np);
		return true;
	}
	else
	{
		iplNoiseGaussianInit(&np,seed,0,(int)stddev);
		iplNoiseImage(src,&np);
		return true;		
	}
	return true;
}
*/

////////////////////////////////////////////////////////////////////////////
// IPL IMAGE
////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// Modification of the CIPLImage class (J.Dale & G.Cowe), that interfaces
// the IPLImage libraries) to support compatibility with the AbstractImage class 
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//Upon creation, set eveything to defaults.  Especially Pointers to NULL
CIplImg::CIplImg()
{
	m_Img=NULL;
	m_Bmi=NULL;
	m_ImgDisp=NULL;

	m_bHasROI=false;
	Destroy();
}

CIplImg::CIplImg(	unsigned int w,unsigned int h,
					unsigned int channels,short int dataType)
{
	m_Img=NULL;
	m_Bmi=NULL;
	m_ImgDisp=NULL;

	m_bHasROI=false;
	Create(w,h,channels,dataType);
}

CIplImg::CIplImg(const CIplImg &img)
{
	m_Img=NULL;
	m_Bmi=NULL;
	m_ImgDisp=NULL;

	m_bHasROI=false;

	Copy(img);
}

AbstractImage &CIplImg::operator=(const AbstractImage &img)
{
	if (this!=&img) 
	{
		Copy(img);
	}
	return *this;
}


CIplImg::~CIplImg ()
{
	Destroy();
}

//////////////////////////////////////////////////////////////////////
//DeAllocates the image (if it is valid) and resets to defaults.
bool CIplImg::Destroy()
{
	//	Delete IPL image if it is valid.  This will delete Header and any data.
	if (m_Img!=NULL){
		if (m_ImgDisp==m_Img) m_ImgDisp = NULL; //Avoid deleting twice
		iplDeallocate(m_Img,IPL_IMAGE_ALL);	//Will deallocate ROI too
		
		//aja
		//ASSERT(m_Img->imageData==NULL);
	}
	//Delete IPL Display Image if it is valid
	if (m_ImgDisp!=NULL) {
		iplDeallocate(m_ImgDisp,IPL_IMAGE_ALL);
	}

	//Set pointers back to NULL
	m_Img=NULL;	
	m_ImgDisp=NULL;
	
	//Clean up Bitmap Infoheader and pallette
	if (m_Bmi!=NULL){
		delete[] m_Bmi;m_Bmi=NULL;
	}

//	m_bHasROI=false;
	return true;
}	


bool CIplImg::CheckDataType(short int dataType) const
{
	if( dataType!=IMG_UCHAR
		&&
		dataType!=IMG_LONG
		&&
		dataType!=IMG_FLOAT
		)
		return false;

	return true;
}

bool CIplImg::ImgDataTypeToIPLDataType(int &iplDataType,short int imgDataType) const
{
	if(imgDataType==IMG_NULL_DATATYPE)
	{
		fprintf(pErrorFile,
	"CIplImg::ImgDataTypeToIPLDataType(), ImgDataType = IMG_NULL_DATATYPE\n");
			return false;
	}
	else if(imgDataType==IMG_UCHAR)
	{
		iplDataType=IPL_DEPTH_8U;
	}
	else if(imgDataType==IMG_LONG)
	{
		iplDataType=IPL_DEPTH_32S;
	}
	else if(imgDataType==IMG_FLOAT){
		iplDataType=IPL_DEPTH_32F;
	}
	else{
fprintf(pErrorFile,
	"CIplImg::ImgDataTypeToIPLDataType(), Datatype (%d) unrecognised\n",imgDataType);
		return false;
	}
	return true;
}

bool CIplImg::IPLDataTypeToImgDataType(short int &imgDataType,int iplDataType) const
{
	
	if(iplDataType==IPL_DEPTH_8U){
		imgDataType=IMG_UCHAR;
	}
	else if(iplDataType==IPL_DEPTH_32S){
		imgDataType=IMG_LONG;
	}
	else if(iplDataType==IPL_DEPTH_32F){
		imgDataType=IMG_FLOAT;
	}
	else{
		fprintf(pErrorFile,"CIplImg::IPLDataTypeToImgDataType(), IPL label (%d) not supported\n",
						iplDataType);
		imgDataType=IMG_NULL_DATATYPE;
		return false;
	}
	return true;
}


bool CIplImg::CalcDepth(unsigned int &depth,short int dataType) const
{
	switch(dataType){
	
	case IMG_UCHAR: {
						depth=1;
					} break;
	case IMG_LONG:	{
						depth=sizeof(long);
					} break;
	case IMG_FLOAT:	{
						depth=sizeof(float);
					} break;
	default:		{
						return false;
					}
	}
	return true;
}

inline unsigned int CIplImg::DataSz() const
{
	
	if(m_Img){
		unsigned int depth;
		short int dataType;

		if(!IPLDataTypeToImgDataType(dataType,m_Img->depth)){
			fprintf(pErrorFile,"CIplImg::DataSz(), Unrecognised data type\n");
			return 0;
		}
		
		if(!CalcDepth(depth,dataType)){
			fprintf(pErrorFile,"CIplImg::DataSz(), Calculating depth, unrecognised data type\n");
			return 0;
		}
		return m_Img->width*m_Img->height*m_Img->nChannels*depth;
	}
	else
		return 0;
}


	//WARNING:: there may be issues with the qword alignment
bool CIplImg::Create(	unsigned int w,unsigned int h,
						unsigned int channels,short int dataType,
						const void *data)
{
	if(!CheckDataType(dataType)){
		fprintf(pErrorFile,"virtual CIplImg::Create(), Invalid data type (%d)\n",dataType);
		Destroy();
		return false;
	}

	unsigned int depth=0;
	int iplDataType=0;

	if(!CalcDepth(depth,dataType)){
		fprintf(pErrorFile,"virtual CIplImg::Create(), Error calculating depth\n");
		Destroy();
		return false;
	}

	if(!ImgDataTypeToIPLDataType(iplDataType,dataType)){
		fprintf(pErrorFile,"virtual CIplImg::Create(), Error converting data type to IPL label\n");
		Destroy();
		return false;
	}

		//if the image data is in the same format copy it in directly
		// TODO: should check for ipl error 
	if (IsValid()&&DataType()==dataType&&DataSz()==w*h*channels*depth){
		if(data)
			memcpy(m_Img->imageData,data,DataSz());

		return true;
	}
	else
		Destroy();

	if(channels==1) //	Allocate IPL Image Header for GreyScale
	{
		m_Img=iplCreateImageHeader(1,0,iplDataType,"GRAY","G",IPL_DATA_ORDER_PIXEL,IPL_ORIGIN_TL,
			IPL_ALIGN_QWORD,w,h,NULL,NULL,"Image",NULL);
		
	}
	else if(channels==3) //	Allocate IPL Image Header for 24 bit RGB
	{
		m_Img=iplCreateImageHeader(3,0,iplDataType,"RGB","BGR",IPL_DATA_ORDER_PIXEL,IPL_ORIGIN_TL,
			IPL_ALIGN_QWORD,w,h,NULL,NULL,"Image",NULL);
	}
	else
	{
		fprintf(pErrorFile,
			"virtual CIplImg::Create(), Doesn't support (%d) channel images as requested (only 1 & 3)\n",
					channels);
		return false;
	}
	//Check we allocated header OK
	if (m_Img==NULL){
		fprintf(pErrorFile,"virtual CIplImg::Create(), Could not Create Ipl Image Header\n");
		return false;
	}

//	Allocate IPL Image Buffer of size specified in Header and clear it
//  IPLAllocate Image creates an image which is padded to be QWORD aligned
	if (iplDataType==IPL_DEPTH_32F)
		iplAllocateImageFP(m_Img,1,0);
	else
		iplAllocateImage(m_Img,1,0);

	if (data)
		memcpy(m_Img->imageData,data,DataSz());

	return true;
}


bool CIplImg::Copy(const AbstractImage &src)
{
	if (this==&src)
		return false;

	if(!src.IsValid())
	{
		Destroy();
		int iplDataType;
		
		if(!ImgDataTypeToIPLDataType(iplDataType,src.DataType()))
		{
			fprintf(pErrorFile,
		"virtual CIplImg::Copy(), Unable to convert src datatype (%d) to IPL label\n",
				src.DataType());
		}

		if(src.Channels()==1) //	Allocate IPL Image Header for GreyScale
		{
			m_Img=iplCreateImageHeader(1,0,iplDataType,"GRAY","G",IPL_DATA_ORDER_PIXEL,IPL_ORIGIN_TL,
				IPL_ALIGN_QWORD,src.Width(),src.Height(),NULL,NULL,"Image",NULL);
		}
		else if(src.Channels()==3) //	Allocate IPL Image Header for 24 bit RGB
		{
			m_Img=iplCreateImageHeader(3,0,iplDataType,"RGB","BGR",IPL_DATA_ORDER_PIXEL,IPL_ORIGIN_TL,
				IPL_ALIGN_QWORD,src.Width(),src.Height(),NULL,NULL,"Image",NULL);
		}
		else
		{
			fprintf(pErrorFile,
				"virtual CIplImg::Copy(), %d Channel images unsupported (only 1 & 3)\n",
					src.Channels());
			return false;
		}
		fprintf(pErrorFile,"virtual CIplImg::Copy() src invalid\n"); 
		
		return false;
	}

	if(SameClass(src)){
		Destroy();
		m_Img=iplCloneImage((IplImage*)src.Header());
		return true;
	}
	
	return Create(src.Width(),src.Height(),src.Channels(),src.DataType(),src.Data());
}

bool CIplImg::BlankCopy(const AbstractImage &src)
{
	if (this==&src)
		return false;

	if(!src.IsValid())
	{
		
		Destroy();
		int iplDataType;
		
		if(!ImgDataTypeToIPLDataType(iplDataType,src.DataType()))
		{
			fprintf(pErrorFile,
		"virtual CIplImg::BlankCopy(), Unable to convert src datatype (%d) to IPL label\n",
				src.DataType());
		}

		if(src.Channels()==1) //	Allocate IPL Image Header for GreyScale
		{
			m_Img=iplCreateImageHeader(1,0,iplDataType,"GRAY","G",IPL_DATA_ORDER_PIXEL,IPL_ORIGIN_TL,
				IPL_ALIGN_QWORD,src.Width(),src.Height(),NULL,NULL,"Image",NULL);
		}
		else if(src.Channels()==3) //	Allocate IPL Image Header for 24 bit RGB
		{
			m_Img=iplCreateImageHeader(3,0,iplDataType,"RGB","BGR",IPL_DATA_ORDER_PIXEL,IPL_ORIGIN_TL,
				IPL_ALIGN_QWORD,src.Width(),src.Height(),NULL,NULL,"Image",NULL);
		}
		else
		{
			fprintf(pErrorFile,
				"virtual CIplImg::BlankCopy(), %d Channel images unsupported (only 1 & 3)\n",
					src.Channels());
			return false;
		}
		fprintf(pErrorFile,"virtual CIplImg::Copy() src invalid\n"); 
		
		return false;
	}

	return Create(src.Width(),src.Height(),src.Channels(),src.DataType());
}


bool CIplImg::Zero()
{
	if (!IsValid()){
		fprintf(pErrorFile,"CIplImg::Zero(), this image invalid\n");
		return false;
	}

	if (m_Img->depth==IPL_DEPTH_32F){
		iplSetFP(m_Img,0.0f);
	}
	else{
		iplSet(m_Img,0);
	}
	return true;
}

AbstractImage* CIplImg::New() const
{
	CIplImg *pReturn;

	try{
		pReturn=new CIplImg;
	}
	catch(bad_alloc xa)
	{
		fprintf(pErrorFile,"CIplImg::New(), Error allocating memory\n");
		pReturn=NULL;
	}

	return pReturn;
}

AbstractImage* CIplImg::NewCopy() const
{
	CIplImg *pReturn;

	try{
		pReturn=new CIplImg(*this);
	}
	catch(bad_alloc xa)
	{
		fprintf(pErrorFile,"CIplImg::NewCopy(), Error allocating memory\n");
		pReturn=NULL;
	}

	return pReturn;
}

AbstractImage* CIplImg::NewHeader() const
{
	CIplImg *pReturn;

	if(!m_Img)
		return NULL;

	try{
		pReturn=new CIplImg;
		int iplDataType=m_Img->depth;

		if(Channels()==1) 
		{
			pReturn->m_Img=iplCreateImageHeader(1,0,iplDataType,"GRAY","G",IPL_DATA_ORDER_PIXEL,
												IPL_ORIGIN_TL,
												IPL_ALIGN_QWORD,Width(),Height(),NULL,NULL,"Image",NULL);
		}
		else if(Channels()==3)
		{
			pReturn->m_Img=iplCreateImageHeader(3,0,iplDataType,"RGB","BGR",IPL_DATA_ORDER_PIXEL,
												IPL_ORIGIN_TL,
												IPL_ALIGN_QWORD,Width(),Height(),NULL,NULL,"Image",NULL);
		}
		else
		{
			fprintf(pErrorFile,
				"virtual CIplImg::BlankCopy(), %d Channel images unsupported (only 1 & 3)\n",
					Channels());
			return false;
		}
	}
	catch(bad_alloc xa)
	{
		fprintf(pErrorFile,"CIplImg::NewHeader(), Error allocating memory\n");
		pReturn=NULL;
	}
	return pReturn;
}



AbstractImage* CIplImg::NewBlankCopy() const
{
	CIplImg *pReturn;

	try{
		pReturn=new CIplImg;
		pReturn->BlankCopy(*this);
	}
	catch(bad_alloc xa)
	{
		fprintf(pErrorFile,"CIplImg::NewBlankCopy(), Error allocating memory\n");
		pReturn=NULL;
	}

	return pReturn;
}

AbstractImage* CIplImg::NewBlankCopy(unsigned int w,unsigned int h) const
{
	CIplImg *pReturn;

	try{
		pReturn=new CIplImg;
		pReturn->Create(w,h,Channels(),DataType());
	}
	catch(bad_alloc xa)
	{
		fprintf(pErrorFile,"CIplImg::NewBlankCopy(), Error allocating memory\n");
		pReturn=NULL;
	}

	return pReturn;
}

bool CIplImg::Resize(const AbstractImage &src)
{
	if(this==&src)
		return false;

	if(!src.IsValid()){
		fprintf(pErrorFile,"CIplImg::Resize(), Arg 'src' invalid\n");
		return false;
	}

	if(!IsValid()){
		return Copy(src);
	}

	if(!SameChannels(src)||!SameDataType(src))
	{
		fprintf(pErrorFile,
			"CIplImg::Resize(), this img and arg 'src' have different formats\n");
		fprintf(pErrorFile,
			"\t this img:%d channels, data-type label: %d, arg:%d channels, data-type label:%d\n", 
						Channels(),DataType(),src.Channels(),src.DataType());
		return false;
	}

	/*	Can't do this 'cos iplResizeFit Func does not take const src arg
	const CIplImg *pSrc;

	pSrc=dynamic_cast<const CIplImg*>(&src);

	CIplImg temp;
	if(!pSrc){
		temp.Copy(src);
		pSrc=&temp;
	}
	*/

	CIplImg convertedSrc;

	if(!SameClass(src)){
		if(!convertedSrc.Create(src.Width(),src.Height(),src.Channels(),src.DataType(),src.Data()))
		{
			fprintf(pErrorFile,"CIplImg::Resize(), Error creating temporary image\n");
			return false;
		}
	}
	else
		convertedSrc.Copy(src);


	iplResizeFit(convertedSrc.GetpImg(),GetpImg(),IPL_INTER_LINEAR);
	
	return true;
}

bool CIplImg::Convert(const AbstractImage &src)
{
	if (this==&src)
		return false;

	if(!src.IsValid())
	{
		fprintf(pErrorFile,"CIplImg::Convert(), Arg 'src' invalid\n");
		return false;
	}

	if(!IsValid()||SameType(src))
		return Copy(src);

	CIplImg temp;
	const AbstractImage *pSource=&src;

	if(!SameDimensions(*pSource)){
		fprintf(pErrorFile,"CIplImg::Convert(), dimension scaling not yet implemented\n");
		
		return false;
	}

	if(!SameDataType(*pSource)&&SameChannels(*pSource)){
		if(!ConvertDataType(*pSource))
		{
			fprintf(pErrorFile,"CIplImg::Convert(), Error converting data type\n");
			return false;
		}
	}

	if(!SameChannels(*pSource)){
		if(pSource->Channels()==3&&Channels()==1)
		{
			if(!ToGreyscale(*pSource)){
				fprintf(pErrorFile,"CIplImg::Convert(), Error converting image to greyscale\n");
				return false;
			}
		}
		else if(pSource->Channels()==1&&Channels()==3)
		{
			if(!To3ChannelGreyscale(*pSource)){
				fprintf(pErrorFile,"CIplImg::Convert(), Error converting image to 3 channel greyscale\n");
				return false;
			}
		}
		else
		{
			fprintf(pErrorFile,"CIplImg::Convert(), Unsupported no. of channels (this=%d), (src=%d)\n",
								Channels(),src.Channels());
			return false;
		}
	}
	return true;
}

			//Note that the ipl function convert does not accept a const arg,
			//therefore we copy src inside the function
bool CIplImg::ConvertDataType(const AbstractImage &src,bool scaleToRange)
{
	if (this==&src)
		return false;

	if(!src.IsValid()) {
		fprintf(pErrorFile,"CIplImg::ConvertDataType(), image arg invalid)\n");
		return false;
	}

	if(!SameDimensions(src)){
		fprintf(pErrorFile,
"CIplImg::ConvertDataType(), Src dimensions (%d*%d) don't match this (%d*%d)\n",
		src.Width(),src.Height(),Width(),Height());
		return false;
	}

	if(!SameChannels(src)){
		fprintf(pErrorFile,
"CIplImg::ConvertDataType(), Src channels (%d) don't match this (%d)\n",
		src.Channels(),Channels());
		return false;
	}

	if(!IsValid()||SameDataType(src))
		return Copy(src);

	
	CIplImg convertedSrc;

	if(!SameClass(src)){
		if(!convertedSrc.Create(src.Width(),src.Height(),src.Channels(),src.DataType(),src.Data()))
		{
			fprintf(pErrorFile,"CIplImg::ConvertDataType(), Error creating temporary image\n");
			return false;	
		}
	}
	else
		convertedSrc.Copy(src);

	IplImage *pSrcHdr=(IplImage*)convertedSrc.Header();

		//If Dest image is float
	if (m_Img->depth==IPL_DEPTH_32F)
	{
		float min, max;
		if (pSrcHdr->depth==IPL_DEPTH_1U) {min=0; max=1;}
		else if (pSrcHdr->depth==IPL_DEPTH_8U) {min=0; max=(float)pow(2.0f,(int)8)-1;}
		else if (pSrcHdr->depth==IPL_DEPTH_8S) {min=-(float)pow(2.0f,(int)7)+1; max=(float)pow(2.0f,(int)7)-1;}
		else if (pSrcHdr->depth==IPL_DEPTH_16U) {min=0; max=(float)pow(2.0f,(int)16)-1;}
		else if (pSrcHdr->depth==IPL_DEPTH_16S) {min=-(float)pow(2.0f,(int)15)+1; max=(float)pow(2.0f,(int)15)-1;}
		else if (pSrcHdr->depth==IPL_DEPTH_32S) {min=-(float)pow(2.0f,(int)31)+1; max=(float)pow(2.0f,(int)31)-1;}
		else {
			fprintf(pErrorFile,"CIplImg::ConvertDataType(), Unknown input bit depth\n");
			return false;
		}
			//NOTE the resultant fp image will adopt the same range as the input data
			//we have already checked that input and output are not the same datatype
			//arg scale to range 
		iplScaleFP(pSrcHdr,m_Img,min,max);
	}
	else if (pSrcHdr->depth==IPL_DEPTH_32F)
	{
		float min, max;
		
		//if(scaleToRange)
		//{
		//	iplMinMaxFP(pSrcHdr,&min,&max);
		//}
		//else
		//{
				//otherwise we assume that the floating point image describes the same datarange
				//as the other data format
			if(m_Img->depth==IPL_DEPTH_1U) {min=0; max=1;}
			else if(m_Img->depth==IPL_DEPTH_8U) {min=0; max=(float)pow(2.0f,(int)8)-1;}
			else if(m_Img->depth==IPL_DEPTH_8S) {min=-(float)pow(2.0f,(int)7)+1; max=(float)pow(2.0f,(int)7)-1;}
			else if(m_Img->depth==IPL_DEPTH_16U) {min=0; max=(float)pow(2.0f,(int)16)-1;}
			else if(m_Img->depth==IPL_DEPTH_16S) {min=-(float)pow(2.0f,(int)15)+1; max=(float)pow(2.0f,(int)15)-1;}
			else if(m_Img->depth==IPL_DEPTH_32S) {min=-(float)pow(2.0f,(int)31)+1; max=(float)pow(2.0f,(int)31)-1;}
			else {
				fprintf(pErrorFile,"CIplImg::ConvertDataType(), Unknown input bit depth\n");
				return false;
			}
		//}
		iplScaleFP(pSrcHdr,m_Img,min,max);
	}
	else {
		if(scaleToRange)
			iplScale(pSrcHdr,m_Img);
		else
			iplConvert(pSrcHdr,m_Img);
	}
	return true;
}


bool CIplImg::RgbToBgr()
{
	if(!IsValid())
	{
		fprintf(pErrorFile,"CIplImg::RgbToBgr(), this is invalid\n");
		return false;
	}

	if(m_Img->nChannels!=3)
	{
		fprintf(pErrorFile,"CIplImg::RgbToBgr(), this image has %d channels, expecting 3\n",
			m_Img->nChannels);
		return false;
	}

	char channelSeq[4]="BGR";

	*((long*)m_Img->channelSeq)=*((long*)channelSeq);
	
	return true;
}


bool CIplImg::ToGreyscale(const AbstractImage &src)
{
	if (this==&src)
		return false;

	if(!src.IsValid())
	{
		Destroy();
		fprintf(pErrorFile,"CIplImg::ToGreyscale(), Arg 'src' invalid\n");
		return false;
	}

	if(src.Channels()==1){
		if(!Copy(src))
			fprintf(pErrorFile,"CIplImg::ToGreyscale(), Error Copying image\n");
		return false;
	}

	if(src.Channels()!=3)
	{
		fprintf(pErrorFile,
			"CIplImg::ToGreyscale(), Arg 'src' has unsupported no. of channels (%d)\n",
				src.Channels());
		return false;
	}

	CIplImg iplSrc;
	IplImage *pSrcHdr=NULL;

		//if data type unsupported by ipl function needs converting to new IPL format whatever
	if(src.DataType()!=IMG_UCHAR||src.DataType()!=IMG_LONG){
		if(	!iplSrc.Create(src.Width(),src.Height(),src.Channels(),IMG_UCHAR)
				||
			!iplSrc.ConvertDataType(src))
		{
			fprintf(pErrorFile,"CIplImg::ToGreyscale(), Error converting source to requisite IPL format\n");
			return false;
		}
		pSrcHdr=(IplImage*)iplSrc.Header();
	}
	else if(!SameClass(src)){	//if not same class (but datatype ok) copy to IPL
		if(!iplSrc.Copy(src)){
			fprintf(pErrorFile,"CIplImg::ToGreyscale(), Error converting source to IPL format\n");
			return false;
		}
		pSrcHdr=(IplImage*)iplSrc.Header();
	}
	else
		pSrcHdr=(IplImage*)src.Header();

	
	if(!IsValid()||!SameDimensions(src)||!Channels()==1){
		if(!Create(src.Width(),src.Height(),1,src.DataType())){
			fprintf(pErrorFile,"CIplImg::ToGreyscale(), Unable to create greyscale image\n");
			return false;
		}
	}

	IplImage *pDstHdr=NULL;
	int desiredDepth=m_Img->depth;

		//Ipl libraries don't support transformation to 32 bit greyscale images
	if(desiredDepth==IPL_DEPTH_32F||desiredDepth==IPL_DEPTH_32S){
		pDstHdr=iplCreateImageHeader(1,0,IPL_DEPTH_16U,"GRAY","G",IPL_DATA_ORDER_PIXEL,IPL_ORIGIN_TL,
									IPL_ALIGN_QWORD,src.Width(),src.Height(),NULL,NULL,"Image",NULL);
		iplAllocateImage(pDstHdr,0,0);
	}
	else
		pDstHdr=m_Img;

	iplColorToGray(pSrcHdr,pDstHdr);

	if(pDstHdr!=m_Img){
		if(desiredDepth==IPL_DEPTH_32F){
			if(src.DataType()==IMG_UCHAR)
				iplScaleFP(pDstHdr,m_Img,0,255);
			else if(src.DataType()==IMG_LONG)
				iplScaleFP(pDstHdr,m_Img,IMG_LONG_MIN,IMG_LONG_MAX);
			else
				iplScaleFP(pDstHdr,m_Img,0,255); //whatever
		}
		else if(desiredDepth==IPL_DEPTH_32S){
			iplScale(pDstHdr,m_Img);
		}
		else
			fprintf(pErrorFile,"CIplImg::ToGreyscale(), Unsupported depth\n");

		iplDeallocateImage(pDstHdr);
		iplDeallocate(pDstHdr,IPL_IMAGE_HEADER);
	}
	return true;
}

bool CIplImg::To3ChannelGreyscale(const AbstractImage &src)
{
	if (this==&src)
		return false;

	if(!src.IsValid())
	{
		Destroy();
		fprintf(pErrorFile,"CIplImg::ToGreyscale(), Arg src invalid\n");
		return false;
	}

	if(src.Channels()!=1){
		fprintf(pErrorFile,
	"CIplImg::To3ChannelGreyscale(), warning arg channels != 1, dunno how this'll behave\n");
	}

	CIplImg iplSrc;
	IplImage *pSrcHdr;

	if(src.DataType()!=IMG_UCHAR){//||src.DataType()!=IMG_LONG){
		if(	!iplSrc.Create(src.Width(),src.Height(),src.Channels(),IMG_UCHAR)
				||
			!iplSrc.ConvertDataType(src))
		{
			fprintf(pErrorFile,"CIplImg::To3ChannelGreyscale(), Error converting source to requisite IPL format\n");
			return false;
		}
		pSrcHdr=(IplImage*)iplSrc.Header();
	}
	else if(!SameClass(src)){	//if not same class (but datatype ok) copy to IPL
		if(!iplSrc.Copy(src)){
			fprintf(pErrorFile,"CIplImg::To3ChannelGreyscale(), Error converting source to IPL format\n");
			return false;
		}
		pSrcHdr=(IplImage*)iplSrc.Header();
	}
	else
		pSrcHdr=(IplImage*)src.Header();

	
	if(!IsValid()||!SameDimensions(src)||!SameDataType(src)){
		if(!Create(src.Width(),src.Height(),3,src.DataType())){
			fprintf(pErrorFile,"CIplImg::To3ChannelGreyscale(), Unable to create greyscale image\n");
			return false;
		}
	}

	IplImage *pDstHdr=NULL;
	int desiredDepth=m_Img->depth;

	if(desiredDepth!=IPL_DEPTH_8U||desiredDepth!=IPL_DEPTH_16U||desiredDepth!=IPL_DEPTH_32S){
		pDstHdr=iplCreateImageHeader(3,0,IPL_DEPTH_8U,"RGB","BGR",IPL_DATA_ORDER_PIXEL,IPL_ORIGIN_TL,
									IPL_ALIGN_QWORD,src.Width(),src.Height(),NULL,NULL,"Image",NULL);
		iplAllocateImage(pDstHdr,0,0);
	}
	else
		pDstHdr=m_Img;

	//should error check?
	iplGrayToColor(pSrcHdr,pDstHdr,1,1,1);

	if(pDstHdr!=m_Img){
		if(desiredDepth==IPL_DEPTH_32F){
			iplScaleFP(pDstHdr,m_Img,0,255);
		}
		else if(desiredDepth==IPL_DEPTH_32S){
			iplScale(pDstHdr,m_Img);
		}
		else
			fprintf(pErrorFile,"CIplImg::ToGreyscale(), Unsupported depth\n");

		iplDeallocateImage(pDstHdr);
		iplDeallocate(pDstHdr,IPL_IMAGE_HEADER);
	}

	return true;
}


bool CIplImg::ScaleF(const AbstractImage &src,float min,float range)
{
	if(!src.IsValid()){
		fprintf(pErrorFile,"CIplImg::ScaleF(), arg 'src' invalid\n");
		return false;
	}
	if(!IsValid()||DataType()!=IMG_FLOAT){
		Create(src.Width(),src.Height(),src.Channels(),IMG_FLOAT);
	}

	const CIplImg *pSrc;

	pSrc=dynamic_cast<const CIplImg*>(&src);

	CIplImg temp;
	if(!pSrc){
		temp.Copy(src);
		pSrc=&temp;
	}

	if(pSrc->DataType()==IMG_FLOAT)
	{
		float srcMin,srcMax,srcRange;
		iplMinMaxFP(pSrc->m_Img,&srcMin,&srcMax);
		srcRange=srcMax-srcMin;
	
		float scale=range/srcRange;
		float A=min-(scale*srcMin);

		iplMultiplySFP(pSrc->m_Img,m_Img,scale);
		iplAddSFP(m_Img,m_Img,A);

		/*
		int offset=WidthStepBytes()/sizeof(float);

		for (y=0;y<Height();y++){
			_i=(float*)Data();
			_src=(float*)src.Data();

			_i+=y*offset;
			_src+=y*offset;

			for (x=0;x<Width();x++){
				*_i=A+(scale* *_i);
				_i++;_src++;
			}
		}
		*/
	}
	else{
		iplScaleFP((_IplImage*)pSrc->Header(),m_Img,min,min+range);
	}

	return true;
}


bool CIplImg::ScaleF(float min,float range)
{
	if(!IsValid()){
		fprintf(pErrorFile,"CIplImg::ScaleF(), This image invalid\n");
		return false;
	}
	if(DataType()!=IMG_FLOAT){
		fprintf(pErrorFile,
"CIplImg::ScaleF(), Only float data types supported (data label=%d)\n",DataType());
		return false;
	}

	float srcMin,srcMax,srcRange;
	iplMinMaxFP(m_Img,&srcMin,&srcMax);
	srcRange=srcMax-srcMin;
	
	float scale=range/srcRange;
	float A=min-(scale*srcMin);

	iplMultiplySFP(m_Img,m_Img,scale);
	iplAddSFP(m_Img,m_Img,A);
	//MultiplyS(src,&scale);
	//AddS(&A);

	return true;
}

//
//	Get Image attributes
//

inline unsigned int CIplImg::Width() const
{
		//inline const bool IsValid(void) const {if (m_Img==NULL) return false; else return true;}
	return m_Img->width;
}

inline unsigned int CIplImg::WidthStep() const
{
	switch(DataType())
	{
		case IMG_UCHAR:	{
							return m_Img->widthStep/sizeof(unsigned char);
						} break;
		case IMG_LONG:	{
							return m_Img->widthStep/sizeof(long);
						} break;
		case IMG_FLOAT:	{
							return m_Img->widthStep/sizeof(float);
						} break;
		default:		{
							fprintf(pErrorFile,"CIplImg::WidthStep(), unrecognised data type\n");
							return m_Img->widthStep;
						}
	}
}

inline unsigned int CIplImg::WidthStepBytes() const
{
	return m_Img->widthStep;	
}

inline unsigned int CIplImg::Height() const
{
	return m_Img->height;
}

inline unsigned int CIplImg::Channels() const
{
	return m_Img->nChannels;	
}

inline unsigned int CIplImg::Depth() const
{
	unsigned int depth=0;
	CalcDepth(depth,m_Img->depth);
	return depth;	
}

inline short int CIplImg::DataType() const
{
	short int dataType=0;

	IPLDataTypeToImgDataType(dataType,m_Img->depth);

	return dataType;
}

inline const char* CIplImg::ClassID() const
{
	return typeid(this).name();
}

inline void* CIplImg::Header() const{
	return m_Img;
}

	// returns NULL if image invalid
inline void* CIplImg::Data() const{
	return m_Img->imageDataOrigin;	
}
	// returns NULL if coordinate out of bounds or data unallocated
inline void* CIplImg::Pixel2(unsigned int x,unsigned int y) const{
	if(x>=m_Img->width||y>=m_Img->height)
		return NULL;
	
	void *pixel=NULL;
	iplGetPixel(m_Img,x,y,pixel);
	return pixel;
}

inline void CIplImg::Pixel(void *pixel,unsigned int x,unsigned int y) const{
	iplGetPixel(m_Img,x,y,pixel);
}

inline void CIplImg::Pixel(void *pixel,unsigned int index) const{
	
	fprintf(pErrorFile,"CIplImg::Pixel(int index) may be dodgy\n");
	iplGetPixel(m_Img,index,0,pixel);
}

inline void CIplImg::SetPixel(void *pixel,unsigned int x,unsigned int y) const{
	iplPutPixel(m_Img,x,y,pixel);
}

inline void CIplImg::SetPixel(void *pixel,unsigned int index) const{
	
	fprintf(pErrorFile,"CIplImg::Pixel(int index) may be dodgy\n");
	iplPutPixel(m_Img,index,0,pixel);
}


//
// Image format checks
//
inline bool CIplImg::IsValid() const
{
	if(!m_Img||!m_Img->imageData)
		return false;

	return true;
}

inline bool CIplImg::SameDepth(const AbstractImage &img) const
{
	if(Depth()!=img.Depth())
		return false;
	
	return true;
}

inline bool CIplImg::SameChannels(const AbstractImage &img) const
{
	if(Channels()!=img.Channels())
		return false;
	
	return true;
}

inline bool CIplImg::SameDimensions(const AbstractImage &img) const
{
	if(Width()!=img.Width()||Height()!=img.Height())
		return false;

	return true;
}

inline bool CIplImg::SameDataType(const AbstractImage &img) const
{
	if(DataType()==IMG_NULL_DATATYPE
		||img.DataType()==IMG_NULL_DATATYPE
			||DataType()!=img.DataType())
		return false;

	return true;
}

inline bool CIplImg::SameClass(const AbstractImage &img) const
{
	//if(typeid(this)!=typeid(&img))
	if(strcmp(ClassID(),img.ClassID())!=0)
	{
		//fprintf(pErrorFile,"CIplImg::SameClass() %s doesn't equal %s\n",ClassID(),img.ClassID());
		return false;
	}
	
	//fprintf(pErrorFile,"CIplImg::SameClass() %s equals %s\n",ClassID(),img.ClassID());
	
	return true;
}

inline bool CIplImg::SameType(const AbstractImage &img) const
{
	if(!IsValid()||!img.IsValid())
		return false;
	//if(!SameClass(img))
	//	return false;
	if(!SameDimensions(img))
		return false;
	if(!SameDataType(img))
		return false;
	if(!SameChannels(img))
		return false;
	
	return true;
}

bool CIplImg::Add(const AbstractImage &img)
{
	if(!SameType(img)){
		fprintf(pErrorFile,"CIplImg::Add(), Images not same type\n");
		return false;
	}

	const CIplImg *iplImg=dynamic_cast<const CIplImg*>(&img);

	if(!iplImg){
		fprintf(pErrorFile,
"CIplImg::Add(), Addition of abstract images from different classes currently unsupported\n");
				return false;
	}

	iplAdd(iplImg->m_Img,m_Img,m_Img);
	
	return true;
}

bool CIplImg::AddS(const void *pV)
{
	if(!IsValid()){
		fprintf(pErrorFile,"CIplImg::AddS(), this image Invalid\n");
		return false;
	}

	switch(DataType()){
	case IMG_UCHAR:	{
						iplAddS(m_Img,m_Img,*((unsigned char*)pV));
					} break;
	case IMG_LONG:	{
						iplAddS(m_Img,m_Img,*((long*)pV));
					} break;
	case IMG_FLOAT:	{
						iplAddSFP(m_Img,m_Img,*((float*)pV));
					} break;
	default:{
				fprintf(pErrorFile,"CIplImg::AddS(), this image has unsupported data type\n");
				return false;
			}
	}
	return true;
}

bool CIplImg::AddS(const AbstractImage &img,const void *pV)
{
	if(!SameType(img)){
		fprintf(pErrorFile,"CIplImg::AddS() Images not same type\n");
		return false;
	}

	const CIplImg *iplImg=dynamic_cast<const CIplImg*>(&img);

	if(!iplImg){
		fprintf(pErrorFile,
"CIplImg::Add(), addition of abstract images from different classes currently unsupported\n");
			return false;
	}

	switch(DataType()){
	case IMG_UCHAR:	{
						iplAddS(iplImg->m_Img,m_Img,*((unsigned char*)pV));
					} break;
	case IMG_LONG:	{
						iplAddS(iplImg->m_Img,m_Img,*((long*)pV));
					} break;
	case IMG_FLOAT:	{
						iplAddSFP(iplImg->m_Img,m_Img,*((float*)pV));
					} break;
	default:{
				fprintf(pErrorFile,"CIplImg::AddS(), this image has unsupported data type\n");
				return false;
			}
	}
	return true;
}


bool CIplImg::Add(const AbstractImage &img,const AbstractImage &img2)
{
	if(!img.SameType(img2)){
		fprintf(pErrorFile,"CIplImg::Multiply() arg Images not same type\n");
		return false;
	}
	
	if(!IsValid()||!SameType(img)){
		BlankCopy(img);
	}

	const CIplImg *iplImg=dynamic_cast<const CIplImg*>(&img);
	const CIplImg *iplImg2=dynamic_cast<const CIplImg*>(&img2);

	if(!iplImg||!iplImg2){
		fprintf(pErrorFile,
"CIplImg::Add(), addition of abstract images from different classes currently unsupported\n");
			return false;
	}
	
	iplAdd(iplImg->m_Img,iplImg2->m_Img,m_Img);
	
	return true;
}


bool CIplImg::Subtract(const AbstractImage &img)
{
	if(!SameType(img)){
		fprintf(pErrorFile,"CIplImg::Subtract(), Images not same type\n");
		return false;
	}

	const CIplImg *iplImg=dynamic_cast<const CIplImg*>(&img);

	if(!iplImg){
		fprintf(pErrorFile,
"CIplImg::Subtract(), Subtraction of abstract images from different classes currently unsupported\n");
		return false;
	}

	iplSubtract(m_Img,iplImg->m_Img,m_Img);

	return true;
}

bool CIplImg::SubtractS(const void *pV)
{
	if(!IsValid()){
		fprintf(pErrorFile,"CIplImg::SubtractS(), this image Invalid\n");
		return false;
	}

	switch(DataType()){
	case IMG_UCHAR:	{
						iplSubtractS(m_Img,m_Img,*((unsigned char*)pV),false);
					} break;
	case IMG_LONG:	{
						iplSubtractS(m_Img,m_Img,*((long*)pV),false);
					} break;
	case IMG_FLOAT:	{
						iplSubtractSFP(m_Img,m_Img,*((float*)pV),false);
					} break;
	default:{
				fprintf(pErrorFile,"CIplImg::SubtractS(), this image has unsupported data type\n");
				return false;
			}
	}
	return true;
}

bool CIplImg::SubtractS(const AbstractImage &img,const void *pV)
{
	if(!SameType(img)){
		fprintf(pErrorFile,"CIplImg::SubtractS() Images not same type\n");
		return false;
	}
	
	const CIplImg *iplImg=dynamic_cast<const CIplImg*>(&img);

	if(!iplImg){
		fprintf(pErrorFile,
"CIplImg::SubtractS(), subtraction of abstract images from different classes currently unsupported\n");
			return false;
	}

	switch(DataType()){
	case IMG_UCHAR:	{
						iplSubtractS(iplImg->m_Img,m_Img,*((unsigned char*)pV),false);
					} break;
	case IMG_LONG:	{
						iplSubtractS(iplImg->m_Img,m_Img,*((long*)pV),false);
					} break;
	case IMG_FLOAT:	{
						iplSubtractSFP(iplImg->m_Img,m_Img,*((float*)pV),false);
					} break;
	default:{
				fprintf(pErrorFile,"CIplImg::SubtractS(), this image has unsupported data type\n");
				return false;
			}
	}
	return true;
}

bool CIplImg::Subtract(const AbstractImage &img,const AbstractImage &img2)
{
	if(!img.SameType(img2)){
		fprintf(pErrorFile,"CIplImg::Multiply() arg Images not same type\n");
		return false;
	}
	
	if(!IsValid()||!SameType(img)){
		BlankCopy(img);
	}

	const CIplImg *iplImg=dynamic_cast<const CIplImg*>(&img);
	const CIplImg *iplImg2=dynamic_cast<const CIplImg*>(&img2);

	if(!iplImg||!iplImg2){
		fprintf(pErrorFile,
"CIplImg::Subtract(), subtraction of abstract images from different classes currently unsupported\n");
			return false;
	}
	
	iplSubtract(iplImg->m_Img,iplImg2->m_Img,m_Img);
	
	return true;
}

bool CIplImg::Multiply(const AbstractImage &img)
{
	if(!SameType(img)){
		fprintf(pErrorFile,"CIplImg::Multiply(), Images not same type\n");
		return false;
	}

	const CIplImg *iplImg=dynamic_cast<const CIplImg*>(&img);

	if(!iplImg){
		fprintf(pErrorFile,
"CIplImg::Multiply(), multiplication of abstract images from different classes currently unsupported\n");
				return false;
	}

	iplMultiply(iplImg->m_Img,m_Img,m_Img);

	return true;
}

bool CIplImg::MultiplyS(const void *pV)
{
	if(!IsValid()){
		fprintf(pErrorFile,"CIplImg::MultiplyS(), this image Invalid\n");
		return false;
	}

	switch(DataType()){
	case IMG_UCHAR:	{
						iplMultiplyS(m_Img,m_Img,*((unsigned char*)pV));
					} break;
	case IMG_LONG:	{
						iplMultiplyS(m_Img,m_Img,*((long*)pV));
					} break;
	case IMG_FLOAT:	{
						iplMultiplySFP(m_Img,m_Img,*((float*)pV));
					} break;
	default:{
				fprintf(pErrorFile,"CIplImg::MultiplyS(), this image has unsupported data type\n");
				return false;
			}
	}
	return true;
}

bool CIplImg::MultiplyS(const AbstractImage &img,const void *pV)
{
	if(!SameType(img)){
		fprintf(pErrorFile,"CIplImg::MultiplyS() Images not same type\n");
		return false;
	}

	const CIplImg *iplImg=dynamic_cast<const CIplImg*>(&img);

	if(!iplImg){
		fprintf(pErrorFile,
"CIplImg::MultiplyS(), multiplication of abstract images from different classes currently unsupported\n");
			return false;
	}


	switch(DataType()){
	case IMG_UCHAR:	{
						iplMultiplyS(iplImg->m_Img,m_Img,*((unsigned char*)pV));
					} break;
	case IMG_LONG:	{
						iplMultiplyS(iplImg->m_Img,m_Img,*((long*)pV));
					} break;
	case IMG_FLOAT:	{
						iplMultiplySFP(iplImg->m_Img,m_Img,*((float*)pV));
					} break;
	default:{
				fprintf(pErrorFile,"CIplImg::MultiplyS(), this image has unsupported data type\n");
				return false;
			}
	}
	return true;
}

bool CIplImg::Multiply(const AbstractImage &img,const AbstractImage &img2)
{
	if(!img.SameType(img2)){
		fprintf(pErrorFile,"CIplImg::Multiply() arg Images not same type\n");
		return false;
	}
	
	if(!IsValid()||!SameType(img)){
		BlankCopy(img);
	}

	const CIplImg *iplImg=dynamic_cast<const CIplImg*>(&img);
	const CIplImg *iplImg2=dynamic_cast<const CIplImg*>(&img2);

	if(!iplImg||!iplImg2){
		fprintf(pErrorFile,
"CIplImg::Subtract(), multiplication of abstract images from different classes currently unsupported\n");
			return false;
	}
	
	iplMultiply(iplImg->m_Img,iplImg2->m_Img,m_Img);
	
	return true;
}



///////////////////////////////////////////////
//Multiply Accumulate the input into this image
// same as mac1 in the old version
bool CIplImg::MultAcc(const AbstractImage &img,float v)
{
	if(!img.IsValid()){
		fprintf(pErrorFile,"CIplImg::MultAcc() arg image invalid\n");
		return false;
	}
	
	if (img.DataType()!=IMG_FLOAT){
		fprintf(pErrorFile,"CIplImg::MultAcc(), Requires Floating Point Image\n");
		return false;
	}
	
	if (img.Channels()!=1){
		fprintf(pErrorFile,"CIplImg::MultAcc(), Requires single channel Images\n");
		return false;
	}

	if(!SameType(img)){
		BlankCopy(img);
	}

	if(!SameClass(img)){
		fprintf(pErrorFile,"CIplImg::MultAcc(), Conversion from different abstract image formats currently unsupported\n");
		return false;
	}

	int x,y;
	float *pDst=(float*)Data();
	float *pSrc=(float*)img.Data();

	int yoffset=WidthStepBytes()/sizeof(float);
	int pos=0;

	for (y=0;y<Height();y++){
		pos=y*yoffset;

		for (x=0;x<Width();x++){
			pDst[pos]+=pSrc[pos]*v;
			pos++;
		}
	}
	return true;
}



bool CIplImg::Divide(const AbstractImage &img)
{
	if(!SameType(img)){
		fprintf(pErrorFile,"_Img::Divide() Images not same type\n");
		return false;
	}

	unsigned int sz=Width()*Height();


	switch(DataType())
	{
	
	case IMG_UCHAR:{
					unsigned char val,*_src,*_dst,*max;
					max=((unsigned char*)Data())+sz;
					for(_dst=(unsigned char*)Data(),_src=(unsigned char*)img.Data();
						_dst<max;
						++_dst,++_src)
					{
						val=((float)*_dst)/ *_src;
						*_dst=val<0 ? 0 : val;
					}
			   } break;
	case IMG_LONG: {
					long *_src,*_dst,*max;
					max=((long*)Data())+sz;
					for(_dst=(long*)Data(),_src=(long*)img.Data();
						_dst<max;
						++_dst,++_src)
					{
						*_dst/=((float)*_src);
					}
			  } break;
	case IMG_FLOAT: {
					float *_src,*_dst,*max;
					max=((float*)Data())+sz;
					for(_dst=(float*)Data(),_src=(float*)img.Data();
						_dst<max;
						++_dst,++_src)
					{
						*_dst/=*_src;
					}
				} break;
	default:	{
			fprintf(pErrorFile,"CIplImg::Divide(), Data type unrecognised\n");
			return false;
			}
	}

	return true;
}


bool CIplImg::Divide(const AbstractImage &img,float numThresh,float denomThresh)
{
	if(!SameType(img)){
		fprintf(pErrorFile,"CIplImg::Divide() Images not same type\n");
		return false;
	}

	unsigned int sz=Width()*Height();
	
	switch(DataType())
	{
	
	case IMG_UCHAR:{
					unsigned char val,*_src,*_dst,*max;
					max=((unsigned char*)Data())+sz;
					for(_dst=(unsigned char*)Data(),_src=(unsigned char*)img.Data();
						_dst<max;
						++_dst,++_src)
					{
							if(*_dst>numThresh)
							{
								if (*_src>denomThresh){
									val=((float)*_dst)/ *_src;
									*_dst=val<0 ? 0 : val;
								}
								else
									*_dst=255;
							}
							else
								*_dst=0;
					}
			   } break;
	case IMG_LONG: {
					long *_src,*_dst,*max;
					max=((long*)Data())+sz;
					for(_dst=(long*)Data(),_src=(long*)img.Data();
						_dst<max;
						++_dst,++_src)
					{
						if(fabs((float)(*_dst))>numThresh)
						{
							if (fabs((float)(*_src))>denomThresh){
								*_dst=((float)*_dst)/ *_src;
							}
							else
								*_dst=0; //NB this should be max
						}
						else
							*_dst=0;
					}
			  } break;
	case IMG_FLOAT: {
					float *_src,*_dst,*max;
					max=((float*)Data())+sz;
					for(_dst=(float*)Data(),_src=(float*)img.Data();
						_dst<max;
						++_dst,++_src)
					{
						if(fabs(*_dst)>numThresh)
						{
							if (fabs(*_src)>denomThresh){
								*_dst/= *_src;
							}
							else
								*_dst=1.0f/denomThresh; //assuming 1 to be the max for FP
						}
						else
							*_dst=0;
					}
				} break;
	default:	{
					fprintf(pErrorFile,"CIplImg::Divide(), Data type unrecognised\n");
					return false;
				}
	}

	return true;
}

bool CIplImg::TestDivideF(	const AbstractImage &num,const AbstractImage &denom,
							float fNumThresh,float fDenThresh)
{
	if(!num.IsValid()){
		fprintf(pErrorFile,"CIplImg::TestDivideF(), Numerator invalid\n");
		return false;
	}
	
	if(num.DataType()!=IMG_FLOAT){
		fprintf(pErrorFile,"CIplImg::TestDivideF(), Only float data type supported\n");
		return false;
	}
	
	if(num.Channels()!=1){
		fprintf(pErrorFile,"CIplImg::TestDivideF(), Only greyscale images supported\n");
		return false;
	}

	if(!num.SameType(denom)){
		fprintf(pErrorFile,"CIplImg::TestDivideF(), image args 'num' and 'denom' not same type\n");
		return false;
	}
	
	if(!SameType(num)){
		if(!BlankCopy(num)){
		fprintf(pErrorFile,"CIplImg::TestDivideF(), Error creating this image for output\n");
		return false;
		}
	}
	
	int x,y;
	float *srcNum=(float*)num.Data();
	float *srcDen=(float*)denom.Data();
	float *dst=(float*)Data();

	int offset=WidthStepBytes()/sizeof(float);

	for (y=0;y<Height();y++){
	srcNum=(float*)num.Data();
	srcDen=(float*)denom.Data();
	dst=(float*)Data();

	srcNum+=y*offset;
	srcDen+=y*offset;
	dst+=y*offset;

		for (x=0;x<Width();x++){
			if (fabs(*srcNum)>fNumThresh&&fabs(*srcDen)>fDenThresh)
			{
					*dst=*srcNum/ *srcDen;
			}
			else
			{
				*dst=0.0f;
			}
			srcNum++;srcDen++;dst++;
		}
	}


	/*
	for (y=0;y<Height();y++){
	num=(float*)Data();
	den=(float*)ImageDenom.Data();
	res=(float*)ImOutput.Data();

	num+=y*offset;
	den+=y*offset;
	res+=y*offset;

		for (x=0;x<Width();x++){
			if (fabs(*(num))>fNumThresh){
				if (fabs(*(den))>fDenThresh){
					*(res)=*(num)/(*(den));
				}
				else
				{
					*(res)=1.0f/fDenThresh;
				}
			}
			else
			{
				*(res)=0.0f;
			}
			num++;den++;res++;
		}
	}
	*/
	return true;
}


bool CIplImg::TestInvertF(const AbstractImage &src,float fDenThresh)
{
	if(!src.IsValid()){
		fprintf(pErrorFile,"CIplImg::TestInvertF(), src image invalid\n");
		return false;
	}
	if(src.DataType()!=IMG_FLOAT){
		fprintf(pErrorFile,"CIplImg::TestInvertF(), Only float data type supported\n");
		return false;
	}
	if(src.Channels()!=1){
		fprintf(pErrorFile,"CIplImg::TestInvertF(), Only greyscale images supported\n");
		return false;
	}
	if(!SameType(src)){
		if(!BlankCopy(src)){
			fprintf(pErrorFile,"CIplImg::TestInvertF(), Error creating output\n");
			return false;
		}
	}

	int x,y;
	float *den=(float*)src.Data();
	float *res=(float*)Data();

	int offset=WidthStepBytes()/sizeof(float);

	for (y=0;y<Height();y++){
	den=(float*)src.Data();
	res=(float*)Data();

	den+=y*offset;
	res+=y*offset;

		for (x=0;x<Width();x++){
			if (fabs(*den)>fDenThresh){
					*res=1.0f/(*den);
			}
			else
			{
				*res=1.0f/fDenThresh;
			}
			den++;res++;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////
//Used in McGM if val at index of test image < threshold, output image at corresponding
//index set to zero
bool CIplImg::ThreshToZeroF(const AbstractImage &test,float fThresh)
{
	if(this==&test){
		fprintf(pErrorFile,"CIplImg::ThreshToZeroF(), this==test\n");
		return false;
	}

	if(!IsValid()||DataType()!=IMG_FLOAT)
	{
		fprintf(pErrorFile,"CIplImg::ThreshToZeroF(), this image invalid or not float data type\n");
		return false;
	}
	if(!SameType(test))
	{
		fprintf(pErrorFile,"CIplImg::ThreshToZeroF(), this image different type to 'test'\n");
		return false;
	}
	
	int x,y;

	float *_t=(float*)test.Data();
	float *_dst=(float*)Data();

	int tOffset=test.WidthStepBytes()/sizeof(float);
	int dstOffset=WidthStepBytes()/sizeof(float);

	for (y=0;y<Height();y++)
	{
		_t=(float*)test.Data();
		_dst=(float*)Data();

		_t+=y*tOffset;
		_dst+=y*dstOffset;

		for (x=0;x<Width();x++)
		{
			if (fabs(*_t)<fThresh)
					*_dst=0;

			_dst++;_t++;
		}
	}
	return true;
}


bool CIplImg::ThreshToZeroF(const AbstractImage &src,const AbstractImage &test,float fThresh)
{
	if(this==&test||this==&src){
		fprintf(pErrorFile,"CIplImg::ThreshToZeroF(), this==src or this==test\n");
		return false;
	}

	if(!src.IsValid()||src.DataType()!=IMG_FLOAT)
	{
		fprintf(pErrorFile,"CIplImg::ThreshToZeroF(), arg 'src' invalid or not float data type\n");
		return false;
	}
	if(!src.SameType(test))
	{
		fprintf(pErrorFile,"CIplImg::ThreshToZeroF(), arg 'src' different type to 'test'\n");
		return false;
	}
	if(!IsValid()||!SameType(src)){
		if(!BlankCopy(src)){
			fprintf(pErrorFile,"CIplImg::ThreshToZeroF(), error creating this image\n");	
		}
	}

	int x,y;

	float *_t=(float*)test.Data();
	float *_src=(float*)src.Data();
	float *_dst=(float*)Data();

	int tOffset=test.WidthStepBytes()/sizeof(float);
	int srcOffset=src.WidthStepBytes()/sizeof(float);
	int dstOffset=WidthStepBytes()/sizeof(float);

	for (y=0;y<Height();y++)
	{
		_t=(float*)test.Data();
		_src=(float*)src.Data();
		_dst=(float*)Data();

		_t+=y*tOffset;
		_src+=y*srcOffset;
		_dst+=y*dstOffset;

		for (x=0;x<Width();x++)
		{
			if (fabs(*_t)<fThresh)
					*_dst=0;
			else
				*_dst=*_src;

			_src++;_dst++;_t++;
		}
	}
	return true;
}


///////////////////////////////////////////////
//Misc Arithmetic
///////////////////////////////////////////////

bool CIplImg::Sq()
{
	if(!IsValid()){
		fprintf(pErrorFile,"CIplImg::Sq() this image invalid\n");
		return false;
	}
	
	iplSquare(m_Img,m_Img);
	return true;
}

bool CIplImg::Sq(const AbstractImage &img)
{

	if(!img.IsValid()){
		fprintf(pErrorFile,"CIplImg::Sq(), arg img invalid\n");
		return false;
	}

	if(!SameType(img)){
		BlankCopy(img);
	}
	
	const CIplImg *pIpl=dynamic_cast<const CIplImg*>(&img);

	if(!pIpl){
		fprintf(pErrorFile,"CIplImg::Sq(), Currently does not support conversions from other AbstractImage formats\n");
		return false;
	}

	iplSquare(pIpl->m_Img,m_Img);
	
	return true;
}


bool CIplImg::Sqrt() {
	
	if(!IsValid()){
		fprintf(pErrorFile,"CIplImg::Sqrt() this image invalid\n");
		return false;
	}

	if (m_Img->depth!=IPL_DEPTH_32F){
		fprintf(pErrorFile,"CIplImg::Sqrt(), requires Floating Point Image\n");
		return false;
	}
	
	if (Channels()!=1){
		fprintf(pErrorFile,"CIplImg::Sqrt(), requires single channel Image\n");
		return false;
	}
	
	int x,y;
	float *pim=(float*)m_Img->imageData;
	int offset=m_Img->widthStep/sizeof(float);

	for (y=0;y<Height();y++){
		pim=(float*)Data();
		pim+=y*offset;
		for (x=0;x<Width();x++){
			if(*pim<0){
				fprintf(pErrorFile,"CIplImg::Sqrt(), negative value (%f)\n",*pim);
				*pim=0;
			}
			else
				*(pim)=(float)sqrt(*(pim));
			pim++;
		}
	}

	return true;
}

bool CIplImg::Sqrt(const AbstractImage &img) {
	
	if(!img.IsValid()){
		fprintf(pErrorFile,"CIplImg::Sqrt(), arg img invalid\n");
		return false;
	}

	if(img.DataType()!=IMG_FLOAT){
		fprintf(pErrorFile,"CIplImg::Sqrt(), requires Floating Point Image\n");
		return false;
	}
	
	if (Channels()!=1){
		fprintf(pErrorFile,"CIplImg::Sqrt(), requires single channel Image\n");
		return false;
	}

	if(!SameType(img)){
		BlankCopy(img);
	}

	int x,y;
	float *_dst=(float*)m_Img->imageData;
	float *_src=(float*)img.Data();
	
	int iplOffset=m_Img->widthStep/sizeof(float);
	int imgOffset=img.WidthStepBytes()/sizeof(float);

	for (y=0;y<Height();y++){
		_dst=(float*)Data();
		_dst+=y*iplOffset;

		_src=(float*)img.Data();
		_src+=y*imgOffset;

		for (x=0;x<Width();x++){
			if(*_src<0){
				fprintf(pErrorFile,"CIplImg::Sqrt(), negative value (%f)\n",*_src);
				*_dst=0;
			}
			else
				*_dst=(float)sqrt(*_src);
			
			++_src; ++_dst;
		}
	}

	return true;
}


bool CIplImg::Abs()
{
	if(!IsValid()){
		fprintf(pErrorFile,"CIplImg::Abs this image invalid\n");
		return false;
	}
		
	iplAbs(m_Img,m_Img);
	
	return true;
}

bool CIplImg::Abs(const AbstractImage &img)
{

	if(!img.IsValid()){
		fprintf(pErrorFile,"CIplImg::Abs(), arg img invalid\n");
		return false;
	}

	if(!SameType(img)){
		BlankCopy(img);
	}
	
	const CIplImg *pIpl=dynamic_cast<const CIplImg*>(&img);

	if(!pIpl){
		fprintf(pErrorFile,"CIplImg::Sq(), Currently does not support conversions from other AbstractImage formats\n");
		return false;
	}

	iplAbs(pIpl->m_Img,m_Img);
	
	return true;
}


bool CIplImg::ATan2(const AbstractImage &XImg,const AbstractImage &YImg)
{
	if(!XImg.IsValid()){
		fprintf(pErrorFile,"CIplImg::ATan2(), arg XImg invalid\n");
		return false;
	}
	
	if (XImg.DataType()!=IMG_FLOAT){
		fprintf(pErrorFile,"CIplImg::ATan2(), Requires Floating Point Image\n");
		return false;
	}

	if (Channels()!=1){
		fprintf(pErrorFile,"CIplImg::ATan2(), Requires single channel Images\n");
		return false;
	}
	if (XImg.Channels()!=1){
		fprintf(pErrorFile,"CIplImg::ATan2(), Requires single channel Images\n");
		return false;
	}

	if (!XImg.SameType(YImg)){
		fprintf(pErrorFile,"CIplImg::ATan2(), Images not compatible!\n");
		return false;
	}

	if(!SameType(XImg)){
		BlankCopy(XImg);
	}


	int x,y;
	float *pim=(float*)Data();
	float *pimx=(float*)XImg.Data();
	float *pimy=(float*)YImg.Data();
	int offset=WidthStepBytes()/sizeof(float);
	int offsetx=XImg.WidthStepBytes()/sizeof(float);
	int offsety=YImg.WidthStepBytes()/sizeof(float);

	for (y=0;y<Height();y++){
		pim=(float*)GetpData();
		pimx=(float*)XImg.Data();
		pimy=(float*)YImg.Data();
		pim+=y*offset;
		pimx+=y*offsetx;
		pimy+=y*offsety;
		for (x=0;x<Width();x++){
			if (*pimx==0 && *pimy==0)
				*(pim)=0.0f;
			else
				*(pim)=(float)atan2(*(pimx),*(pimy));
			pim++;pimx++;pimy++;
		}
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// CONVOLUTION

bool CIplImg::ErrorCheckConvolve(const AbstractImage &src,const AbstractFilter &f)
{
	if(!src.IsValid()){
		fprintf(pErrorFile,"CIplImg::Convolve(), Arg 'src' invalid\n");
		return false;
	}

	if(!f.xyIsValid()){
		fprintf(pErrorFile,"CIplImg::Convolve(), Filter invalid\n");
		return false;
	}

	return true;
}

bool CIplImg::Convolve(const AbstractImage &src,const AbstractFilter &f)
{
	if(!ErrorCheckConvolve(src,f))
		return false;
	
	CIplImg tempsrc;
	const CIplImg *pSrc;
	
	if(src.DataType()!=f.DataType()||!SameClass(src)){
		tempsrc.Create(src.Width(),src.Height(),src.Channels(),f.DataType());
		tempsrc.ConvertDataType(src);
		pSrc=&tempsrc;
	}
	else
		pSrc=dynamic_cast<const CIplImg*>(&src);

	if(!pSrc){
		fprintf(pErrorFile,"CIplImg::Convolve(), Error in dynamic cast to CIplImg\n");
		return false;
	}

	CIplFilter tempFilt;
	const CIplFilter *pIplF=dynamic_cast<const CIplFilter*>(&f);

	if(!pIplF){
		if(!tempFilt.Copy(f)){
			fprintf(pErrorFile,"CIplImg::Convolve(), Error copying filter\n");
			return false;
		}
		pIplF=&tempFilt;
	}

	if(!IsValid()||!SameType(*pSrc))
		BlankCopy(*pSrc);
	

	if(f.IsSep()){
		
		switch(DataType())
		{
		case IMG_LONG:	{
							ConvolveSep2Di(*pSrc,*pIplF);
						}break;
		case IMG_FLOAT:	{
							ConvolveSep2Df(*pSrc,*pIplF);
						} break;
		default:
			fprintf(pErrorFile,"CIplImg::Convolve(), Invalid data type (%d)\n",f.DataType());
			return false;
		}
	}
	else{
		switch(f.DataType())
		{
			case IMG_LONG:{
							Convolve2Di(*pSrc,*pIplF);
				   } break;
			case IMG_FLOAT:{
							Convolve2Df(*pSrc,*pIplF);
				   } break;
			default:
			fprintf(pErrorFile,"CIplImg::Convolve(), Invalid data type (%d)\n",f.DataType());
			return false;
		}
	}
	return true;
}

bool CIplImg::ConvolveSep2Df(const CIplImg &src,const CIplFilter &f)
{
	iplConvolveSep2DFP(src.m_Img,m_Img,(IplConvKernelFP*)f.iplXFilter(),(IplConvKernelFP*)f.iplYFilter());
	return true;
}

bool CIplImg::ConvolveSep2Di(const CIplImg &src,const CIplFilter &f)
{
	iplConvolveSep2D(src.m_Img,m_Img,(IplConvKernel*)f.iplXFilter(),(IplConvKernel*)f.iplYFilter());
	return true;
}

bool CIplImg::Convolve2Df(const CIplImg &src,const CIplFilter &f)
{
	IplConvKernelFP *k=(IplConvKernelFP*)f.iplFilter();
	iplConvolve2DFP(src.m_Img,m_Img,&k,1,IPL_SUM);
	return true;
}
bool CIplImg::Convolve2Di(const CIplImg &src,const CIplFilter &f)
{	
	IplConvKernel *k=(IplConvKernel*)f.iplFilter();
	iplConvolve2D(src.m_Img,m_Img,&k,1,IPL_SUM);
	return true;
}

bool CIplImg::Blur(unsigned int xSize,unsigned int ySize)
{
	if(!IsValid()){
		fprintf(pErrorFile,"CIplImg::Blur(), this img invalid\n");
		return false;
	}
	if ((xSize<2&&ySize<2)
			||xSize%2==0||ySize%2==0){
		fprintf(pErrorFile,
"CIplImg::Blur(), kernel arg sizes (%d,%d) incompatible, expecting odd no.s, one of which >2\n",
			xSize,ySize);
		return false;
	}
	
	int xax=(int)((xSize-1)/2);
	int yax=(int)((ySize-1)/2);
	iplBlur(m_Img,m_Img,xSize,ySize,xax,yax);
	return true;
}

bool CIplImg::BlurF(unsigned int size)
{
	if(!IsValid()){
		fprintf(pErrorFile,"CIplImg::Blur(), this img invalid\n");
		return false;
	}
	if (size<2||size%2==0){
		fprintf(pErrorFile,"CIplImg::Blur(), arg size (%d) incompatible, expecting odd no. >2\n",size);
		return false;
	}
	int ax=(int)((size-1)/2);
	iplBlur(m_Img,m_Img,size,size,ax,ax);
	return true;
}


bool CIplImg::ErrorCheckTConvolve(	const AbstractImage **srcs,
									unsigned int sz,
									const AbstractFilter &f)
{
	if(!f.tIsValid()){
		fprintf(pErrorFile,"CIplImg::ErrorCheckTConvolve(), Temporal filter invalid\n");
		return false;
	}

	if(f.tDataType()!=IMG_FLOAT){
		fprintf(pErrorFile,
"CIplImg::ErrorCheckTConvolve, Filter not float, currently only floating point filtering supported\n");
		return false;
	}
		
	if(sz<f.tSize()){
		fprintf(pErrorFile,"CIplImg::ErrorCheckTConvolve(), Arg sz (%d) too small for temporal filter (%d)\n",
					sz,f.tSize());
		return false;
	}	

	if(!srcs){
		fprintf(pErrorFile,"CIplImg::ErrorCheckTConvolve(), Arg srcs=NULL\n");
		return false;
	}
	
	bool first=true;
	for(const AbstractImage **i=srcs;i<srcs+sz;++i){
		
		if(!*i){
			fprintf(pErrorFile,"CIplImg::ErrorCheckTConvolve(), Arg *srcs=NULL\n");
			return false;
		}

		if(!first){
			if(!(*i)->SameType(**srcs)){
				fprintf(pErrorFile,"CIplImg::ErrorCheckTConvolve(), Srcs have differing image formats\n");
				return false;
			}
		}
		
		if(!(*i)->SameClass(*this)){
				fprintf(pErrorFile,"CIplImg::ErrorCheckTConvolve(), Src class type currently unsupported\n");
				return false;
		}
		
		if(!(*i)->IsValid()){
			fprintf(pErrorFile,"CIplImg::ErrorCheckTConvolve, Invalid src\n");
			return false;
		}

		if((*i)->DataType()!=f.tDataType()){
			fprintf(pErrorFile,"CIplImg::ErrorCheckTConvolve, Src datatype (%d) does not match filter (%d)\n",
								(*i)->DataType(),f.DataType());
			return false;
		}
		first=false;
	}

	return true;
}

bool CIplImg::TConvolve(	const AbstractImage **srcs,
							unsigned int sz,
							const AbstractFilter &f)
{
	if(!ErrorCheckTConvolve(srcs,sz,f)){
		fprintf(pErrorFile,"CIplImg::TConvolve(), incorrect arguments, quitting\n");
		return false;
	}

	if(!IsValid()||!SameType(**srcs)){
		if(!BlankCopy(**srcs)){
			fprintf(pErrorFile,"CIplImg::TConvolve(), Error recreating this image\n");
		}
	}
	
	//this doesn't work
	//const CIplImg **ppIplSrcs=dynamic_cast<const CIplImg**>(srcs);
	//if(!ppIplSrcs){
	//	fprintf(pErrorFile,"CIplImg::TConvolve(), Error casting AbstractImage to CIplImgs\n");
	//	return false;
	//}

	void **ppIplSrcs=(void**)srcs;

	const CIplFilter *pIplF=dynamic_cast<const CIplFilter *> (&f);

	CIplFilter temp;
	if(!pIplF){
		if(!temp.Copy(f)){
			fprintf(pErrorFile,"CIplImg::TConvolve(), Error casting AbstractFilter to CIplFilter\n");
			return false;
		}
		pIplF=&temp;
	}
	
	TConvolveIPL((const CIplImg**)ppIplSrcs,sz,(const CIplFilter)*pIplF);

	return true;
}

//Convolve a sequence of images with temporal filter
bool CIplImg::TConvolveIPL(const CIplImg **srcs,unsigned int sz,
							const CIplFilter &f)
{
	unsigned int tKernelSz=f.tSize();

	//Floating point filtering operation
	if ((*(srcs))->m_Img->depth==IPL_DEPTH_32F&&m_Img->depth==IPL_DEPTH_32F){
		//Make Temp Img
		IplImage *temp=iplCloneImage((*srcs)->m_Img);

		float *tKernel=(float*) f.TFilter();
		
		//First Value
		iplMultiplySFP(srcs[0]->m_Img,m_Img,*tKernel);

		for (int t=1;t<tKernelSz;t++){	//tap 0 is always zero so start at 1
			if (fabs(tKernel[t])>0.0001){
				iplMultiplySFP(srcs[tKernelSz-t]->m_Img,temp,tKernel[t]);
				
				//(const_cast <CIplImg**> (srcs))[tKernelSz-t]->WriteBMP(cfnm);
				iplAdd(temp,m_Img,m_Img);
			}
		}
		iplDeallocate(temp,IPL_IMAGE_ALL);
		return true;
	}
	else{//Integer operation
		//Make Temp Img 
		IplImage *temp=iplCloneImage(m_Img);
		long *tKernel=(long*) f.TFilter();
		
		//First Value
		iplMultiplyS(srcs[0]->m_Img,m_Img,(long)(*tKernel));
		for (long t=1;t<tKernelSz;t++){	//tap 0 is always zero so start at 1 
			iplMultiplyS(srcs[tKernelSz-t]->m_Img,temp,(long)tKernel[t]);
				iplAdd(temp,m_Img,m_Img);
		}
		iplDeallocate(temp,IPL_IMAGE_ALL);
		return true;
	}
	return false;
}



//////////////////////////////////////////////////////////////////////
//Query the IPL DLL for the library version
/*
void CIplImg::GetIplVersion(CString *Info)
{
	if (Info==NULL){return;}
	const IPLLibVersion *m_LibVersion=iplGetLibVersion();//The IPL Library Version
	CString Tmp;
	Info->Format("Intel Image Processing Library Info\n\n");
	Tmp.Format("Ipl DLL:	%s  \n",m_LibVersion->Name);*(Info)+=Tmp;
	Tmp.Format("Ipl DLL:	%s  \n",m_LibVersion->Version);*(Info)+=Tmp;
	Tmp.Format("\nMajor:	%d\nMinor:	%d \n",m_LibVersion->major,m_LibVersion->minor);*(Info)+=Tmp;
	Tmp.Format("Build:	%d \n",m_LibVersion->build);*(Info)+=Tmp;
	Tmp.Format("Date:	%s \n",m_LibVersion->BuildDate);*(Info)+=Tmp;
}
*/

//Return the version number of this class
//int CIplImg::GetVersion(void)
//{
//	return (VERSION);
//}
//////////////////////////////////////////////
//OnDraw(CDC *pDC)
//
//Draws the IPL image to a device context.  (Only 8-bit depth images!)
//
//Notes:
//This function should be called by the appropriate OnDraw() function
//from the calling document view or Dialog box.  It will initialise itself first
//the time it is called and so will be slightly slower first time. 
//The function creates an appropriate BitmapInfo header (m_Bmi) valid 
//for the current IPL image and uses StretchDIBits to render. 
//
//If a CRect is supplied, stretching will occur on the current DC to the
//size and position specified.
/*
void CIplImg::OnDraw(CDC *pDC, const CRect *rectDestSize)
{
	if (!IsValid()){fprintf(pErrorFile,"CIplImg: Draw() - invalid image\n"); return;}
	if (pDC==NULL){fprintf(pErrorFile,"CIplImg: Draw() - invalid device context: calling OnDrawExternal()\n"); OnDrawExternal(); return;}
	if (m_ImgDisp==NULL){
		if (!MakeViewableImage()) {
			fprintf(pErrorFile,"CIplImg: Draw() - couldn't make a viewable image\n");
			return;
		}
	}

	//Makes colour copying work properly
	pDC->SetStretchBltMode(COLORONCOLOR);
	
	ASSERT(m_Bmi!=NULL);
	
	if (rectDestSize==NULL){//if no rect supplied, just blit to top left corner of DC
		CRect rect;
		pDC->GetWindow()->GetClientRect(&rect);
		SetDIBitsToDevice(	pDC->m_hDC,
			rect.left, rect.top, m_ImgDisp->width, m_ImgDisp->height,
			0, 0, 0, abs(m_ImgDisp->height),
			m_ImgDisp->imageData, (BITMAPINFO*)m_Bmi, DIB_RGB_COLORS);
	}
	else{//if rect supplied, use its values
		CRect rect = *rectDestSize;
		if (rect.Width()!=Width() || rect.Height()!=Height()){
			StretchDIBits(	pDC->m_hDC,
				rect.left, rect.top, rect.Width(), rect.Height(),
				0, 0, m_ImgDisp->width, m_ImgDisp->height,
				m_ImgDisp->imageData, (BITMAPINFO*)m_Bmi, DIB_RGB_COLORS, SRCCOPY);
		}
		else
		{
			SetDIBitsToDevice(	pDC->m_hDC,
				rect.left, rect.top, m_ImgDisp->width, m_ImgDisp->height,
				0, 0, 0, abs(m_ImgDisp->height),
				m_ImgDisp->imageData, (BITMAPINFO*)m_Bmi, DIB_RGB_COLORS);
		}
	}
}

void CIplImg::OnDrawExternal()
{
	HANDLE hMapFile;

	SECURITY_ATTRIBUTES sa;
	SECURITY_DESCRIPTOR sd;

	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = &sd;
	if(!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION)) fprintf(pErrorFile,"failed to initialise security descriptor\n");
	if(!SetSecurityDescriptorDacl(&sd, TRUE, (PACL)NULL, FALSE)) fprintf(pErrorFile,"failed to set security descriptor\n");

	hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE,	// handle to file- invalid in this case
		&sa,										
		PAGE_READWRITE,				
		0,												
		m_Img->nSize+m_Img->imageSize,									
		"IPLFileMappingObject");						

	if (hMapFile == NULL) {
		fprintf(pErrorFile,"Could not create file-mapping object\n");
		return;
	}

	LPVOID lpMapAddress;
	lpMapAddress = MapViewOfFile(hMapFile, // Handle to mapping object. 
		FILE_MAP_ALL_ACCESS,               // Read/write permission 
		0,                                 // Max. object size. 
		0,                                 // Size of hFile. 
		0);                                // Map entire file. 

	if (lpMapAddress == NULL) 
	{ 
		fprintf(pErrorFile,"Could not map view of file\n");
		return;
	}

	memcpy(lpMapAddress, m_Img, m_Img->nSize);
	memcpy((BYTE *)lpMapAddress+m_Img->nSize, m_Img->imageData, m_Img->imageSize);

	// call the app...
	PROCESS_INFORMATION procinfo;
	STARTUPINFO startinfo = {0};
	startinfo.cb= sizeof(STARTUPINFO);
	startinfo.dwFlags = STARTF_USESHOWWINDOW;
	startinfo.wShowWindow = SW_HIDE;

	char strCmdLine [256] = IPL_VIEWER_NAME;

	BOOL bProcessCreated = CreateProcess(
		NULL,        // pointer to name of executable module
		strCmdLine,       // pointer to command line string
		NULL,        // pointer to process security attributes
		NULL,        // pointer to thread security attributes
		FALSE,        // handle inheritance flag
		0,         // creation flags
		NULL,        // pointer to new environment block
		NULL,        // pointer to current directory name
		(LPSTARTUPINFO)&startinfo,   // pointer to STARTUPINFO
		(LPPROCESS_INFORMATION)&procinfo  // pointer to PROCESS_INFORMATION
	);

	// wait for viewer to pick up before closing mapping
	if (bProcessCreated) {
		WaitForInputIdle(procinfo.hProcess, 1000);
	}
	else {
		fprintf(pErrorFile,"failed to create process\n");
	}

	// clean up
	if (!UnmapViewOfFile(lpMapAddress)) { 
		fprintf(pErrorFile,"Could not unmap view of file\n");
	}
	CloseHandle(hMapFile);
}
*/

/*
//////////////////////////////////////////////////////////////////////
// Viewable Image generation
//////////////////////////////////////////////////////////////////////
bool CIplImg::MakeViewableImage(float min, float max)
{
	if (!IsValid()) return false;
	if (min==max){fprintf(pErrorFile,"***Error:MakeViewableImage(min,max):Min==Max\n");return false;}
	//Allocate Bitmapinfo and palette mem
	if (m_Bmi) delete [] m_Bmi;

	long headerlength = sizeof(BITMAPINFOHEADER);
	if (m_Img->nChannels == 1) headerlength += 256*(sizeof(RGBQUAD));
	m_Bmi = new BYTE[headerlength];
	BITMAPINFOHEADER *bmih=(BITMAPINFOHEADER *)m_Bmi;
	bmih->biSize=sizeof(BITMAPINFOHEADER);
	bmih->biWidth=(long)(m_Img->width);
	bmih->biHeight=-(long)(m_Img->height); //Negative since layout is top-to-bottom
	bmih->biPlanes=1;bmih->biBitCount=8*m_Img->nChannels;
	bmih->biCompression=0;bmih->biSizeImage=0;
	bmih->biXPelsPerMeter=2000L;bmih->biYPelsPerMeter=2000L;
	bmih->biClrUsed=0;bmih->biClrImportant=0;
	if (m_Img->nChannels == 1) {
		//Create a greyscale Palette
		BYTE *cols=m_Bmi+sizeof(BITMAPINFOHEADER);
		for (int c=0;c<256;c++){*(cols++)=c;*(cols++)=c;*(cols++)=c; cols++;}
	}

	if (m_Img->depth == IPL_DEPTH_8U) {
		// no need to create anything new- m_Img is viewable already
//		if (m_ImgDisp) {
			// destroy viewable image
//			iplDeallocate(m_ImgDisp,IPL_IMAGE_ALL);
//			m_ImgDisp =NULL;
//		}
	// knocked out the previous because an image with viewable image equal to itself
	// is effectively instructed to delete itself
		m_ImgDisp = m_Img;
		return true;
	}

	// need to destroy viewable image if it doesn't match characteristics of image
	if (m_ImgDisp) {
		BOOL bDiff = FALSE;
		if (m_ImgDisp->nChannels != m_Img->nChannels) bDiff = TRUE;
		if (m_ImgDisp->width != Width()) bDiff = TRUE;
		if (m_ImgDisp->height != Height()) bDiff = TRUE;

		if (bDiff) {
			// destroy viewable image
			iplDeallocate(m_ImgDisp,IPL_IMAGE_ALL);
			m_ImgDisp =NULL;
		}
	}

	if (m_ImgDisp==NULL) {
		// Generate an 8-bit version of m_Img's header
		m_ImgDisp=iplCreateImageHeader(m_Img->nChannels,
			m_Img->alphaChannel, IPL_DEPTH_8U, m_Img->colorModel,
			m_Img->channelSeq, m_Img->dataOrder, m_Img->origin, m_Img->align,
			m_Img->width, m_Img->height, m_Img->roi, m_Img->maskROI,
			"Image", m_Img->tileInfo);

		//	Allocate IPL Image Buffer of size specified in Header and clear it
		iplAllocateImage(m_ImgDisp,1,0);
	}
	if (m_Img->depth == IPL_DEPTH_32F) iplScaleFP(m_Img, m_ImgDisp, min, max);
	else iplScale(m_Img, m_ImgDisp);
	return true;
}

// Only useful for float images: scales the viewable image between min and max vals
bool CIplImg::MakeFullRangeViewableImage()
{
	if (!IsValid()) return false;
	if (m_Img->depth != IPL_DEPTH_32F) {
		fprintf(pErrorFile,"MakeFullRangeViewableImage() - only rescales 32F images.\nCalling MakeViewableImage() instead\n");
		return MakeViewableImage();
	}
	else {
		float min, max;
		iplMinMaxFP(m_Img, &min, &max);
		fprintf(stdout,"min=%f, max=%f\n", min, max);
		return MakeViewableImage(min, max);
	}
}
*/
/////////////////////////////////////////////////////////////////////
// Image Creation functions.  Create a blank image of specified type
bool CIplImg::Create8U(unsigned int Width, unsigned int Height,const void *data)
{
	return Create(Width,Height,data,IPL_DEPTH_8U,1);
}
bool CIplImg::Create16U(unsigned int Width, unsigned int Height,const void *data)
{
	return Create(Width,Height,data,IPL_DEPTH_16U,1);
}
bool CIplImg::Create16S(unsigned int Width, unsigned int Height,const void *data)
{
	return Create(Width,Height,data,IPL_DEPTH_16S,1);
}
bool CIplImg::Create32S(unsigned int Width, unsigned int Height,const void *data)
{
	return Create(Width,Height,data,IPL_DEPTH_32S,1);
}
bool CIplImg::Create32F(unsigned int Width, unsigned int Height,const void *data)
{
	return Create(Width,Height,data,IPL_DEPTH_32F,1);
}
bool CIplImg::CreateRGB(unsigned int Width, unsigned int Height,const void *data)
{
	return Create(Width,Height,data,IPL_DEPTH_8U,3);
}
bool CIplImg::CreateRGBFP(unsigned int Width, unsigned int Height,const void *data)
{
	return Create(Width,Height,data,IPL_DEPTH_32F,3);
}


//////////////////////////////////////////////////////////////////////
//Create(unsigned int Width, unsigned int Height,const BYTE *data,int Depth,int Planes)
//
//Private Creation Routine called by CreateX (where X is 8U,16U,16S,32S,32F,RGB)
// 
//Creates a new IPL Image Object and allocate all image data
//If data is not NULL then image will be initialised from supplied data pointer
// 
bool CIplImg::Create(unsigned int Width, unsigned int Height,
					   const void *data,int Depth,int Planes)
{

//	If already have an image then destroy it

	//TO DO CHECK TO SEE IF SAME TYPE, in which case destruction is unnecessary

	if (IsValid()){Destroy();}
	//ASSERT (Planes==1||Planes==3);
	if(!(Planes==1||Planes==3))
		return false;

	//	Allocate IPL Image Header for GreyScale
	if (Planes==1){
		m_Img=iplCreateImageHeader(1,0,Depth,"GRAY","G",IPL_DATA_ORDER_PIXEL,IPL_ORIGIN_TL,
			IPL_ALIGN_QWORD,Width,Height,NULL,NULL,"Image",NULL);
		
	}
	//	Allocate IPL Image Header for 24 bit RGB
	if (Planes==3){
		m_Img=iplCreateImageHeader(3,0,Depth,"RGB","BGR",IPL_DATA_ORDER_PIXEL,IPL_ORIGIN_TL,
			IPL_ALIGN_QWORD,Width,Height,NULL,NULL,"Image",NULL);
	}
	//Check we allocated header OK
	if (m_Img==NULL){
		fprintf(pErrorFile,"Could not Create Ipl Image Header!",MB_OK);
		return false;
	}

//	Allocate IPL Image Buffer of size specified in Header and clear it
//  IPLAllocate Image creates an image which is padded to be QWORD aligned
	if (Depth==IPL_DEPTH_32F){iplAllocateImageFP(m_Img,1,0);}
	else{iplAllocateImage(m_Img,1,0);}

//	Copy data from pointer in Input if non-zero
	unsigned char Size=1;
	if (data!=NULL){
		switch(Depth)
		{
		case IPL_DEPTH_8U:
			Size=1;break;
		case IPL_DEPTH_16U:
			Size=2;break;
		case IPL_DEPTH_16S:
			Size=2;break;
		case IPL_DEPTH_32S:
			Size=4;break;
		case IPL_DEPTH_32F:
			Size=4;break;
		}
		memcpy(m_Img->imageData,data,Width*Height*Size*Planes);
	}

	return true;
}


//////////////////////////////////////////////////////////////////////
// Viewable Image generation
//////////////////////////////////////////////////////////////////////
bool CIplImg::MakeViewableImage(float min,float max)
{
	if (!IsValid()){ 
		fprintf(pErrorFile,"CIplImg::MakeViewableImage(min,max), this image not valid\n");
		return false;
	}
		
	if (min>=max){
		fprintf(pErrorFile,"CIplImg::MakeViewableImage(min,max):Min(%f)>=Max(%f)\n",min,max);
		return false;
	}
		
		// need to destroy viewable image if it doesn't match characteristics of image
	if (m_ImgDisp){
		BOOL bDiff = FALSE;
		if (m_ImgDisp->nChannels!=m_Img->nChannels) bDiff=TRUE;
		if (m_ImgDisp->width != Width()) bDiff=TRUE;
		if (m_ImgDisp->height != Height()) bDiff=TRUE;

		if (bDiff) {
				// destroy viewable image
			iplDeallocate(m_ImgDisp,IPL_IMAGE_ALL);
			m_ImgDisp =NULL;

			if (m_Bmi){
				delete [] m_Bmi;
				m_Bmi=NULL;
				
			}
		}
	}

	if(!m_Bmi){
		long headerlength = sizeof(BITMAPINFOHEADER);
		if (m_Img->nChannels == 1) headerlength += 256*(sizeof(RGBQUAD));

		m_Bmi = new BYTE[headerlength];
		BITMAPINFOHEADER *bmih=(BITMAPINFOHEADER *)m_Bmi;
		bmih->biSize=sizeof(BITMAPINFOHEADER);
		bmih->biWidth=(long)(m_Img->width);
		bmih->biHeight=-(long)(m_Img->height); //Negative since layout is top-to-bottom
		bmih->biPlanes=1;bmih->biBitCount=8*m_Img->nChannels;
		bmih->biCompression=0;bmih->biSizeImage=0;
		bmih->biXPelsPerMeter=2000L;bmih->biYPelsPerMeter=2000L;
		bmih->biClrUsed=0;bmih->biClrImportant=0;

		if (m_Img->nChannels == 1) {
				//Create a greyscale Palette
			BYTE *cols=m_Bmi+sizeof(BITMAPINFOHEADER);
			for (int c=0;c<256;c++){*(cols++)=c;*(cols++)=c;*(cols++)=c; *(cols++)=0;}
		}
	}

	if (m_Img->depth==IPL_DEPTH_8U) {
		m_ImgDisp=m_Img;
		return true;
	}

	if (!m_ImgDisp) {
		// Generate an 8-bit version of m_Img's header
		m_ImgDisp=iplCreateImageHeader(m_Img->nChannels,
							m_Img->alphaChannel, IPL_DEPTH_8U, m_Img->colorModel,
							m_Img->channelSeq, m_Img->dataOrder, m_Img->origin, m_Img->align,
							m_Img->width, m_Img->height, m_Img->roi, m_Img->maskROI,
							"Image", m_Img->tileInfo);

			//	Allocate IPL Image Buffer of size specified in Header and clear it
		iplAllocateImage(m_ImgDisp,1,0);
	}

	if (m_Img->depth==IPL_DEPTH_32F) 
		iplScaleFP(m_Img,m_ImgDisp,min,max);
	else 
		iplScale(m_Img,m_ImgDisp);

	return true;
}





/*


//////////////////////////////////////////////////////////////////////
//Save image as an IPL file or a BMP file.  
//If no filename is supplied then a dialog box will be used.
bool CIplImg::Save(const char *Filename)
{
	//Check filename is sensible
	if (strlen(szFilename)<1){return false;}

	////////////////////////////
	//	OPEN AND WRITE FILE
	//	Open File using API call
	HANDLE hFile=NULL;
	

	// call appropriate save function
	CString strFilename = szFilename;
	CString strExt = strFilename.Right(4);

	bool bResult;
	if (strExt == ".ipl") bResult = SaveIPL(hFile);
	else if (strExt == ".bmp") bResult = SaveBMP(hFile);
	else {
		AfxMessageBox("Invalid File Extension. Only '.ipl' and '.bmp' are currently supported");
		bResult = false;
	}

	CloseHandle(hFile);
	return bResult;
}
*/




bool CIplImg::ReadBMP(const char *fnm)
{
	if(!fnm){
		fprintf(pErrorFile,"CIplImg::Read() fnm == NULL\n");
		return false;
	}

	FILE *_f = fopen(fnm,"rb");
	if (!_f)
		return false;

	if(!ReadBMP((FILE*)_f))
		fprintf(pErrorFile,"CIplImg::Read(), error reading from %s\n",fnm);

	if (ferror(_f))
	{
		fprintf(pErrorFile,"CIplImg::Read(), ferror reading from to %s\n",fnm);
		perror(NULL);
		return false;
	}

	fclose(_f);
	return true;
}



bool CIplImg::WriteBMP(const char *fnm)
{
	if(!fnm){
		fprintf(pErrorFile,"CIplImg::Write() fnm == NULL\n");
		return false;
	}

	FILE *_f = fopen(fnm,"wb");
	if (!_f)
		return false;

	if(!WriteBMP((FILE*)_f))
		fprintf(pErrorFile,"CIplImg::Write(), error writing to %s\n",fnm);

	if (ferror(_f))
	{
		fprintf(pErrorFile,"CIplImg::Write(), ferror writing to %s\n",fnm);
		perror(NULL);
		return false;
	}

	fclose(_f);
	return true;
}




////////////////////////////////////////////////////////////////////////
//Save viewable image as a BMP file (8bit grey with palette if 1 channel
//or 24bit colour if 3 channels).
bool CIplImg::WriteBMP(FILE *_f)
{
	if (!MakeViewableImage()) {
			fprintf(pErrorFile,"CIplImg::SaveBMP(), couldn't make a viewable image\n");
			return false;
	}

	int nPaletteEntries = 0;

	//	Write BMP File Header
	BITMAPFILEHEADER BFH;
	BFH.bfType=19778;
	BFH.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	if (m_ImgDisp->nChannels == 1) BFH.bfOffBits += 256*sizeof(RGBQUAD);
	BFH.bfSize = BFH.bfOffBits + m_ImgDisp->imageSize;
	BFH.bfReserved1 = 0;
	BFH.bfReserved2 = 0;

	DWORD nNumberOfBytesToWrite=sizeof(BITMAPFILEHEADER);
	DWORD nNumberOfBytesWritten=0;
	//DWORD bWriteOK=WriteFile(hFile, &BFH, nNumberOfBytesToWrite,  &nNumberOfBytesWritten,NULL);
	
	nNumberOfBytesWritten=fwrite(&BFH,sizeof(BYTE),nNumberOfBytesToWrite,_f);

	if (nNumberOfBytesWritten!=nNumberOfBytesToWrite){
		fprintf(pErrorFile,
		"CIplImg::WriteBMP(), Error Writing BITMAPFILEHEADER, Written %d bytes, Expecting to write %d\n",
						nNumberOfBytesWritten,nNumberOfBytesToWrite);
		return false;
	}

	// set up a chunk of memory to put dib into
	BYTE *dib = new BYTE[BFH.bfSize-sizeof(BITMAPFILEHEADER)];

	// copy BITMAPINFO from m_ImgDisp into it
	memcpy(dib, m_Bmi, BFH.bfOffBits - sizeof(BITMAPFILEHEADER));

	// make the height value positive
	BITMAPINFOHEADER *pBIH = (BITMAPINFOHEADER *)dib;
	pBIH->biHeight *= -1;
	
	// use ipl's conversion function (used to save m_Bmi and data in m_ImgDisp, but windows 2000
	// doesn't like negative heights in bitmaps- this saves us having to flip rows ourselves)
	
	if(m_ImgDisp)
		iplConvertToDIB(m_ImgDisp,pBIH,IPL_DITHER_NONE,IPL_PALCONV_NONE);
	
	//	Write dib to File 
	nNumberOfBytesToWrite=BFH.bfSize-sizeof(BITMAPFILEHEADER);
	nNumberOfBytesWritten=0;
	
	nNumberOfBytesWritten=fwrite(dib,sizeof(BYTE),nNumberOfBytesToWrite,_f);

	//bWriteOK=WriteFile(hFile, dib, nNumberOfBytesToWrite,  &nNumberOfBytesWritten,NULL);
	
	if(nNumberOfBytesWritten!=nNumberOfBytesToWrite){
		fprintf(pErrorFile,
	"CIplImg::WriteBMP(), Error Writing dib, Written %d bytes, Expecting to write %d\n",
						nNumberOfBytesWritten,nNumberOfBytesToWrite);
		return false;
	}

	delete [] dib;

	return true;
}



/*
//////////////////////////////////////////////////////////////////////
//Load an IPL or BMP file.  
//If no filename is supplied then a dialog box will be used.
bool CIplImg::Load(const char *Filename)
{
	char szFilename[256];//Internal filename to this function

	//Copy filename from input or Dialog Box
	if (Filename==NULL){
		//create file dialog box
		CFileDialog *cfd=new CFileDialog( true, "*.ipl", NULL, NULL,
			"IPL/BMP Images (*.ipl;*.bmp)|*.ipl;*.bmp|All Files (*.*)|*.*||",NULL );
		//check it got created
		if (cfd==NULL){fprintf(pErrorFile,"Error opening File Dialog",MB_OK);return false;}
		//do it
		cfd->DoModal();
		//copy path to szFilename
		strcpy(szFilename,cfd->GetPathName());
		//clean up
		delete cfd;
		
		//aja
		fprintf(pErrorFile,"CIplImg::Load() filename=NULL\n");
		return false;
	}
	else
	{
		//Copy input filename to internal one (max 255 chars)
		strncpy(szFilename,Filename,255);
	}
	//Check filename is sensible
	if (strlen(szFilename)<1){return false;}

	////////////////////////////
	//	OPEN AND READ FILE
	//	Open File using API call
	HANDLE hFile=NULL;
	fprintf(pErrorFile,"Opening file:%s\n",szFilename);
	hFile=CreateFile(szFilename, GENERIC_READ,FILE_SHARE_READ,NULL,                               // pointer to security attributes
		OPEN_EXISTING,FILE_ATTRIBUTE_READONLY,  NULL);
	//	If Failed to Open generate error message and bail out
	if (hFile==INVALID_HANDLE_VALUE)
	{
		fprintf(pErrorFile,"Cannot Open File to Read",MB_OK);
		return false;
	}

	long buffersize = sizeof(IplFileHeader);
	if (sizeof(BITMAPFILEHEADER)>buffersize) buffersize = sizeof(BITMAPFILEHEADER);
	BYTE *buffer = new BYTE [buffersize];

	DWORD nNumberOfBytesRead=0;
	DWORD bReadOK=ReadFile(hFile, buffer, buffersize, &nNumberOfBytesRead, NULL);

	//Read IPL file header and check it is valid
	IplFileHeader *pIFH = (IplFileHeader *)buffer;
	BITMAPFILEHEADER *pBFH = (BITMAPFILEHEADER *)buffer;

	bool bResult;
	if (strncmp(pIFH->ID,"IPL",3) == 0)	bResult = LoadIPL(hFile);
	else if (pBFH->bfType == 19778)		bResult = LoadBMP(hFile);
	else bResult = false;

	delete [] buffer;
	CloseHandle(hFile);
	return bResult;
}

*/


//////////////////////////////////////////////////////////////////////
//Load a BMP file.  
bool CIplImg::ReadBMP(FILE *_f)
{
	//Read bitmap file header and check it is valid
	BITMAPFILEHEADER BFH; 
	DWORD nNumberOfBytesRead=0;
	
	//DWORD bReadOK=ReadFile(hFile, &BFH, sizeof(BITMAPFILEHEADER), &nNumberOfBytesRead,NULL);
	
	nNumberOfBytesRead=fread(&BFH,sizeof(BITMAPFILEHEADER),1,_f);
	
	if (BFH.bfType != 19778){
		fprintf(pErrorFile,"Invalid Bitmap File.");
		return false;
	}

	//At this point we should destroy any current image
	Destroy();

	// load the whole dib into a chunk of memory
	DWORD diblength = BFH.bfSize-sizeof(BITMAPFILEHEADER);
	BYTE * dib = new BYTE [diblength];
	//bReadOK=ReadFile(hFile, dib, diblength,  &nNumberOfBytesRead,NULL);
	
	
	nNumberOfBytesRead=fread(dib,sizeof(BYTE),diblength,_f);
	
	
	if (nNumberOfBytesRead != diblength) {
		fprintf(pErrorFile,"CIplImg::ReadBMP(): Failed reading bitmap header info.\nExpected %d bytes, read %d.", diblength, nNumberOfBytesRead);
		delete [] dib;
		return false;
	}

	BITMAPINFOHEADER *BIH = (BITMAPINFOHEADER *)dib;

		//Always make an IPL image that is of width aligned to quadword
		//DIBs are always aligned to doubleword
	DWORD qwordwidth=BIH->biWidth;
	DWORD dwordwidth=BIH->biWidth;
	while(qwordwidth%8!=0){qwordwidth++;}
	while(dwordwidth%4!=0){dwordwidth++;}

	//	Allocate the appropriate IPL Image Header
	if (BIH->biBitCount==8){//Assume 8-bit is Grey Scale Image
			m_Img=iplCreateImageHeader(1,0,IPL_DEPTH_8U,"GRAY","G",IPL_DATA_ORDER_PIXEL,IPL_ORIGIN_TL,
			IPL_ALIGN_QWORD,qwordwidth,abs(BIH->biHeight),NULL,NULL,"Image",NULL);
	}
	if (BIH->biBitCount==24){//24-bit Image
			m_Img=iplCreateImageHeader(3,0,IPL_DEPTH_8U,"RGB","BGR",IPL_DATA_ORDER_PIXEL,IPL_ORIGIN_TL,
			IPL_ALIGN_QWORD,qwordwidth,abs(BIH->biHeight),NULL,NULL,"Image",NULL);
	}

	//Check we allocated header OK
	if (m_Img==NULL){
		fprintf(pErrorFile,"CIplImg::ReadBMP(), Could not Create Ipl Image Header!");
		delete [] dib;
		return false;
	}

	//Allocater Image Data
	iplAllocateImage(m_Img,true,0);

	//Copy the DIB data into the IPL image
	if (BIH->biBitCount==8){//8-Bit palette dibs
		unsigned char col=0;
		unsigned char pix=0;
		int sum;
		RGBQUAD *rgba;
		unsigned char *dibdata = dib+BIH->biSize+256*sizeof(RGBQUAD);
		//Convert colour DIB palette to greyscale palette by averaging components
		for (DWORD pal=0;pal<256;pal++){
			rgba=(RGBQUAD*)(dib+BIH->biSize+pal*sizeof(RGBQUAD));
			sum=rgba->rgbBlue+rgba->rgbGreen+rgba->rgbRed;
			sum/=3;
			*(dib+BIH->biSize+pal*sizeof(RGBQUAD))=sum;
			*(dib+BIH->biSize+pal*sizeof(RGBQUAD)+1)=sum;
			*(dib+BIH->biSize+pal*sizeof(RGBQUAD)+2)=sum;
		}
		//Copy DIB data into IPL image data, taking care of IPL`s QWORD alignment
		for (DWORD ydib=0;ydib<BIH->biHeight;ydib++){
			for (DWORD xdib=0;xdib<dwordwidth;xdib++){
				pix=*(dibdata+(BIH->biHeight-ydib-1)*dwordwidth+xdib);//Pixel value
				col=*(dib+BIH->biSize+pix*sizeof(RGBQUAD));
				*(m_Img->imageData+m_Img->widthStep*ydib+xdib) = col; 	
		}}
	}
	else if (BIH->biBitCount==24){//24-bit dibs
	//Copy DIB data into IPL image data, taking care of IPL`s QWORD alignment
	for (DWORD ydib=0;ydib<BIH->biHeight;ydib++){
		for (DWORD xdib=0;xdib<dwordwidth;xdib++){
			for (DWORD c=0;c<3;c++){
			*(m_Img->imageData+m_Img->widthStep*ydib+xdib*3+c) = *(dib+BIH->biSize+(BIH->biHeight-ydib-1)*dwordwidth*3+xdib*3+c); 	
	}}}
	}
	else{
		fprintf(pErrorFile,"Bitmap Type unsupported.  Not 8-bit or 24-bit.\n");
		return false;
	}
	

	// use IPL's own conversion function
	//iplConvertFromDIB(BIH, m_Img);
	
	
	// deallocate dib memory (m_Img has it's own data allocated)
	delete [] dib;

	// check iplConvertFromDIB was successful...
	if (m_Img == NULL) {
		fprintf(pErrorFile,"Failed to convert DIB.");
		return false;
	}

	return true;
}



//////////////////////////////////////////////////////////////////////
//Set up a Region of Interest															//
//////////////////////////////////////////////////////////////////////
/*
bool CIplImg::SetROI(const CRect &roi)
{
	if (!IsValid()){return false;}
	if (roi==NULL){return false;}
	if (m_bHasROI==true){
		iplDeleteROI(m_Img->roi);m_bHasROI=false;fprintf(pErrorFile,"DeallocatedROI\n");
	}

	//Set the ROI
	m_Img->roi=iplCreateROI(0,roi.left,roi.top,roi.right-roi.left,roi.bottom-roi.top );
	m_bHasROI=true;
	return true;
}
*/
/*
//Delete region of interest.  All image processing will occur on full image.
bool CIplImg::ResetROI(void)
{
	if (!IsValid()){return false;}
	if (!m_bHasROI){return true;}
	iplDeleteROI(m_Img->roi);
	m_bHasROI=false;
	fprintf(pErrorFile,"DeallocatedROI\n");
	return true;
}
*/
/*
//////////////////////////////////////////////////////////////////////
//File IO															//
//The file extension is *.IPL and has a special format				//
//Class will also load raw 8-bit bytes								//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//Load an IPL or BMP file.  
//If no filename is supplied then a dialog box will be used.
bool CIplImg::Load(const char *Filename)
{
	char szFilename[256];//Internal filename to this function

	//Copy filename from input or Dialog Box
	if (Filename==NULL){
		//create file dialog box
		CFileDialog *cfd=new CFileDialog( true, "*.ipl", NULL, NULL,
			"IPL/BMP Images (*.ipl;*.bmp)|*.ipl;*.bmp|All Files (*.*)|*.*||",NULL );
		//check it got created
		if (cfd==NULL){fprintf(pErrorFile,"Error opening File Dialog",MB_OK);return false;}
		//do it
		cfd->DoModal();
		//copy path to szFilename
		strcpy(szFilename,cfd->GetPathName());
		//clean up
		delete cfd;
		
		//aja
		fprintf(pErrorFile,"CIplImg::Load() filename=NULL\n");
		return false;
	}
	else
	{
		//Copy input filename to internal one (max 255 chars)
		strncpy(szFilename,Filename,255);
	}
	//Check filename is sensible
	if (strlen(szFilename)<1){return false;}

	////////////////////////////
	//	OPEN AND READ FILE
	//	Open File using API call
	HANDLE hFile=NULL;
	fprintf(pErrorFile,"Opening file:%s\n",szFilename);
	hFile=CreateFile(szFilename, GENERIC_READ,FILE_SHARE_READ,NULL,                               // pointer to security attributes
		OPEN_EXISTING,FILE_ATTRIBUTE_READONLY,  NULL);
	//	If Failed to Open generate error message and bail out
	if (hFile==INVALID_HANDLE_VALUE)
	{
		fprintf(pErrorFile,"Cannot Open File to Read",MB_OK);
		return false;
	}

	long buffersize = sizeof(IplFileHeader);
	if (sizeof(BITMAPFILEHEADER)>buffersize) buffersize = sizeof(BITMAPFILEHEADER);
	BYTE *buffer = new BYTE [buffersize];

	DWORD nNumberOfBytesRead=0;
	DWORD bReadOK=ReadFile(hFile, buffer, buffersize, &nNumberOfBytesRead, NULL);

	//Read IPL file header and check it is valid
	IplFileHeader *pIFH = (IplFileHeader *)buffer;
	BITMAPFILEHEADER *pBFH = (BITMAPFILEHEADER *)buffer;

	bool bResult;
	if (strncmp(pIFH->ID,"IPL",3) == 0)	bResult = LoadIPL(hFile);
	else if (pBFH->bfType == 19778)		bResult = LoadBMP(hFile);
	else bResult = false;

	delete [] buffer;
	CloseHandle(hFile);
	return bResult;
}


//////////////////////////////////////////////////////////////////////
//Save image as an IPL file or a BMP file.  
//If no filename is supplied then a dialog box will be used.
bool CIplImg::Save(const char *Filename)
{
	char szFilename[256];//Internal filename to this function
	if (!IsValid()){return false;}

	//Copy filename from input or Dialog Box
	if (Filename==NULL){
		//create file dialog box
		CFileDialog *cfd=new CFileDialog( false, "*.ipl", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
			"IPL/BMP Images (*.ipl;*.bmp)|*.ipl;*.bmp|All Files (*.*)|*.*||",NULL );
		//check it get created
		if (cfd==NULL){fprintf(pErrorFile,"Error opening File Dialog",MB_OK);return false;}
		//do it
		cfd->DoModal();
		//copy path to szFilename
		strcpy(szFilename,cfd->GetPathName());
		//clean up
		delete cfd;
	}
	else
	{
		//Copy input filename to internal one (max 255 chars)
		strncpy(szFilename,Filename,255);
	}
	//Check filename is sensible
	if (strlen(szFilename)<1){return false;}

	////////////////////////////
	//	OPEN AND WRITE FILE
	//	Open File using API call
	HANDLE hFile=NULL;
	fprintf(pErrorFile,"Saving file:%s\n",szFilename);
	if (Filename == NULL) { // we've already asked if we can overwrite file if it exists
		hFile=CreateFile(szFilename, GENERIC_WRITE,FILE_SHARE_READ,NULL,                               // pointer to security attributes
					CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,  NULL);
	}
	else { // don't know for sure if an existing file can be overwritten, so play safe and ask
		hFile=CreateFile(szFilename, GENERIC_WRITE,FILE_SHARE_READ,NULL,                               // pointer to security attributes
					CREATE_NEW,FILE_ATTRIBUTE_NORMAL,  NULL);
		if (hFile==INVALID_HANDLE_VALUE) {
			if (GetLastError() == ERROR_FILE_EXISTS) {
				fprintf(pErrorFile,"file exists\n");
				int resp = AfxMessageBox("File exists. Overwrite?", MB_YESNO);
				if (resp == IDNO) return false;
				else {
					hFile=CreateFile(szFilename, GENERIC_WRITE,FILE_SHARE_READ,NULL,                               // pointer to security attributes
						CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,  NULL);
				}
			}
		}	
	}

	// finally verify we've got a valid handle
	if (hFile==INVALID_HANDLE_VALUE) {
		fprintf(pErrorFile,"Cannot Open File to Write",MB_OK);
		return false;
	}

	// call appropriate save function
	CString strFilename = szFilename;
	CString strExt = strFilename.Right(4);

	bool bResult;
	if (strExt == ".ipl") bResult = SaveIPL(hFile);
	else if (strExt == ".bmp") bResult = SaveBMP(hFile);
	else {
		AfxMessageBox("Invalid File Extension. Only '.ipl' and '.bmp' are currently supported");
		bResult = false;
	}

	CloseHandle(hFile);
	return bResult;
}

//////////////////////////////////////////////////////////////////////
//Load an IPL file.
bool CIplImg::LoadIPL(HANDLE hFile)
{

	DWORD nNumberOfBytesRead=0;
	SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
	IplFileHeader IFH;
	DWORD bReadOK=ReadFile(hFile, &IFH, sizeof(IFH), &nNumberOfBytesRead, NULL);
	ASSERT(strncmp(IFH.ID,"IPL",3) == 0);

	// additional check on filesize
	if (IFH.nFileSize != (long)GetFileSize(hFile, NULL)) {
		fprintf(pErrorFile,"IFH.nFileSize = %d, actual size = %d\n", IFH.nFileSize, GetFileSize(hFile, NULL));
		return false;
	}

	//At this point we should destroy any current image
	Destroy();ASSERT(!m_Img);
	//Allocate a dummy IplImage structure.  We will read the structure from the file into here
	//We cannot use m_Img=new IplImage because IPL deallocate wont work.
	m_Img=iplCreateImageHeader(1,0,IPL_DEPTH_8U,"GRAY","G",IPL_DATA_ORDER_PIXEL,IPL_ORIGIN_TL,IPL_ALIGN_QWORD,2,2,NULL,NULL,"Image",NULL);
	if (m_Img==NULL){fprintf(pErrorFile,"Unable to Create Image Header");}
	
	//Read in The IPL Header
	nNumberOfBytesRead=0;
	bReadOK=ReadFile(hFile, m_Img, sizeof(IplImage),  &nNumberOfBytesRead,NULL);
	if (nNumberOfBytesRead!=sizeof(IplImage)){
		fprintf(pErrorFile,"Unable to read IPL header");
		return false;
	}

	//Allocate IPL Image Buffer of size specified in Header and clear it
	m_Img->imageData=NULL;m_Img->imageDataOrigin=NULL;m_Img->roi=NULL;
	if (m_Img->depth==IPL_DEPTH_32F){iplAllocateImageFP(m_Img,1,0);}
	else{iplAllocateImage(m_Img,1,0);}

	//Read in the ROI if it exists
	if (IFH.nHasROI){
		nNumberOfBytesRead=0;
		//create dummy roi
		m_Img->roi=iplCreateROI(0,0,0,2,2);
		//read file roi
		ReadFile(hFile, m_Img->roi, sizeof(IplROI),  &nNumberOfBytesRead,NULL);
		m_bHasROI=true;
	}


	//Read in The IPL Image Data
	nNumberOfBytesRead=0;
	bReadOK=ReadFile(hFile, m_Img->imageData, m_Img->imageSize,  &nNumberOfBytesRead,NULL);
	if (nNumberOfBytesRead!=(DWORD)m_Img->imageSize)
	{
		fprintf(pErrorFile,"An error occured whilst reading the image data");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////
//Save as an IPL file.  
bool CIplImg::SaveIPL(HANDLE hFile)
{
	ASSERT(hFile!=INVALID_HANDLE_VALUE);

	//	Write IPL Class Header to Identify File Format
	IplFileHeader IFH;
	strcpy(&IFH.ID[0],"IPL");
	IFH.nHasROI=m_bHasROI;
	IFH.nFrames = 1;
	IFH.Reserved1 = 0;
	IFH.Reserved2 = 0;
	IFH.nFileSize = sizeof(IplFileHeader) + m_Img->nSize + m_Img->imageSize;
	if (IFH.nHasROI) IFH.nFileSize += sizeof(IplROI);

	DWORD nNumberOfBytesToWrite=sizeof(IplFileHeader);
	DWORD nNumberOfBytesWritten=0;
	DWORD bWriteOK=WriteFile(hFile, &IFH, nNumberOfBytesToWrite,  &nNumberOfBytesWritten,NULL);
	if (nNumberOfBytesWritten!=nNumberOfBytesToWrite){
		fprintf(pErrorFile,"Error Writing File Header!");
		return false;
	}

	//	Write IPL Header to File 
	nNumberOfBytesToWrite=m_Img->nSize;
	nNumberOfBytesWritten=0;
	bWriteOK=WriteFile(hFile, m_Img, nNumberOfBytesToWrite,  &nNumberOfBytesWritten,NULL);
	if (nNumberOfBytesWritten!=nNumberOfBytesToWrite){
		fprintf(pErrorFile,"Error Writing IPL Header!");
		return false;
	}

	// Write IPL ROI to File if exists
	if (m_Img->roi!=NULL){
		nNumberOfBytesToWrite=sizeof(IplROI);
		nNumberOfBytesWritten=0;
		bWriteOK=WriteFile(hFile, m_Img->roi, nNumberOfBytesToWrite,  &nNumberOfBytesWritten,NULL);
		if (nNumberOfBytesWritten!=nNumberOfBytesToWrite){
			fprintf(pErrorFile,"Error Writing IPL ROI!");
			return false;
		}
		
	}

	//	Write Data to File directly from IPL Image
	nNumberOfBytesToWrite=m_Img->imageSize;
	nNumberOfBytesWritten=0;
	bWriteOK=WriteFile(hFile, m_Img->imageData, nNumberOfBytesToWrite,  &nNumberOfBytesWritten,NULL);
	if (nNumberOfBytesWritten!=nNumberOfBytesToWrite){
		fprintf(pErrorFile,"Error Writing IPL ImageData!");
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
//Load a BMP file.  
bool CIplImg::LoadBMP(HANDLE hFile)
{
	SetFilePointer(hFile, 0, NULL, FILE_BEGIN);

	//Read bitmap file header and check it is valid
	BITMAPFILEHEADER BFH; 
	DWORD nNumberOfBytesRead=0;
	DWORD bReadOK=ReadFile(hFile, &BFH, sizeof(BITMAPFILEHEADER),  &nNumberOfBytesRead,NULL);
	if (BFH.bfType != 19778){
		fprintf(pErrorFile,"Invalid Bitmap File.");
		return false;
	}

	//At this point we should destroy any current image
	Destroy();ASSERT(!m_Img);

	// load the whole dib into a chunk of memory
	DWORD diblength = BFH.bfSize-sizeof(BITMAPFILEHEADER);
	BYTE * dib = new BYTE [diblength];
	bReadOK=ReadFile(hFile, dib, diblength,  &nNumberOfBytesRead,NULL);
	if (nNumberOfBytesRead != diblength) {
		fprintf(pErrorFile,"Failed reading bitmap header info.\nExpected %d bytes, read %d.", diblength, nNumberOfBytesRead);
		delete [] dib;
		return false;
	}

	BITMAPINFOHEADER *BIH = (BITMAPINFOHEADER *)dib;

	//Always make an IPL image that is of width aligned to quadword
	//DIBs are always aligned to doubleword
	DWORD qwordwidth=BIH->biWidth;
	DWORD dwordwidth=BIH->biWidth;
	while(qwordwidth%8!=0){qwordwidth++;}
	while(dwordwidth%4!=0){dwordwidth++;}

	//	Allocate the appropriate IPL Image Header
	if (BIH->biBitCount==8){//Assume 8-bit is Grey Scale Image
			m_Img=iplCreateImageHeader(1,0,IPL_DEPTH_8U,"GRAY","G",IPL_DATA_ORDER_PIXEL,IPL_ORIGIN_TL,
			IPL_ALIGN_QWORD,qwordwidth,abs(BIH->biHeight),NULL,NULL,"Image",NULL);
	}
	if (BIH->biBitCount==24){//24-bit Image
			m_Img=iplCreateImageHeader(3,0,IPL_DEPTH_8U,"RGB","BGR",IPL_DATA_ORDER_PIXEL,IPL_ORIGIN_TL,
			IPL_ALIGN_QWORD,qwordwidth,abs(BIH->biHeight),NULL,NULL,"Image",NULL);
	}

	//Check we allocated header OK
	if (m_Img==NULL){
		fprintf(pErrorFile,"Could not Create Ipl Image Header!");
		delete [] dib;
		return false;
	}

	//Allocater Image Data
	iplAllocateImage(m_Img,true,0);

	//Copy the DIB data into the IPL image
	if (BIH->biBitCount==8){//8-Bit palette dibs
		unsigned char col=0;
		unsigned char pix=0;
		int sum;
		RGBQUAD *rgba;
		unsigned char *dibdata = dib+BIH->biSize+256*sizeof(RGBQUAD);
		//Convert colour DIB palette to greyscale palette by averaging components
		for (DWORD pal=0;pal<256;pal++){
			rgba=(RGBQUAD*)(dib+BIH->biSize+pal*sizeof(RGBQUAD));
			sum=rgba->rgbBlue+rgba->rgbGreen+rgba->rgbRed;
			sum/=3;
			*(dib+BIH->biSize+pal*sizeof(RGBQUAD))=sum;
			*(dib+BIH->biSize+pal*sizeof(RGBQUAD)+1)=sum;
			*(dib+BIH->biSize+pal*sizeof(RGBQUAD)+2)=sum;
		}
		//Copy DIB data into IPL image data, taking care of IPL`s QWORD alignment
		for (DWORD ydib=0;ydib<BIH->biHeight;ydib++){
			for (DWORD xdib=0;xdib<dwordwidth;xdib++){
				pix=*(dibdata+(BIH->biHeight-ydib-1)*dwordwidth+xdib);//Pixel value
				col=*(dib+BIH->biSize+pix*sizeof(RGBQUAD));
				*(m_Img->imageData+m_Img->widthStep*ydib+xdib) = col; 	
		}}
	}
	else if (BIH->biBitCount==24){//24-bit dibs
	//Copy DIB data into IPL image data, taking care of IPL`s QWORD alignment
	for (DWORD ydib=0;ydib<BIH->biHeight;ydib++){
		for (DWORD xdib=0;xdib<dwordwidth;xdib++){
			for (DWORD c=0;c<3;c++){
			*(m_Img->imageData+m_Img->widthStep*ydib+xdib*3+c) = *(dib+BIH->biSize+(BIH->biHeight-ydib-1)*dwordwidth*3+xdib*3+c); 	
	}}}
	}
	else{
		TRACE0("Bitmap Type unsupported.  Not 8-bit or 24-bit.\n");
		return false;
	}
	

	// use IPL's own conversion function
	//iplConvertFromDIB(BIH, m_Img);
	
	
	// deallocate dib memory (m_Img has it's own data allocated)
	delete [] dib;

	// check iplConvertFromDIB was successful...
	if (m_Img == NULL) {
		fprintf(pErrorFile,"Failed to convert DIB.");
		return false;
	}

	return true;
}

*/
/*
////////////////////////////////////////////////////////////////////////
//Copy viewable image as a BMP file to ClipBoard (8bit grey with palette if 1 channel
//or 24bit colour if 3 channels).
bool CIplImg::CopyToClipboard(void)
{
	if (!m_ImgDisp) {
		if (!MakeViewableImage()) {
			fprintf(pErrorFile,"couldn't make a viewable image\n");
			return false;
		}
	}


	// copy BITMAPINFO from m_ImgDisp into it
	int bmihsize = sizeof(BITMAPINFOHEADER);
	if (m_ImgDisp->nChannels == 1) bmihsize += 256*sizeof(RGBQUAD);

	// set up a chunk of memory to put dib into must use GlobalAlloc
	// for clipboard
	long dibsize=m_ImgDisp->imageSize+bmihsize;
	HGLOBAL hMem=GlobalAlloc(GMEM_MOVEABLE,dibsize);
	BYTE *dib=(BYTE*)GlobalLock(hMem);
	memcpy(dib, m_Bmi, bmihsize);

	// make the height value positive
	BITMAPINFOHEADER *pBIH = (BITMAPINFOHEADER *)dib;
	pBIH->biHeight *= -1;

	// use ipl's conversion function (used to save m_Bmi and data in m_ImgDisp, but windows 2000
	// doesn't like negative heights in bitmaps- this saves us having to flip rows ourselves)
	iplConvertToDIB(m_ImgDisp, pBIH, IPL_DITHER_NONE, IPL_PALCONV_NONE);

	//Unlock global memory
	GlobalUnlock(hMem);

	//Open Clipboard
	if (!OpenClipboard(NULL)){fprintf(pErrorFile,"Could Not Open Clipboard\n");return false;}
	EmptyClipboard();
	if (SetClipboardData(CF_DIB,hMem)==NULL){fprintf(pErrorFile,"Could not Paste to Clipboard\n");return false;}
	CloseClipboard();

	return true;
}
*/
/*
////////////////////////////////////////////////////////////////////////
//Save viewable image as a BMP file (8bit grey with palette if 1 channel
//or 24bit colour if 3 channels).
bool CIplImg::SaveBMP(HANDLE hFile)
{
	ASSERT(hFile!=INVALID_HANDLE_VALUE);

	if (!m_ImgDisp) {
		if (!MakeViewableImage()) {
			fprintf(pErrorFile,"couldn't make a viewable image\n");
			return false;
		}
	}

	int nPaletteEntries = 0;

	//	Write BMP File Header
	BITMAPFILEHEADER BFH;
	BFH.bfType = 19778;
	BFH.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	if (m_ImgDisp->nChannels == 1) BFH.bfOffBits += 256*sizeof(RGBQUAD);
	BFH.bfSize = BFH.bfOffBits + m_ImgDisp->imageSize;
	BFH.bfReserved1 = 0;
	BFH.bfReserved2 = 0;

	DWORD nNumberOfBytesToWrite=sizeof(BITMAPFILEHEADER);
	DWORD nNumberOfBytesWritten=0;
	DWORD bWriteOK=WriteFile(hFile, &BFH, nNumberOfBytesToWrite,  &nNumberOfBytesWritten,NULL);
	if (nNumberOfBytesWritten!=nNumberOfBytesToWrite){
		fprintf(pErrorFile,"Error Writing BITMAPFILEHEADER!");
		return false;
	}

	// set up a chunk of memory to put dib into
	BYTE *dib = new BYTE[BFH.bfSize-sizeof(BITMAPFILEHEADER)];

	// copy BITMAPINFO from m_ImgDisp into it
	memcpy(dib, m_Bmi, BFH.bfOffBits - sizeof(BITMAPFILEHEADER));

	// make the height value positive
	BITMAPINFOHEADER *pBIH = (BITMAPINFOHEADER *)dib;
	pBIH->biHeight *= -1;

	// use ipl's conversion function (used to save m_Bmi and data in m_ImgDisp, but windows 2000
	// doesn't like negative heights in bitmaps- this saves us having to flip rows ourselves)
	ASSERT(m_ImgDisp);
	iplConvertToDIB(m_ImgDisp, pBIH, IPL_DITHER_NONE, IPL_PALCONV_NONE);

	//	Write dib to File 
	nNumberOfBytesToWrite=BFH.bfSize-sizeof(BITMAPFILEHEADER);
	nNumberOfBytesWritten=0;
	bWriteOK=WriteFile(hFile, dib, nNumberOfBytesToWrite,  &nNumberOfBytesWritten,NULL);
	if (nNumberOfBytesWritten!=nNumberOfBytesToWrite){
		fprintf(pErrorFile,"Error Writing dib!");
		return false;
	}

	delete [] dib;

	return true;
}

//////////////////////////////////////////////////////////////////////
//Load a RAW 8bit byte greylevel image without dialog
bool CIplImg::LoadRaw(const char *Filename,unsigned long Width, unsigned long Height,unsigned int Planes)
{
	if (Height<1 || Width<1){TRACE0("***Error:LoadRaw:Invalid Image Dimensions\n");return false;}
	if (Planes<1){TRACE0("***Error:LoadRaw:Invalid Planes\n");return false;}
	DWORD nNumberOfBytesToRead=Width*Height*Planes;
	DWORD nNumberOfBytesRead=0;
	
	////////////////////////////
	//	OPEN AND READ FILE
	//	Open File using API call
	HANDLE hFile=NULL;
	hFile=CreateFile(Filename, GENERIC_READ,FILE_SHARE_READ,NULL,                               // pointer to security attributes
		OPEN_EXISTING,FILE_ATTRIBUTE_READONLY,  NULL);
	//	If Failed to Open generate error message and bail out
	if (hFile==INVALID_HANDLE_VALUE)
	{
		fprintf(pErrorFile,"Cannot Open File",MB_OK);
		return false;
	}
	
	//	Get File Size
	DWORD FileSizeHigh=0;
	DWORD dwFileSize=GetFileSize(hFile,&FileSizeHigh);
	//	If file size smaller than Image then only read whats in file
	if (Width*Height>dwFileSize){
		if (dwFileSize==0){
			fprintf(pErrorFile,"Cannot Open File",MB_OK);
			return false;
		}
		fprintf(pErrorFile,"Premature end of file",MB_OK);
		nNumberOfBytesToRead=dwFileSize;
	}

	//	Create an IPL image of 8Bit
	ASSERT(Planes==1 || Planes==3);
	if (Planes==1)	Create8U(Width,Height,NULL);
	if (Planes==3)	CreateRGB(Width,Height,NULL);

	//	Read Data from File directly into IPL Image
	DWORD bReadOK=ReadFile(hFile, m_Img->imageData, nNumberOfBytesToRead,  &nNumberOfBytesRead,NULL);
	
	//	Close File
	CloseHandle(hFile);
	
	return true;
}

//////////////////////////////////////////////////////////////////////
//Save as an 8-bit raw image
bool CIplImg::SaveRaw(const char *Filename)
{
	if (!IsValid()){return false;}
	if (m_Img->depth!=IPL_DEPTH_8U){
		fprintf(pErrorFile,"Can only save 8-bit images to Raw Files",MB_OK);
		return false;
	}
	DWORD nNumberOfBytesToWrite=m_Img->imageSize;
	DWORD nNumberOfBytesWritten=0;

	////////////////////////////
	//	OPEN AND WRITE FILE
	//	Open File using API call
	HANDLE hFile=NULL;
	hFile=CreateFile(Filename, GENERIC_WRITE,FILE_SHARE_READ,NULL,                               // pointer to security attributes
					CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,  NULL);
	//	If Failed to Open generate error message and bail out
	if (hFile==INVALID_HANDLE_VALUE)
	{
		fprintf(pErrorFile,"Cannot Open File to Write",MB_OK);
		return false;
	}

	//	Write Data to File directly from IPL Image
	DWORD bWriteOK=WriteFile(hFile, m_Img->imageData, nNumberOfBytesToWrite,  &nNumberOfBytesWritten,NULL);
	
	//	Close File
	CloseHandle(hFile);

	return true;
}
*/
/*
void CIplImg::SetPixel(int x, int y,float *val)
{
#if defined (_DEBUG)
	if (Depth()!=IPL_DEPTH_32F){return;}
#endif
	iplPutPixel(GetpImg(),x,y,val);
}

void CIplImg::SetPixel(int x, int y,long *val)
{
	if (Depth()==IPL_DEPTH_32F){
		float fval=(float)(*val);
		iplPutPixel(GetpImg(),x,y,&fval);
	}
	else
	{
		iplPutPixel(GetpImg(),x,y,val);
	}
}

void CIplImg::SetPixel(int x, int y,CIpl_RGBA32F val)
{
	if (Depth()==IPL_DEPTH_32F){
		float *fval=(float*)(&val);
		iplPutPixel(GetpImg(),x,y,fval);
	}
	else
	{
		BYTE buf[4]={0};
		float *fval=(float*)(&val);
		for (int plane=0;plane<3;plane++){
			if (*(fval+plane)>255.0f){*(fval+plane)=255.0f;}
			if (*(fval+plane)<0.0f){*(fval+plane)=0.0f;}
			*(buf+plane)=(BYTE)*(fval+plane);
		}
		
		iplPutPixel(GetpImg(),x,y,&buf[0]);
	}
}


void CIplImg::SetPixel(int x,int y,CIpl_RGB val)
{
	if (Depth()!=IPL_DEPTH_8U){return;}
	iplPutPixel(GetpImg(),x,y,&val);
}

//This function is usefull for FAST floating point pixel operations
void CIplImg::GetPixel32F(int x,int y,float *result)
{
#if defined (_DEBUG)
	if (x<0 || y<0 || x>Width() || y>Height()){fprintf(pErrorFile,"***ERR:GetPixel32F (%d,%d)\n",x,y);return;}
#endif
	iplGetPixel(m_Img, x, y, result);	
}


CIpl_RGBA32F CIplImg::GetPixel(int x, int y)
{
	CIpl_RGBA32F result;
	result.r = 0;
	result.g = 0;
	result.b = 0;
	result.a = 0;
	if (!IsValid()) return result;
	if (x<0 || y<0 || x>Width() || y>Height()){	return result;}
	BYTE buffer[16];
	iplGetPixel(m_Img, x, y, buffer);

	if (m_Img->nChannels == 4) {
		switch(this->m_Img->depth)
		{
		case IPL_DEPTH_8U:
			result.b=(float)buffer[0];
			result.g=(float)buffer[1];
			result.r=(float)buffer[2];
			result.a=(float)buffer[3];
			break;
		case IPL_DEPTH_16U:
			result.b=(float)((unsigned short *)buffer)[0];
			result.g=(float)((unsigned short *)buffer)[1];
			result.r=(float)((unsigned short *)buffer)[2];
			result.a=(float)((unsigned short *)buffer)[3];
			break;
		case IPL_DEPTH_16S:
			result.b=(float)((short *)buffer)[0];
			result.g=(float)((short *)buffer)[1];
			result.r=(float)((short *)buffer)[2];
			result.a=(float)((short *)buffer)[3];
			break;
		case IPL_DEPTH_32S:
			result.b=(float)((long *)buffer)[0];
			result.g=(float)((long *)buffer)[1];
			result.r=(float)((long *)buffer)[2];
			result.a=(float)((long *)buffer)[3];
			break;
		case IPL_DEPTH_32F:
			result.b=((float *)buffer)[0];
			result.g=((float *)buffer)[1];
			result.r=((float *)buffer)[2];
			result.a=((float *)buffer)[3];
			break;
		}
	}
	else if (m_Img->nChannels == 3) {
		switch(this->m_Img->depth)
		{
		case IPL_DEPTH_8U:
			result.b=(float)buffer[0];
			result.g=(float)buffer[1];
			result.r=(float)buffer[2];
			break;
		case IPL_DEPTH_16U:
			result.b=(float)((unsigned short *)buffer)[0];
			result.g=(float)((unsigned short *)buffer)[1];
			result.r=(float)((unsigned short *)buffer)[2];
			break;
		case IPL_DEPTH_16S:
			result.b=(float)((short *)buffer)[0];
			result.g=(float)((short *)buffer)[1];
			result.r=(float)((short *)buffer)[2];
			break;
		case IPL_DEPTH_32S:
			result.b=(float)((long *)buffer)[0];
			result.g=(float)((long *)buffer)[1];
			result.r=(float)((long *)buffer)[2];
			break;
		case IPL_DEPTH_32F:
			result.b=((float *)buffer)[0];
			result.g=((float *)buffer)[1];
			result.r=((float *)buffer)[2];
			break;
		}
	}
	else if (m_Img->nChannels == 1) {
		switch(this->m_Img->depth)
		{
		case IPL_DEPTH_8U:
			result.b=(float)buffer[0];
			result.g=(float)buffer[0];
			result.r=(float)buffer[0];
			break;
		case IPL_DEPTH_16U:
			result.b=(float)((unsigned short *)buffer)[0];
			result.g=(float)((unsigned short *)buffer)[0];
			result.r=(float)((unsigned short *)buffer)[0];
			break;
		case IPL_DEPTH_16S:
			result.b=(float)((short *)buffer)[0];
			result.g=(float)((short *)buffer)[0];
			result.r=(float)((short *)buffer)[0];
			break;
		case IPL_DEPTH_32S:
			result.b=(float)((long *)buffer)[0];
			result.g=(float)((long *)buffer)[0];
			result.r=(float)((long *)buffer)[0];
			break;
		case IPL_DEPTH_32F:
			result.b=((float *)buffer)[0];
			result.g=((float *)buffer)[0];
			result.r=((float *)buffer)[0];
			break;
		}
	}
	return result;
}

const CIpl_RGBA32F CIplImg::GetPixel(int x, int y) const
{
	CIpl_RGBA32F result;
	result.r = 0;
	result.g = 0;
	result.b = 0;
	result.a = 0;
	if (!IsValid()) return result;
	if (x<0 || y<0 || x>Width() || y>Height()){	return result;}
	BYTE buffer[16];
	iplGetPixel(m_Img, x, y, buffer);

	if (m_Img->nChannels == 4) {
		switch(this->m_Img->depth)
		{
		case IPL_DEPTH_8U:
			result.b=(float)buffer[0];
			result.g=(float)buffer[1];
			result.r=(float)buffer[2];
			result.a=(float)buffer[3];
			break;
		case IPL_DEPTH_16U:
			result.b=(float)((unsigned short *)buffer)[0];
			result.g=(float)((unsigned short *)buffer)[1];
			result.r=(float)((unsigned short *)buffer)[2];
			result.a=(float)((unsigned short *)buffer)[3];
			break;
		case IPL_DEPTH_16S:
			result.b=(float)((short *)buffer)[0];
			result.g=(float)((short *)buffer)[1];
			result.r=(float)((short *)buffer)[2];
			result.a=(float)((short *)buffer)[3];
			break;
		case IPL_DEPTH_32S:
			result.b=(float)((long *)buffer)[0];
			result.g=(float)((long *)buffer)[1];
			result.r=(float)((long *)buffer)[2];
			result.a=(float)((long *)buffer)[3];
			break;
		case IPL_DEPTH_32F:
			result.b=((float *)buffer)[0];
			result.g=((float *)buffer)[1];
			result.r=((float *)buffer)[2];
			result.a=((float *)buffer)[3];
			break;
		}
	}
	else if (m_Img->nChannels == 3) {
		switch(this->m_Img->depth)
		{
		case IPL_DEPTH_8U:
			result.b=(float)buffer[0];
			result.g=(float)buffer[1];
			result.r=(float)buffer[2];
			break;
		case IPL_DEPTH_16U:
			result.b=(float)((unsigned short *)buffer)[0];
			result.g=(float)((unsigned short *)buffer)[1];
			result.r=(float)((unsigned short *)buffer)[2];
			break;
		case IPL_DEPTH_16S:
			result.b=(float)((short *)buffer)[0];
			result.g=(float)((short *)buffer)[1];
			result.r=(float)((short *)buffer)[2];
			break;
		case IPL_DEPTH_32S:
			result.b=(float)((long *)buffer)[0];
			result.g=(float)((long *)buffer)[1];
			result.r=(float)((long *)buffer)[2];
			break;
		case IPL_DEPTH_32F:
			result.b=((float *)buffer)[0];
			result.g=((float *)buffer)[1];
			result.r=((float *)buffer)[2];
			break;
		}
	}
	else if (m_Img->nChannels == 1) {
		switch(this->m_Img->depth)
		{
		case IPL_DEPTH_8U:
			result.b=(float)buffer[0];
			result.g=(float)buffer[0];
			result.r=(float)buffer[0];
			break;
		case IPL_DEPTH_16U:
			result.b=(float)((unsigned short *)buffer)[0];
			result.g=(float)((unsigned short *)buffer)[0];
			result.r=(float)((unsigned short *)buffer)[0];
			break;
		case IPL_DEPTH_16S:
			result.b=(float)((short *)buffer)[0];
			result.g=(float)((short *)buffer)[0];
			result.r=(float)((short *)buffer)[0];
			break;
		case IPL_DEPTH_32S:
			result.b=(float)((long *)buffer)[0];
			result.g=(float)((long *)buffer)[0];
			result.r=(float)((long *)buffer)[0];
			break;
		case IPL_DEPTH_32F:
			result.b=((float *)buffer)[0];
			result.g=((float *)buffer)[0];
			result.r=((float *)buffer)[0];
			break;
		}
	}
	return result;
}
*/




//////////////////////////////////////////////////////////////////////
//Copying
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
//Make a new copy from this header and fill it with zeros
//This Function returns a new image which MUST be deleted at a later point
CIplImg * CIplImg::BlankCopy() const
{
	if(!IsValid()) return false;
	CIplImg *copy = new CIplImg;
	copy->Create(m_Img->width, m_Img->height, NULL, m_Img->depth, m_Img->nChannels);
	return copy;
}

/*
//////////////////////////////////////////////////////////////////////
//Copies data from another IPL Image into this image.  No new CIplImg is created,
bool CIplImg::Copy(CIplImg &src)
{
	if(!IsValid()) return false;;
	if (!src.IsSameTypeAs(this)){fprintf(pErrorFile,"***Error:Copy(): Imgs Different Types\n");return false;}
	iplCopy(src.GetpImg(),GetpImg());
	return true;
}
*/
//////////////////////////////////////////////////////////////////////
//Make a new copy from this header and fill it with copied data
//This Function returns a new image which MUST be deleted at a later point
CIplImg *CIplImg::Clone() const
{
	if(!IsValid()) return NULL;
	CIplImg *copy = new CIplImg;
	copy->Create(m_Img->width, m_Img->height, m_Img->imageData, m_Img->depth, m_Img->nChannels);
	return copy;
}
//Returns a new image reduced in size by a factor
CIplImg *CIplImg::Decimate(int factor)
{
	if(!IsValid()) return NULL;
	CIplImg *copy = new CIplImg;
	copy->Create(m_Img->width/factor, m_Img->height/factor, NULL, m_Img->depth, m_Img->nChannels);
	iplDecimate(GetpImg(),copy->GetpImg(),1,factor,1,factor,IPL_INTER_NN);
	return copy;
}
//Returns a new image reduced in size by a factor
CIplImg *CIplImg::Zoom(int factor)
{
	if(!IsValid()) return NULL;
	CIplImg *copy = new CIplImg;
	copy->Create(m_Img->width*factor, m_Img->height*factor, NULL, m_Img->depth, m_Img->nChannels);
	iplZoom(GetpImg(),copy->GetpImg(),factor,1,factor,1,IPL_INTER_NN);
	return copy;
}
//Resizes img to dimensions of dst and puts result in dst
bool CIplImg::Resize(CIplImg *dst, int inter)
{
	if(!IsValid()) return false;
	if(!dst->IsValid()) return false;
	iplResizeFit(GetpImg(), dst->GetpImg(), inter);
	return true;
}


//Move image data up/down
bool CIplImg::ShiftVertical(int nPixels)
{
	long nByteOffset=nPixels*WidthStepBytes();
	long nBytesToCopy=(Height()-nPixels)*WidthStepBytes();
	BYTE *psrc=(BYTE*)GetpData();
	psrc+=nByteOffset;
	memcpy((void*)GetpData(),psrc,nBytesToCopy);
	return true;
}


//Move image data left/right
bool CIplImg::ShiftHorizontal(int nPixels)
{
	return false;
}

//////////////////////////////////////////////////////////////////////
//Check that this image is the same as a comparison image
//Checks Width, Height, Depth, Channels
bool CIplImg::IsSameTypeAs(const CIplImg *src) const
{
	if(!IsValid()) return false;
	//if (src==NULL){return false;}
	if (src->Width()!=Width()){return false;}
	if (src->Height()!=Height()){return false;}
	if (src->Depth()!=Depth()){return false;}
	if (src->Channels()!=Channels()){return false;}
	return true;
}


//////////////////////////////////////////////////////////////////////
//Image Conversion
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
//Convert to greyscale (generates a new CIplImg- memory allocated)
CIplImg * CIplImg::ToGreyScale()
{
	if (!IsValid()) return NULL;
	CIplImg *result = new CIplImg;
	result->Create(m_Img->width, m_Img->height, NULL, m_Img->depth, 1);
	if (!ToGreyScale(result)) {
		fprintf(pErrorFile,"Failed greyscale conversion\n");
		delete result;
		return NULL;
	}

	return result;
}

//////////////////////////////////////////////////////////////////////
//Convert to greyscale (puts result into dst - no new memory allocated)
bool CIplImg::ToGreyScale(CIplImg *dst)
{
	if (!IsValid()) return false;
	if (dst==NULL){fprintf(pErrorFile,"ToGreyScale - Dest NULL\n");return false;}

	if (m_Img->nChannels==1) {
		fprintf(pErrorFile,"Source image is greyscale already\n");
		dst=Clone();
		return true;
	}

	if (dst->pImg->nChannels!=1) {
		fprintf(pErrorFile,"Destination image must have 1 channel\n");
		return false;
	}
	if ((m_Img->width != dst->pImg->width)
		| (m_Img->height != dst->pImg->height)) {
		fprintf(pErrorFile,"Images must be the same size\n");
		return false;
	}
	iplColorToGray(m_Img, (IplImage *)dst->pImg);
	return true;

}

//////////////////////////////////////////////////////////////////////
//Convert Bit Depth (creates a new CIplImg- memory allocated)
CIplImg * CIplImg::ConvertDepthTo(int depth,bool bScaleToFitRange)
{
	if (!IsValid()) return NULL;
	CIplImg *result = new CIplImg;
	result->Create(m_Img->width, m_Img->height, NULL, depth, m_Img->nChannels);
	if (!ConvertDepthTo(result,bScaleToFitRange)) {
		fprintf(pErrorFile,"Failed depth conversion\n");
		delete result;
		return NULL;
	}
	else return result;
}

//////////////////////////////////////////////////////////////////////
//Convert to Bit Depth of dst (result put in dst- no new memory allocated)
bool CIplImg::ConvertDepthTo(CIplImg *dst,bool bScaleToFitRange)
{
	if ((!IsValid()) | (!dst->IsValid())) return NULL;
	if (m_Img->depth==dst->pImg->depth) {
		fprintf(pErrorFile,"Image is already this depth\n");
		return false;
	}
	if (m_Img->nChannels!=dst->pImg->nChannels) {
		fprintf(pErrorFile,"Images must have same number of channels\n");
		return false;
	}
	if ((m_Img->width != dst->pImg->width)
		| (m_Img->height != dst->pImg->height)) {
		fprintf(pErrorFile,"Images must be the same size\n");
		return false;
	}

	//If Dest image is float
	if (dst->pImg->depth==IPL_DEPTH_32F) {
		float min, max;
		if (m_Img->depth==IPL_DEPTH_1U) {min=0; max=1;}
		else if (m_Img->depth==IPL_DEPTH_8U) {min=0; max=(float)pow(2.0f,(int)8)-1;}
		else if (m_Img->depth==IPL_DEPTH_8S) {min=-(float)pow(2.0f,(int)7)+1; max=(float)pow(2.0f,(int)7)-1;}
		else if (m_Img->depth==IPL_DEPTH_16U) {min=0; max=(float)pow(2.0f,(int)16)-1;}
		else if (m_Img->depth==IPL_DEPTH_16S) {min=-(float)pow(2.0f,(int)15)+1; max=(float)pow(2.0f,(int)15)-1;}
		else if (m_Img->depth==IPL_DEPTH_32S) {min=-(float)pow(2.0f,(int)31)+1; max=(float)pow(2.0f,(int)31)-1;}
		else {
			fprintf(pErrorFile,"Unknown input bit depth\n");
			return false;
		}
		iplScaleFP(m_Img, (IplImage *)dst->pImg, min, max);
	}
	//If dest is an integer type and we convert from float
	else if (m_Img->depth==IPL_DEPTH_32F) {
		float min, max;
		iplMinMaxFP(m_Img, &min, &max);
		iplScaleFP(m_Img, (IplImage *)dst->pImg, min, max);
	}
	//If dest is an integer type and we are also an integer type
	else {
		if (bScaleToFitRange){
			iplScale(m_Img, (IplImage *)dst->pImg);
		}
		else{
			iplConvert(m_Img, (IplImage *)dst->pImg);
		}
		
	}
	return true;
}


//////////////////////////////////////////////////////////////////////
// Image creation functions for synthetic images
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
/*
bool CIplImg::Zero(void)
{
	if (!IsValid()){return false;}
	if (Depth()==IPL_DEPTH_32F){
		iplSetFP(GetpImg(),0.0f);
	}
	else{
		iplSet(GetpImg(),0);
	}
	return true;
}
*/
bool CIplImg::Fill(int value)
{
	if (!IsValid()){return false;}
	if (Depth()==IPL_DEPTH_32F){
		iplSetFP(GetpImg(),(float)value);
	}
	else{
		iplSet(GetpImg(),value);
	}
	return true;
}

bool CIplImg::FillCheck(unsigned int size,unsigned int offset)
{
	return true;
}

//Frequencies in degrees/pixel
bool CIplImg::FillSine(float xfreq, float yfreq, float xphase,float yphase, float rot_deg)
{
	if (!IsValid()){return false;}
	//Current Image attributes
	int w=m_Img->width;
	int h=m_Img->height;
	int d=m_Img->depth;	

	double amp=127.0,off=127.0;	//Sine amplitude and offset
	double xrot,yrot;
	double rot_rad=3.141592653589*rot_deg/180.0;
	double xfreq_rad=3.141592653589*xfreq/180.0;
	double yfreq_rad=3.141592653589*yfreq/180.0;
	double xphase_rad=3.141592653589*xphase/180.0;
	double yphase_rad=3.141592653589*yphase/180.0;
	double dSinval=0.0;
	float fSinval=0.0f;
	int iSinval=0;
	short sSinval=0;
	char cSinval=0;
	//Determine correct amplitude to fill bit depth
	if (d==IPL_DEPTH_8U){amp=127.0;off=127.0;}
	if (d==IPL_DEPTH_16U){amp=32767.0;off=32767.0;}
	if (d==IPL_DEPTH_16S){amp=32767;off=0;}
	if (d==IPL_DEPTH_32S){amp=32767;off=32767;}
	if (d==IPL_DEPTH_32F){amp=127.0;off=127.0;}
	
	for (int y=0;y<h;y++){
		for (int x=0;x<w;x++){
			xrot=x*cos(rot_rad)+y*sin(rot_rad);
			yrot=y*cos(rot_rad)-x*sin(rot_rad);
			dSinval=amp*(cos(xrot*xfreq_rad+xphase_rad)*cos(yrot*yfreq_rad+yphase_rad))+off;
			fSinval=(float)dSinval;
			sSinval=(short)dSinval;
			iSinval=(int)dSinval;
			cSinval=(char)dSinval;
			if (d==IPL_DEPTH_8U)iplPutPixel(m_Img,x,y,(void*)&cSinval);
			if (d==IPL_DEPTH_16U)iplPutPixel(m_Img,x,y,(void*)&sSinval);
			if (d==IPL_DEPTH_16S)iplPutPixel(m_Img,x,y,(void*)&sSinval);
			if (d==IPL_DEPTH_32S)iplPutPixel(m_Img,x,y,(void*)&iSinval);
			if (d==IPL_DEPTH_32F)iplPutPixel(m_Img,x,y,(void*)&fSinval);
		}
	}
	return true;
}

bool CIplImg::FillRamp(float xinc,float yinc,float offset)
{
	if (!IsValid()){return false;}
	//Current Image attributes
	int w=m_Img->width;
	int h=m_Img->height;
	int d=m_Img->depth;	
	float fval=0.0f;
	int ival=0;
	short sval=0;
	char cval=0;

	for (int y=0;y<h;y++){
		for (int x=0;x<w;x++){
			fval=(float)((x*xinc)+(y*yinc)+offset);
			sval=(short)fval;
			ival=(int)fval;
			cval=(char)fval;
			if (d==IPL_DEPTH_8U)iplPutPixel(m_Img,x,y,(void*)&cval);
			if (d==IPL_DEPTH_16U)iplPutPixel(m_Img,x,y,(void*)&sval);
			if (d==IPL_DEPTH_16S)iplPutPixel(m_Img,x,y,(void*)&sval);
			if (d==IPL_DEPTH_32S)iplPutPixel(m_Img,x,y,(void*)&ival);
			if (d==IPL_DEPTH_32F)iplPutPixel(m_Img,x,y,(void*)&fval);
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////
// Fill with test pattern.  Have to delete old image first.
bool CIplImg::FillJaehne(void)
{
	if (!IsValid()){return false;}
//	Store Current Image attributes
	int x=m_Img->width;
	int y=m_Img->height;
	int d=m_Img->depth;
//	Destroy image
	Destroy();	
//	Create Jaehne Image of same type/size	
	m_Img=iplCreateImageJaehne(d,x,y);
	if(!m_Img) return false;
	return true;
}


//////////////////////////////////////////////////////////////////////
// In-Place Image processing operations
//////////////////////////////////////////////////////////////////////

bool CIplImg::GBlur3(int iter)
{
	if (iter<1){return false;}
	CIplImg *cpy;
	for (int l=0;l<iter;l++){
		cpy=this->Clone();
		iplFixedFilter(cpy->GetpImg(),m_Img,IPL_GAUSSIAN_3x3);
		delete cpy;
	}

	return true;
}
bool CIplImg::GBlur5(int iter)
{
	if (iter<1){return false;}
	CIplImg *cpy;
	for (int l=0;l<iter;l++){
		cpy=this->Clone();
		iplFixedFilter(cpy->GetpImg(),m_Img,IPL_GAUSSIAN_5x5);
		delete cpy;
	}
	
	return true;
}


//////////////////////////////////////////////////////////////////////
// Arithmetic
//////////////////////////////////////////////////////////////////////

/*
CIplImg * CIplImg::operator=(CIplImg &image) {
	return image.Clone();	
}
*/
/*
//Addition
CIplImg * operator+(CIplImg &image1, CIplImg &image2)
{
	CIplImg *result = image1.BlankCopy();
	iplAdd((IplImage *)image1.pImg, (IplImage *)image2.pImg, (IplImage *)result->pImg);
	return result;
}

void CIplImg::operator+=(CIplImg &image) {
	if(!IsValid()) return;
	iplAdd(m_Img, (IplImage *)image.pImg, m_Img);	
}
void CIplImg::operator+=(float val) {
	if(!IsValid()) return;
	if(Depth()!=IPL_DEPTH_32F) return;
	iplAddSFP(m_Img, m_Img, val);	
}
void CIplImg::operator+=(int val) {
	if(!IsValid()) return;
	if(Depth()!=IPL_DEPTH_32F) return;
	iplAddS(m_Img, m_Img, val);	
}


//Subtraction
CIplImg * operator-(CIplImg &image1, CIplImg &image2)
{
	CIplImg *result = image1.BlankCopy();
	iplSubtract((IplImage *)image1.pImg, (IplImage *)image2.pImg, (IplImage *)result->pImg);
	return result;
}
void CIplImg::operator-=(CIplImg &image) {
	if(!IsValid()) return;
	iplSubtract(m_Img, (IplImage *)image.pImg, m_Img);	
}
void CIplImg::operator-=(float val) {
	if(!IsValid()) return;
	if(Depth()!=IPL_DEPTH_32F) return;
	iplSubtractSFP(m_Img, m_Img, val,false);	
}
void CIplImg::operator-=(int val) {
	if(!IsValid()) return;
	if(Depth()!=IPL_DEPTH_32F) return;
	iplSubtractS(m_Img, m_Img, val, false);	
}

//Multiplication
CIplImg * operator*(CIplImg &image1, CIplImg &image2)
{
	CIplImg *result = image1.BlankCopy();
	iplMultiply(image1.pImg, image2.pImg, result->pImg);
	return result;
}
void CIplImg::operator*=(CIplImg &image) {
	if(!IsValid()) return;
	if(!image.IsSameTypeAs(this)) return;
	iplMultiply(m_Img, image.pImg, m_Img);	
}
void CIplImg::operator*=(float k) {
	if(!IsValid()) return;
	if(Depth()!=IPL_DEPTH_32F) return;
	iplMultiplySFP(m_Img, m_Img, k);	
}
void CIplImg::operator*=(int k) {
	if(!IsValid()) return;
	if(Depth()!=IPL_DEPTH_32F) return;
	iplMultiplyS(m_Img, m_Img, k);	
}

//Division
CIplImg * operator/(CIplImg &image1, CIplImg &image2)
{
	if (image1.Depth() != IPL_DEPTH_32F){return NULL;}
	if (image2.Depth() != IPL_DEPTH_32F){return NULL;}
	if (image1.Channels() != 1){fprintf(pErrorFile,"Division Not supported for Channels>1\n");return NULL;}
	if (image2.Channels() != 1){fprintf(pErrorFile,"Division Not supported for Channels>1\n");return NULL;}

	CIplImg *result = image1.BlankCopy();
	if (!image2.IsSameTypeAs(&image1)){return NULL;}
	
	int x,y;
	float *pres=(float*)result->GetpData();
	float *pnum=(float*)image1.GetpData();
	float *pden=(float*)image2.GetpData();
	int offset=result->WidthStepBytes()/sizeof(float);

	for (y=0;y<result->Height();y++){
	pres=(float*)result->GetpData();
	pnum=(float*)image1.GetpData();
	pden=(float*)image2.GetpData();
	pres+=y*offset;
	pnum+=y*offset;
	pden+=y*offset;
		for (x=0;x<result->Width();x++){
			if (fabs(*(pden))>MIN_DENOM){
				*(pres)=*(pnum)/ *(pden);
			}
			else
			{
				*(pres)=0.0;
			}
			pres++;pnum++;pden++;
		}
	}
	return result;
}

void CIplImg::operator/=(float k) {

	iplMultiplySFP(m_Img, m_Img, (float)(1.0/k));	
}
*/
/*
//////////////////////////////////////////////////////////////
//Used in McGM for thresholded division
void CIplImg::TestDivide(CIplImg *ImageDenom, float fNumThresh,float fDenThresh, CIplImg *ImOutput)
{
	if(!IsValid()) return;
	if (Depth() != IPL_DEPTH_32F){return;}
	if (Channels() != 1){fprintf(pErrorFile,"Division Not supported for Channels>1\n");return;}
	if (ImageDenom==NULL){fprintf(pErrorFile,"Denominator NULL\n");return;}
	if (!ImageDenom->IsSameTypeAs(this)){return;}
	if (ImOutput==NULL) {ImOutput=this;}

	int x,y;
	float *num=(float*)GetpData();
	float *den=(float*)ImageDenom->GetpData();
	float *res=(float*)ImOutput->GetpData();

	int offset=WidthStepBytes()/sizeof(float);

	for (y=0;y<Height();y++){
	num=(float*)GetpData();
	den=(float*)ImageDenom->GetpData();
	res=(float*)ImOutput->GetpData();

	num+=y*offset;
	den+=y*offset;
	res+=y*offset;

		for (x=0;x<Width();x++){
			if (fabs(*(num))>fNumThresh){
				if (fabs(*(den))>fDenThresh){
					*(res)=*(num)/(*(den));
				}
				else
				{
					*(res)=1.0f/fDenThresh;
				}
			}
			else
			{
				*(res)=0.0f;
			}
			num++;den++;res++;
		}
	}
}


//////////////////////////////////////////////////////////////
//Used in McGM for thresholded division
void CIplImg::TestInvert(float fDenThresh, CIplImg *ImOutput)
{
	if(!IsValid()) return;
	if (Depth() != IPL_DEPTH_32F){return;}
	if (Channels() != 1){fprintf(pErrorFile,"Division Not supported for Channels>1\n");return;}
	if (ImOutput==NULL) {ImOutput=this;}

	int x,y;
	float *den=(float*)GetpData();
	float *res=(float*)ImOutput->GetpData();

	int offset=WidthStepBytes()/sizeof(float);

	for (y=0;y<Height();y++){
	den=(float*)GetpData();
	res=(float*)ImOutput->GetpData();

	den+=y*offset;
	res+=y*offset;

		for (x=0;x<Width();x++){
			if (fabs(*den)>fDenThresh){
					*res=1.0f/(*den);
			}
			else
			{
				*res=1.0f/fDenThresh;
			}
			den++;res++;
		}
	}
}
*/


/*
//////////////////////////////////////////////////////////////
//Used in McGM if val at index of test image < threshold, output image at corresponding
//index set to zero
void CIplImg::RoundToZero(float fThresh, CIplImg *ImTest, CIplImg *ImOutput)
{
	if(!IsValid()) return;;
	if (Depth() != IPL_DEPTH_32F){return;}
	if (Channels() != 1){fprintf(pErrorFile,"Division Not supported for Channels>1\n");return;}
	if (ImOutput==NULL) {ImOutput=this;}

	int x,y;

	float *test=(float*)ImTest->GetpData();
	float *val=(float*)GetpData();
	float *res=(float*)ImOutput->GetpData();

	int offset=WidthStepBytes()/sizeof(float);

	for (y=0;y<Height();y++){
	test=(float*)ImTest->GetpData();
	val=(float*)GetpData();
	res=(float*)ImOutput->GetpData();

	val+=y*offset;
	res+=y*offset;
	test+=y*offset;

		for (x=0;x<Width();x++){
			if (fabs(*test)<fThresh){
					*res=0;
			}
			else
			{
				*res=*val;
			}
			val++;res++; test++;
		}
	}
}

*/




//void CIplImg::operator/=(CIplImg &ImageDenom) {
//	TestDivide(&ImageDenom);
//}

/*
///////////////////////////////////////////////
//Mac1
//Multiply Accumulate the input into this image
bool CIplImg::Mac1(CIplImg &AImg,float A)
{
	if(!IsValid()) return false;;
	if (Depth()!=IPL_DEPTH_32F){fprintf(pErrorFile,"***Error:Mac1:Requires Floating Point Output Image\n");return false;}
	if (Channels()!=1){fprintf(pErrorFile,"***Error:Mac1:Requires single channel Images\n");return false;}

	int x,y;
	float *pim=(float*)GetpData();
	float *pima=(float*)AImg.GetpData();

	int yoffset=WidthStepBytes()/sizeof(float);
	int pos=0;

	for (y=0;y<Height();y++){
		pos=y*yoffset;

		for (x=0;x<Width();x++){
			*(pim+pos)+=*(pima+pos)*A;
			pos++;
		}
	}
	return true;
}

////////////////////////////////////////////////////////////////////
//AbstractImage
//Multiply Accumulate function with 2 inputs - overwrites this image
bool CIplImg::Mac2(CIplImg &AImg,CIplImg &BImg,float A,float B)
{
	if(!IsValid()) return false;;
	if (Depth()!=IPL_DEPTH_32F){fprintf(pErrorFile,"***Error:AbstractImage:Requires Floating Point Image\n");return false;}
	if (Channels()!=1){fprintf(pErrorFile,"***Error:AbstractImage:Requires single channel Images\n");return false;}
	if (!IsSameTypeAs(&AImg)){fprintf(pErrorFile,"***Error:AbstractImage:Images not compatible!\n");return false;}	
	if (!AImg.IsSameTypeAs(&BImg)){fprintf(pErrorFile,"***Error:AbstractImage:Images not compatible!\n");return false;}

	int x,y;
	float *pim=(float*)GetpData();
	float *pima=(float*)AImg.GetpData();
	float *pimb=(float*)BImg.GetpData();
	int yoffset=WidthStepBytes()/sizeof(float);
	int pos=0;

	for (y=0;y<Height();y++){
		pos=y*yoffset;

		for (x=0;x<Width();x++){
			*(pim+pos)=*(pima+pos)*A+*(pimb+pos)*B;
			pos++;
		}
	}
	return true;
}

////////////////////////////////////////////////////////////////////
//Mac3
//Multiply Accumulate function with 3 inputs - overwrites this image
bool CIplImg::Mac3(CIplImg &AImg,CIplImg &BImg,CIplImg &CImg,float A,float B,float C)
{
	if(!IsValid()) return false;;
	if (Depth()!=IPL_DEPTH_32F){fprintf(pErrorFile,"***Error:AbstractImage:Requires Floating Point Image\n");return false;}
	if (Channels()!=1){fprintf(pErrorFile,"***Error:AbstractImage:Requires single channel Images\n");return false;}
	if (!IsSameTypeAs(&AImg)){fprintf(pErrorFile,"***Error:AbstractImage:Images not compatible!\n");return false;}	
	if (!AImg.IsSameTypeAs(&BImg)){fprintf(pErrorFile,"***Error:AbstractImage:Images not compatible!\n");return false;}
	if (!AImg.IsSameTypeAs(&CImg)){fprintf(pErrorFile,"***Error:AbstractImage:Images not compatible!\n");return false;}

	int x,y;
	float *pim=(float*)GetpData();
	float *pima=(float*)AImg.GetpData();
	float *pimb=(float*)BImg.GetpData();
	float *pimc=(float*)CImg.GetpData();

	int yoffset=WidthStepBytes()/sizeof(float);
	int pos=0;

	for (y=0;y<Height();y++){
		pos=y*yoffset;

		for (x=0;x<Width();x++){
			*(pim+pos)=*(pima+pos)*A+*(pimb+pos)*B+*(pimc+pos)*C;
			pos++;
		}
	}
	return true;
}
*/
//////////////////////////////////////////////////////////////////////
// Image Statistics functions
//////////////////////////////////////////////////////////////////////
/*
CIpl_RGBA32F  CIplImg::Mean(CRect *r)
{
	CIpl_RGBA32F Mean;
	Mean.a=0.0f;Mean.b=0.0f;Mean.g=0.0f;Mean.r=0.0f;
	if (!IsValid()){return Mean;}
	int x,y;
	long count=0;	
	CIpl_RGBA32F val;
	CRect roi;

	//if user supplied a region of interest then use it
	if (r==NULL){
		roi.top=0;roi.bottom=Height();
		roi.left=0;roi.right=Width();
	}
	else
	{
		roi=*r;
	}

	for (y=roi.top;y<roi.bottom;y++){
		for (x=roi.left;x<roi.right;x++){
			val=GetPixel(x,y);	
			Mean.r+=val.r;
			Mean.g+=val.g;
			Mean.b+=val.b;
			Mean.a+=val.a;
			count++;
		}
	}
	
	Mean.r/=(float)count;
	Mean.g/=(float)count;
	Mean.b/=(float)count;
	Mean.a/=(float)count;
	return (Mean);
}

//Stats computation.
//This function fills in mean, sd, min and max of the channels in
//the image over the specified rectangular region.
void CIplImg::GetStats(CRect *region, CIpl_RGBA32F *usrMean, CIpl_RGBA32F *usrStdDev, CIpl_RGBA32F *usrMin, CIpl_RGBA32F *usrMax)
{
	if (!IsValid()) return;

	CIpl_RGBA32F M=Mean(region);
	CIpl_RGBA32F rgbSigma;rgbSigma.a=0.0f;rgbSigma.r=0.0f;rgbSigma.g=0.0f;rgbSigma.b=0.0f;
	CIpl_RGBA32F rgbMin, rgbMax;

	int x,y;
	long count=0;	
	CIpl_RGBA32F val;
	CRect *roi;
	CRect rcImg = CRect(0,0,Width(),Height());

	//if user supplied a region of interest then use it
	if (region==NULL){
		roi=&rcImg;
	}
	else
	{
		roi=region;
	}

	rgbMin = rgbMax = GetPixel(roi->left, roi->top);

	for (y=roi->top;y<roi->bottom;y++){
		for (x=roi->left;x<roi->right;x++){
			val=GetPixel(x,y);	

			if (val.r < rgbMin.r) rgbMin.r = val.r;
			if (val.g < rgbMin.g) rgbMin.g = val.g;
			if (val.b < rgbMin.b) rgbMin.b = val.b;
			if (val.a < rgbMin.a) rgbMin.a = val.a;

			if (val.r > rgbMax.r) rgbMax.r = val.r;
			if (val.g > rgbMax.g) rgbMax.g = val.g;
			if (val.b > rgbMax.b) rgbMax.b = val.b;
			if (val.a > rgbMax.a) rgbMax.a = val.a;

			val.r-=M.r;val.r*=val.r;
			val.g-=M.g;val.g*=val.g;
			val.b-=M.b;val.b*=val.b;
			val.a-=M.a;val.a*=val.a;
			rgbSigma.r+=val.r;
			rgbSigma.g+=val.g;
			rgbSigma.b+=val.b;
			rgbSigma.a+=val.a;

			count++;
		}
	}
	rgbSigma.r/=(count-1);
	rgbSigma.g/=(count-1);
	rgbSigma.b/=(count-1);
	rgbSigma.a/=(count-1);
	rgbSigma.r=(float)sqrt(rgbSigma.r);
	rgbSigma.g=(float)sqrt(rgbSigma.g);
	rgbSigma.b=(float)sqrt(rgbSigma.b);
	rgbSigma.a=(float)sqrt(rgbSigma.a);

	if (usrMean) memcpy(usrMean, &M, sizeof(CIpl_RGBA32F));
	if (usrStdDev) memcpy(usrStdDev, &rgbSigma, sizeof(CIpl_RGBA32F));
	if (usrMin) memcpy(usrMin, &rgbMin, sizeof(CIpl_RGBA32F));
	if (usrMax) memcpy(usrMax, &rgbMax, sizeof(CIpl_RGBA32F));
}
*/

#endif

//////////////////////////////////////////////////////////////////////
// Basis Set of Steerable filters
//////////////////////////////////////////////////////////////////////


CSteerBasis::CSteerBasis(	const AbstractFilter &prototypeFilter,
							unsigned int order,unsigned int support,float sigma)
{
	m_nOrder=order;
	m_fSigma=sigma;
	m_nSupport=support;
	m_nNumFilterOrder=2;
	m_fDrawContrast=5.0f;

	m_nResponseW=0;
	m_nResponseH=0;

	for (int yo=0;yo<CSTEERBASIS_MAXORDER;yo++){
		for (int xo=0;xo<CSTEERBASIS_MAXORDER;xo++){
			m_FilterSep[yo][xo]=NULL;
			m_Response[yo][xo]=NULL;
			m_RotResponse[yo][xo]=NULL;
			for (int term=0;term<=CSTEERBASIS_MAXORDER;term++){
				m_fSteerWeights[yo][xo][term]=0.0f;
			}
		}
	}
	m_NumFilterSep[0]=NULL;
	m_NumFilterSep[1]=NULL;

	InitFilters(prototypeFilter,m_nOrder,m_nSupport,m_fSigma);
	SteerWeight(0.0);
}


bool CSteerBasis::Copy(const CSteerBasis &sb)
{
	if(this==&sb)
		return false;

	m_fDrawContrast=sb.m_fDrawContrast;
	m_nOrder=sb.m_nOrder;
	m_nSupport=sb.m_nSupport;
	m_nNumFilterOrder=sb.m_nNumFilterOrder;
	m_fSigma=sb.m_fSigma;

	m_nResponseW=sb.m_nResponseW;
	m_nResponseH=sb.m_nResponseH;

	int i=0,j=0;

	for(i=0;i<CSTEERBASIS_MAXORDER;++i)
		for(j=0;j<CSTEERBASIS_MAXORDER;++j)
			if(sb.m_FilterSep[i][j])
				m_FilterSep[i][j]=m_FilterSep[i][j]->NewCopy();
			else
				m_FilterSep[i][j]=NULL;

	for(i=0;i<2;++i)
		if(sb.m_NumFilterSep)
			m_NumFilterSep[i]=m_NumFilterSep[i]->NewCopy();
		else
			m_NumFilterSep[i]=NULL;

	for(i=0;i<CSTEERBASIS_MAXORDER;++i)
		for(j=0;j<CSTEERBASIS_MAXORDER;++j)
			if(sb.m_Response[i][j])
				m_Response[i][j]=sb.m_Response[i][j]->NewCopy();
			else
				m_Response[i][j]=NULL;
		
	for(i=0;i<CSTEERBASIS_MAXORDER;++i)
		for(j=0;j<CSTEERBASIS_MAXORDER;++j)
			if(sb.m_RotResponse[i][j])
				m_RotResponse[i][j]=sb.m_RotResponse[i][j]->NewCopy();
			else
				m_RotResponse[i][j]=NULL;

	memcpy(m_fSteerWeights,sb.m_fSteerWeights,CSTEERBASIS_MAXORDER*CSTEERBASIS_MAXORDER*(CSTEERBASIS_MAXORDER+1));

	return true;
}


CSteerBasis::CSteerBasis(const CSteerBasis &sb)
{
	Copy(sb);
}

CSteerBasis &CSteerBasis::operator=(const CSteerBasis &sb)
{
	if(this!=&sb)
	{
		Copy(sb);
	}
	return *this;
}

//////////////////////////////////////////////////////////////////////
// Allocate Filters (once only on construction)
//////////////////////////////////////////////////////////////////////
bool CSteerBasis::InitFilters(const AbstractFilter &prototypeFilter,
							  unsigned int order,unsigned int support,float sigma)
{
	//printf("CSteerBasis::InitFilters(order=%d,support=%d,sigma=%3.3f)\n",order,support,sigma);
	
	int xo,yo;

	//Create Floating Point Filters
	for (yo=0;yo<=m_nOrder;yo++){
		for (xo=0;xo<=m_nOrder;xo++){
			m_FilterSep[yo][xo]=prototypeFilter.New();
			if(!m_FilterSep[yo][xo]){
				fprintf(pErrorFile,"CSteerBasis::InitFilters(), Error allocating Filter!\n");
				return false;
			}
			else{
				m_FilterSep[yo][xo]->CreateDogSep(support,support,xo,yo,m_fSigma);
			}
		}
	}
	
		//to do: check to see whether null after alloc
	if (m_nNumFilterOrder>1){
		float filt[5]={-0.0833f,+0.666f,0.0,-0.666f,+0.0833f};
		m_NumFilterSep[0]=prototypeFilter.New();
		m_NumFilterSep[0]->CreateSep(5,1,IMG_FLOAT,filt,NULL);
		m_NumFilterSep[1]=prototypeFilter.New();
		m_NumFilterSep[1]->CreateSep(1,5,IMG_FLOAT,NULL,filt);
	}
	else if (m_nNumFilterOrder==1){
		float filt[3]={0.5f,0.0,-0.5f};
		m_NumFilterSep[0]=prototypeFilter.New();
		m_NumFilterSep[0]->CreateSep(3,1,IMG_FLOAT,filt,NULL);
		m_NumFilterSep[1]=prototypeFilter.New();
		m_NumFilterSep[1]->CreateSep(1,3,IMG_FLOAT,NULL,filt);
	}
	else if (m_nNumFilterOrder==0){
		float filt[2]={1.0f,-1.0f};
		m_NumFilterSep[0]=prototypeFilter.New();
		m_NumFilterSep[0]->CreateSep(2,1,IMG_FLOAT,filt,NULL);
		m_NumFilterSep[1]=prototypeFilter.New();
		m_NumFilterSep[1]->CreateSep(1,2,IMG_FLOAT,NULL,filt);
	}

	return true;
}

/*
//////////////////////////////////////////////////////////////////////
// Quantise the DoG Filters
//////////////////////////////////////////////////////////////////////
bool CSteerBasis::QuantiseFilters(int bitdepth)
{
//	printf("CSteerBasis::QuantiseFilters(bitdepth=%d)",bitdepth);
	int xo,yo;
	char temp[100];
	//CString filename;

	for (yo=0;yo<=m_nOrder;yo++){
		for (xo=0;xo<=m_nOrder;xo++){
			if (m_FilterSep[yo][xo]!=NULL){
			m_FilterSep[yo][xo]->QuantiseFilter(bitdepth);
			}
		}
	}


//	for (xo=0;xo<=m_nOrder;xo++){
//		sprintf(temp,"Filter%d.txt",xo);
//		m_FilterSep[0][xo]->Print(temp);
//	}


//	fprintf(stdout,"OK\n");
	return true;
}
*/
//////////////////////////////////////////////////////////////////////
// Deallocate Basis responses and Filters and Free RAM
//////////////////////////////////////////////////////////////////////

bool CSteerBasis::Destroy()
{
	int xo,yo;

		//Destroy Response Images
	for (yo=0;yo<CSTEERBASIS_MAXORDER;yo++){
		for (xo=0;xo<CSTEERBASIS_MAXORDER;xo++){
			if (m_Response[yo][xo]!=NULL) {delete m_Response[yo][xo];}
		}
	}	
	for (yo=0;yo<CSTEERBASIS_MAXORDER;yo++){
		for (xo=0;xo<CSTEERBASIS_MAXORDER;xo++){
			if (m_RotResponse[yo][xo]!=NULL) {delete m_RotResponse[yo][xo];m_RotResponse[yo][xo]=NULL;}
		}
	}	
	for (yo=0;yo<CSTEERBASIS_MAXORDER;yo++){
		for (xo=0;xo<CSTEERBASIS_MAXORDER;xo++){
			if (m_FilterSep[yo][xo]!=NULL) {delete m_FilterSep[yo][xo];m_FilterSep[yo][xo]=NULL;}
		}
	}
	
	if (m_NumFilterSep[0]!=NULL) {delete m_NumFilterSep[0];m_NumFilterSep[0]=NULL;}
	if (m_NumFilterSep[1]!=NULL) {delete m_NumFilterSep[1];m_NumFilterSep[1]=NULL;}

	return true;
}




CSteerBasis::~CSteerBasis()
{
	Destroy();
}


/*
//////////////////////////////////////////////////////////////////////
// Draw - Draw the entire basis set
//
//////////////////////////////////////////////////////////////////////
bool CSteerBasis::OnDraw(CDC *pDC,int nZoom,bool DrawOriented)
{
	CRect drawrect;
	if (pDC==NULL){TRACE("***Error:CSteerBasis::OnDraw() CDC==NULL !\n");return false;}
	if (nZoom==0){nZoom=1;}

	//Draw all responses
	for (int yo=0;yo<=m_nOrder;yo++){
		for (int xo=0;xo<=(m_nOrder-yo);xo++){
			if (!DrawOriented ) 
			{
				if (GetResponse(xo,yo)!=NULL){
					drawrect.top=GetResponse(xo,yo)->Height()*yo*nZoom+yo;
					drawrect.left=GetResponse(xo,yo)->Width()*xo*nZoom+xo;
					drawrect.bottom=drawrect.top+GetResponse(xo,yo)->Height()*nZoom;
					drawrect.right=drawrect.left+GetResponse(xo,yo)->Width()*nZoom;
					if ((xo+yo)>0){
						GetResponse(xo,yo)->MakeViewableImage(-m_fDrawContrast,m_fDrawContrast);
						GetResponse(xo,yo)->OnDraw(pDC,&drawrect);
					}
					else
					{
						//GetResponse(xo,yo)->MakeViewableImage(-m_fDrawContrast,m_fDrawContrast);
						GetResponse(xo,yo)->MakeFullRangeViewableImage();
						GetResponse(xo,yo)->OnDraw(pDC,&drawrect);
					}
				}
			}
			else{
				if (GetOrientedResponse(xo,yo)!=NULL){
					drawrect.top=GetResponse(xo,yo)->Height()*yo*nZoom+yo;
					drawrect.left=GetResponse(xo,yo)->Width()*xo*nZoom+xo;
					drawrect.bottom=drawrect.top+GetResponse(xo,yo)->Height()*nZoom;
					drawrect.right=drawrect.left+GetResponse(xo,yo)->Width()*nZoom;
					if ((xo+yo)>0){
						GetOrientedResponse(xo,yo)->MakeViewableImage(-m_fDrawContrast,m_fDrawContrast);
						GetOrientedResponse(xo,yo)->OnDraw(pDC,&drawrect);
					}
					else
					{
						GetOrientedResponse(xo,yo)->MakeFullRangeViewableImage();
						GetOrientedResponse(xo,yo)->OnDraw(pDC,&drawrect);
					}
				}
			}
		}
	}
	return true;
}
*/

//////////////////////////////////////////////////////////////////////
// Make - Generates the entire basis set
//
// The Response Images are allocated dynamically in this routine if they are required.  
// The first time this is run will be slightly slower because of the allocation.
//////////////////////////////////////////////////////////////////////
bool CSteerBasis::MakeBasis(const AbstractImage &src)
{
	//printf("CSteerBasis::MakeBasis(AbstractImage *src)...\n");
	
	if (src.Width()<2||src.Height()<2){
		fprintf(pErrorFile,"CSteerBasis::MakeBasis(), Image too Small (%d x %d) \n",
							src.Width(),src.Height());
		return false;
	}

	m_nResponseW=src.Width();
	m_nResponseH=src.Height();

	//CStopWatch sw;
	int xo,yo;
	bool status=true;
	//sw.Start();

	//Cycle through all derivatives (previously created filters) 
	for (yo=0;yo<=m_nOrder;yo++){
		for (xo=0;xo<=(m_nOrder-yo);xo++){

				//Check that the Response Images have been allocated
			if (!m_Response[yo][xo]){
				m_Response[yo][xo]=src.NewBlankCopy();	
			}
				//And that if they have already been allocated that they are of the right type.
			else if (!m_Response[yo][xo]->SameType(src)){
				delete m_Response[yo][xo];
				m_Response[yo][xo]=src.NewBlankCopy();	
			}
				//Do the filtering
			status=m_Response[yo][xo]->Convolve(src,*m_FilterSep[yo][xo]);
			//status=m_FilterSep[yo][xo]->Convolve(src->GetpImg(),m_Response[yo][xo]->GetpImg());

			//CIplImg temp;
			//char num[50];
			//sprintf(fnm,"Basis/OrigBasis[%d][%d].bmp",yo,xo);
			//temp.ScaleF(m_Response[yo][xo],0,255);
			//temp.WriteBMP(fnm);

			if (!status)
			{
				fprintf(pErrorFile,"CSteerBasis::MakeBasis(), Convolve Failed\n");
			}
		}
	}	

	//printf("CSteerBasis::Make(AbstractImage *src)...(%3.3fms) OK \n",sw.Read());
	return status;
}

//////////////////////////////////////////////////////////////////////
// MakeBasisFast - Generates the entire basis set
// Uses numerical Derivatives
//
// The Response Images are allocated dynamically in this routine if they are required.  
// The first time this is run will be slightly slower because of the allocation.
//////////////////////////////////////////////////////////////////////
bool CSteerBasis::MakeBasisFast(const AbstractImage &src)
{

	//fdb
	AbstractFilter * afp1 = m_NumFilterSep[1];
	//if (!afp1->m_Kernel){
	//	int a=1;
	//}
//	printf("CSteerBasis::MakeBasisFast(AbstractImage *src)...\n");
	
	if (src.Width()<2||src.Height()<2)
	{
		fprintf(pErrorFile,"CSteerBasis::MakeBasisFast(), Image too Small (%d x %d) \n",
						src.Width(),src.Height());
		return false;
	}

	m_nResponseW=src.Width();
	m_nResponseH=src.Height();

	//CStopWatch sw;
	int xo,yo;
	bool status=true;
//	sw.Start();

	//Blur Image
	if (m_Response[0][0]==NULL){
		m_Response[0][0]=src.NewBlankCopy();		
	}
	else if (!m_Response[0][0]->SameType(src)){
		delete m_Response[0][0];
		m_Response[0][0]=src.NewBlankCopy();	
	}

	//Do the Blur
	//status=m_FilterSep[0][0]->Convolve(src->GetpImg(),m_Response[0][0]->GetpImg());
	status=m_Response[0][0]->Convolve(src,*m_FilterSep[0][0]);
	if (status==false){
		fprintf(pErrorFile,"CSteerBasis::MakeBasisFast(), Blur failed\n");
		return false;
	}

	//Do Y-Derivatives
	for (yo=1;yo<=m_nOrder;yo++){
			//Check that the Response Images have been allocated
			if (m_Response[yo][0]==NULL){
				m_Response[yo][0]=src.NewBlankCopy();		
			}
			//And that if they have already been allocated that they are of the right type.
			else if (!m_Response[yo][0]->SameType(src)){
				delete m_Response[yo][0];
				m_Response[yo][0]=src.NewBlankCopy();	
			}
			
			//Do the filtering
			//status=m_NumFilterSep[1]->Convolve(m_Response[yo-1][0]->GetpImg(),m_Response[yo][0]->GetpImg());
			status=m_Response[yo][0]->Convolve(*m_Response[yo-1][0],*m_NumFilterSep[1]);
			if(!status){
				fprintf(pErrorFile,"CSteerBasis::MakeBasisFast(), Convolve Failed\n");
				return false;
			}
	}

	//Pull-out X Derivatives 
	for (yo=0;yo<=m_nOrder;yo++){
		for (xo=1;xo<=(m_nOrder-yo);xo++){
			
			//Check that the Respone Images have been allocated
			if (m_Response[yo][xo]==NULL){
				m_Response[yo][xo]=src.NewBlankCopy();	
			}
			//And that if they have already been allocated that they are of the right type.
			else if (!m_Response[yo][xo]->SameType(src)){
				delete m_Response[yo][xo];
				m_Response[yo][xo]=src.NewBlankCopy();	
			}
			
			//Do the filtering
			//status=m_NumFilterSep[0]->Convolve(m_Response[yo][xo-1]->GetpImg(),m_Response[yo][xo]->GetpImg());
			status=m_Response[yo][xo]->Convolve(*m_Response[yo][xo-1],*m_NumFilterSep[0]);
			if (!status)
			{
				fprintf(pErrorFile,"CSteerBasis::MakeBasisFast(), Convolve Failed\n");
			}
		}
	}	

//	printf("CSteerBasis::MakeBasisFast(AbstractImage *src)...(%3.3fms) OK \n",sw.Read());
	return status;
}

//////////////////////////////////////////////////////////////////////
// CSteerBasis - Generates an Oriented Basis Set from the Orthogonal
// basis set.  Requires that the function MakeBasis has already been 
// called to generate a basis set
//
// The RotResponse Images are allocated dynamically in this routine if they are required.  
// The first time this is run will be slightly slower because of the allocation.
//////////////////////////////////////////////////////////////////////
bool CSteerBasis::SteerBasis(float rad_angle)
{
	//printf("CSteerBasis::CSteerBasis(float angle)...\n");

	//CStopWatch sw;
	//sw.Start();
	int xo,yo,xt;
	
	AbstractImage *temp=NULL;
	
	//Make Steering Weights
	if (!SteerWeight(rad_angle)){return false;}
				
	for (yo=0;yo<=m_nOrder;yo++){
		for (xo=0;xo<=(m_nOrder-yo);xo++){
			//Check that the Respone Images have been allocated
			if (m_RotResponse[yo][xo]==NULL){
				m_RotResponse[yo][xo]=m_Response[yo][xo]->NewBlankCopy();	
			}
			//And that if they have already been allocated that they are of the right type.
			else if (!m_RotResponse[yo][xo]->SameType(*m_Response[yo][xo])){
				delete m_RotResponse[yo][xo];
				m_RotResponse[yo][xo]=m_Response[yo][xo]->NewBlankCopy();
			}
			
			if(!temp)
				temp=m_Response[yo][xo]->NewBlankCopy();

			m_RotResponse[yo][xo]->Zero();

			//Synthesize Oriented response (except for zero`th order)
			if (xo>0 || yo>0){
				for (int term=0;term<=(xo+yo);term++){
					xt=xo+yo-term;
					if (fabs(m_fSteerWeights[xo][yo][term])>CSTEERBASIS_MINSTEERWEIGHT){
						temp->MultiplyS(*m_Response[term][xt],&m_fSteerWeights[xo][yo][term]);
						m_RotResponse[yo][xo]->Add(*temp);
						//iplMultiplySFP(m_Response[term][xt]->GetpImg(),temp->GetpImg(),m_fSteerWeights[xo][yo][term]);
						//*(m_RotResponse[yo][xo])+=(*temp);						
					}
				}
			}
		}
	}

	delete temp;
	//printf("CSteerBasis::SteerBasis(float angle)...(%3.3fms) OK \n",sw.Read());
	return true;
}

//////////////////////////////////////////////////////////////////////
// CSteerBasis - Generates an Oriented Basis Set from the Orthogonal
// basis set.  Requires that the function MakeBasis has already been 
// called to generate a basis set
//
// The RotResponse Images are allocated dynamically in this routine if they are required.  
// The first time this is run will be slightly slower because of the allocation.
//////////////////////////////////////////////////////////////////////
bool CSteerBasis::SteerBasis(float rad_angle,CSteerBasis *SBdst)
{
	//printf("CSteerBasis::CSteerBasis(float angle,SBDst)...\n");
	
	//CStopWatch sw;
	//sw.Start();

	if(!SBdst){
		fprintf(pErrorFile,"CSteerBasis::SteerBasis(), SBdst arg==NULL\n");
		return false;
	}

	int xo,yo,xt;

	AbstractImage *temp=NULL;

	//Make Steering Weights
	if (!SteerWeight(rad_angle)){return false;}
		
	for (yo=0;yo<=m_nOrder;yo++){
		for (xo=0;xo<=(m_nOrder-yo);xo++){
			//Check that the Response Images have been allocated
			if (!SBdst->GetResponse(xo,yo)){
				SBdst->SetResponse(xo,yo,m_Response[yo][xo]->NewBlankCopy());	
			}
			//And that if they have already been allocated that they are of the right type.
			else if (!SBdst->GetResponse(xo,yo)->SameType(*m_Response[yo][xo])){
				delete SBdst->GetResponse(xo,yo);
				SBdst->SetResponse(xo,yo,m_Response[yo][xo]->NewBlankCopy());
			}
			if (temp==NULL){
				temp=m_Response[yo][xo]->NewBlankCopy();
			}

			SBdst->GetResponse(xo,yo)->Zero();

			//Synthesize Oriented response (except for zero`th order)
			for (int term=0;term<=(xo+yo);term++){
				xt=xo+yo-term;
				if (fabs(m_fSteerWeights[xo][yo][term])>CSTEERBASIS_MINSTEERWEIGHT){
					
					temp->MultiplyS(*m_Response[term][xt],&m_fSteerWeights[xo][yo][term]);
					SBdst->GetResponse(xo,yo)->Add(*temp);
					//iplMultiplySFP(m_Response[term][xt]->GetpImg(),temp->GetpImg(),m_fSteerWeights[xo][yo][term]);
					//*(SBdst->GetResponse(xo,yo))+=(*temp);
					
				}
			}
		}
	}

	SBdst->m_nResponseW=m_nResponseW;
	SBdst->m_nResponseH=m_nResponseH;
	
	delete temp;
	//printf("CSteerBasis::CSteerBasis(float angle)...(%3.3fms) OK \n",sw.Read());
	return true;
}

//In order to rotate the basis set by 90 degrees, we can do this VERY simply
bool CSteerBasis::SteerBasis90(CSteerBasis *SBdst)
{
	//printf("SteerBasis::SteerBasis90(SBDst)...\n");
	
	//CStopWatch sw;
	//sw.Start();

	if(!SBdst){
		fprintf(pErrorFile,"SteerBasis::SteerBasis90(), SBdst arg==NULL\n");
		return false;
	}

	int xo,yo,xt;
	
	AbstractImage *temp=NULL;		

	for (yo=0;yo<=m_nOrder;yo++){
		for (xo=0;xo<=(m_nOrder-yo);xo++){
			//Check that the Response Images have been allocated
			if (!SBdst->GetResponse(xo,yo)){
				SBdst->SetResponse(xo,yo,m_Response[yo][xo]->NewBlankCopy());	
			}
			//And that if they have already been allocated that they are of the right type.
			else if (!SBdst->GetResponse(xo,yo)->SameType(*m_Response[yo][xo])){
				delete SBdst->GetResponse(xo,yo);
				SBdst->SetResponse(xo,yo,m_Response[yo][xo]->NewBlankCopy());
			}
			if (!temp){
				temp=m_Response[yo][xo]->NewBlankCopy();
			}

			SBdst->GetResponse(xo,yo)->Zero();

			//Synthesize Oriented response (except for zero`th order)
			if (xo>0 || yo>0){
				for (int term=0;term<=(xo+yo);term++){
					xt=xo+yo-term;
					
						if (m_fSteerWeights[xo][yo][term]==1.0f){
							SBdst->GetResponse(xo,yo)->Copy(*m_Response[term][xt]);
						}
						else if (m_fSteerWeights[xo][yo][term]==-1.0f){
							SBdst->GetResponse(xo,yo)->Subtract(*m_Response[term][xt]);
						}
					
				}
			}
		}
	}

	SBdst->m_nResponseW=m_nResponseW;
	SBdst->m_nResponseH=m_nResponseH;

	delete temp;
	//printf("SteerBasis::SteerBasis90()...(%3.3fms) OK \n",sw.Read());
	return true;
}


AbstractFilter *CSteerBasis::GetFilter(unsigned int XOrder,unsigned int YOrder) 
{
	if (XOrder+YOrder>m_nOrder){
		fprintf(pErrorFile,"CSteerBasis::GetFilter, Order out of bounds X+Y=%d (%d max)\n",XOrder+YOrder,m_nOrder);
		return NULL;
	}
	return m_FilterSep[YOrder][XOrder];
}

const AbstractFilter *CSteerBasis::GetFilter(unsigned int XOrder,unsigned int YOrder) const 
{
	if (XOrder+YOrder>m_nOrder){
		fprintf(pErrorFile,"CSteerBasis::GetFilter, Order out of bounds X+Y=%d (%d max)\n",XOrder+YOrder,m_nOrder);
		return NULL;
	}
	return m_FilterSep[YOrder][XOrder];
}


//////////////////////////////////////////////////////////////////////
// Get a Result
//////////////////////////////////////////////////////////////////////

AbstractImage *CSteerBasis::GetResponse(unsigned int XOrder,unsigned int YOrder)
{
	if (XOrder+YOrder>m_nOrder){
		fprintf(pErrorFile,"CSteerBasis::GetResponse, Order out of bounds X+Y=%d (%d max)\n",XOrder+YOrder,m_nOrder);
		return NULL;
	}
	return m_Response[YOrder][XOrder];
}


const AbstractImage *CSteerBasis::GetResponse(unsigned int XOrder,unsigned int YOrder) const
{
	if (XOrder+YOrder>m_nOrder){
		fprintf(pErrorFile,"CSteerBasis::GetResponse - Order out of bounds X+Y=%d (%d max)\n",XOrder+YOrder,m_nOrder);
		return NULL;
	}
	return m_Response[YOrder][XOrder];
}


AbstractImage *CSteerBasis::GetOrientedResponse(unsigned int XOrder,unsigned int YOrder)
{
	if (XOrder+YOrder>m_nOrder){
		fprintf(pErrorFile,"CSteerBasis::GetOrientedResponse, Order out of bounds X+Y=%d (%d max)\n",XOrder+YOrder,m_nOrder);
		return NULL;
	}
	//The Zero order is not oriented so get it from the basis set
	if (XOrder==0 && YOrder==0){
		return GetResponse(0,0);	
	}

	if (m_RotResponse[YOrder][XOrder]==NULL){
		fprintf(pErrorFile,"CSteerBasis::GetOrientedResponse, RotResponse(%d,%d)==NULL\n",XOrder,YOrder);
		return NULL;
	}
	return m_RotResponse[YOrder][XOrder];
}

const AbstractImage *CSteerBasis::GetOrientedResponse(unsigned int XOrder, unsigned int YOrder) const
{
	if (XOrder+YOrder>m_nOrder){
		fprintf(pErrorFile,"CSteerBasis::GetOrientedResponse, Order out of bounds X+Y=%d (%d max)\n",XOrder+YOrder,m_nOrder);
		return NULL;
	}
	//The Zero order is not oriented so get it from the basis set
	if (XOrder==0 && YOrder==0){
		return GetResponse(0,0);	
	}

	if (m_RotResponse[YOrder][XOrder]==NULL){
		fprintf(pErrorFile,"CSteerBasis::GetOrientedResponse, RotResponse(%d,%d)==NULL\n",XOrder,YOrder);
		return NULL;
	}
	return m_RotResponse[YOrder][XOrder];
}


//////////////////////////////////////////////////////////////////////
// Set a Result - used when we steer and make a new basis
//////////////////////////////////////////////////////////////////////
bool CSteerBasis::SetResponse(unsigned int XOrder,unsigned int YOrder,AbstractImage *response)
{
	if (XOrder+YOrder>m_nOrder){
		fprintf(pErrorFile,"***Error:SetResponse - Order out of bounds X+Y=%d (%d max)\n",XOrder+YOrder,m_nOrder);
		return false;
	}
	if (m_Response[YOrder][XOrder]!=NULL){delete m_Response[YOrder][XOrder];}

	m_Response[YOrder][XOrder]=response;

	return true;
}


///////////////////////////////////////////////////////////////////////////
//  Mathematical support functions
///////////////////////////////////////////////////////////////////////////

//Steerweight generates ALL the weights for a particular angle
//Checked and verified against hand calculations JDale 25th March
bool CSteerBasis::SteerWeight(float rad_angle)
{
	int BinX,BinY;
	double rad=rad_angle;
	double co=cos(rad);
	double si=sin(rad);
	double xt,yt,xtyt;
	double parity=1.0;
	double w=0;
	int Term;

	string m_WeightFunc[CSTEERBASIS_MAXORDER][CSTEERBASIS_MAXORDER][CSTEERBASIS_MAXORDER+1];
	//FILE *file;file=fopen("debugsteer.txt","at");

	//Clear Weights
	long arraysize=CSTEERBASIS_MAXORDER*CSTEERBASIS_MAXORDER*(CSTEERBASIS_MAXORDER+1);
	ZeroMemory(m_fSteerWeights,sizeof(float)*arraysize);
	
	int XOrder;
	for (XOrder=0;XOrder<=m_nOrder;XOrder++){
		for (int YOrder=0;YOrder<=m_nOrder-XOrder;YOrder++){
			
			for (int y_terms=0;y_terms<=YOrder;y_terms++){
				for (int x_terms=0;x_terms<=XOrder;x_terms++){
					Term=x_terms+y_terms;
					BinX=Binomial(XOrder,x_terms);
					BinY=Binomial(YOrder,y_terms);
					xt=pow(co,XOrder-x_terms)*pow(si,x_terms);
					yt=pow(-1.0*si,YOrder-y_terms)*pow(co,y_terms);
					parity=pow(-1.0,YOrder-y_terms);
					xtyt=xt*yt*BinX*BinY;
					
					m_fSteerWeights[XOrder][YOrder][Term]+=(float)xtyt;
					m_WeightFunc[XOrder][YOrder][Term]+=SteerWeightFunctionString(int(parity*BinX*BinY),XOrder-x_terms+y_terms,x_terms+YOrder-y_terms);
				}
			}
			//If number is very small, it should be exactly zero
			for (int t=0;t<m_nOrder;t++){	
				if (fabs(m_fSteerWeights[XOrder][YOrder][t])<CSTEERBASIS_MINSTEERWEIGHT){m_fSteerWeights[XOrder][YOrder][t]=0.0f;}
			}
		}
	}

	
	for (XOrder=0;XOrder<=m_nOrder;XOrder++){
		for (int YOrder=0;YOrder<=m_nOrder-XOrder;YOrder++){
			for (int t=0;t<=XOrder+YOrder;t++){	
				//TRACE("SteerFunc  (%3.3f) %dX %dY Term %d = %s = %3.5f\n",rad_angle,XOrder,YOrder,t,m_WeightFunc[XOrder][YOrder][t],m_fSteerWeights[XOrder][YOrder][t]);
				//if (XOrder==2 && YOrder==0)
				//fprintf(file,"SteerFunc  (%3.3f) %dX %dY Term %d = %s = %3.5f\n",rad_angle,XOrder,YOrder,t,m_WeightFunc[XOrder][YOrder][t],m_fSteerWeights[XOrder][YOrder][t]);
			}
		}
	}
	
	//fclose(file);
	return true;
}	


string CSteerBasis::SteerWeightFunctionString(int Num,int CosPow,int SinPow)
{
	string TempString;
	char temp[100];
	if (Num==0){TempString="";return TempString;}

	

	if (Num==1){
		if (CosPow==0 && SinPow==0) {sprintf(temp,"%d + ",Num);return TempString=temp;}
		if (CosPow==0 && SinPow==1) {sprintf(temp,"Sin + ");return TempString=temp;}
		if (CosPow==1 && SinPow==0) {sprintf(temp,"Cos + ");return TempString=temp;}
		if (CosPow==1 && SinPow==1) {sprintf(temp,"SinCos + ");return TempString=temp;}
		if (CosPow==0 && SinPow>1) {sprintf(temp,"Sin^%d + ",SinPow);return TempString=temp;}
		if (CosPow>1 && SinPow==0) {sprintf(temp,"Cos^%d + ",CosPow);return TempString=temp;}
		if (CosPow==1 && SinPow>1) {sprintf(temp,"Sin^%dCos + ",SinPow);return TempString=temp;}
		if (CosPow>1 && SinPow==1) {sprintf(temp,"SinCos^%d + ",CosPow);return TempString=temp;}
		sprintf(temp,"Sin^%dCos^%d+",SinPow,CosPow);return TempString=temp;
	}
	else{
		if (CosPow==0 && SinPow==0) {sprintf(temp,"%d + ",Num);return TempString=temp;}
		if (CosPow==0 && SinPow==1) {sprintf(temp,"%dSin + ",Num);return TempString=temp;}
		if (CosPow==1 && SinPow==0) {sprintf(temp,"%dCos + ",Num);return TempString=temp;}
		if (CosPow==1 && SinPow==1) {sprintf(temp,"%dSinCos + ",Num);return TempString=temp;}
		if (CosPow==0 && SinPow>1) {sprintf(temp,"%dSin^%d + ",Num,SinPow);return TempString=temp;}
		if (CosPow>1 && SinPow==0) {sprintf(temp,"%dCos^%d + ",Num,CosPow);return TempString=temp;}
		if (CosPow==1 && SinPow>1) {sprintf(temp,"%dSin^%dCos + ",Num,SinPow);return TempString=temp;}
		if (CosPow>1 && SinPow==1) {sprintf(temp,"%dSinCos^%d + ",Num,CosPow);return TempString=temp;}
		sprintf(temp,"%dSin^%dCos^%d + ",Num,SinPow,CosPow);return TempString=temp;
	}

	sprintf(temp,"Error - String not Caught!!");
	return TempString=temp;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TaylorReconstruct::TaylorReconstruct()
{
	m_nOrder=0;
	m_fXOffset=0.0f;
	m_fYOffset=0.0f;
	m_fSOffset=0.0f;

	//m_fRecErr=0.0f;
	//m_ImOutput=NULL;
	//m_ImInput=NULL;
	//m_ImDiff=NULL;
	//m_ImG0Recon=NULL;
	//m_Basis=NULL;
	//m_XWarp=NULL;
	//m_YWarp=NULL;

	//m_nG0Mode=0;
	//m_bDoThreshBasis=true;	
	//m_fBasisThresh=0.0;
	//m_nImScale=1;
	//m_nViewMode=0;
	//m_bInitOK=false;

	//m_bAnimate=false;
}

TaylorReconstruct::TaylorReconstruct(	unsigned int order,
									float xoffset,float yoffset,float soffset)
{
	Set(order,xoffset,yoffset,soffset);
}

TaylorReconstruct::~TaylorReconstruct()
{
	Destroy();
}

void TaylorReconstruct::Destroy(void)
{
	//if (m_ImInput!=NULL) delete m_ImInput;
	//if (m_ImOutput!=NULL) delete m_ImOutput;
	//if (m_ImDiff!=NULL) delete m_ImDiff;
	//if (m_Basis!=NULL) delete m_Basis;
	//if (m_ImG0Recon!=NULL)delete m_ImG0Recon;
	//if (m_XWarp!=NULL) delete m_XWarp;
	//if (m_YWarp!=NULL) delete m_YWarp;

	//m_bInitOK=false;
}


bool TaylorReconstruct::Set(unsigned int order,
							float xoffset,float yoffset,float soffset)
{
	bool returnstatus=true;

	if(order<TAYLORRECONSTRUCT_MAXORDER){
		m_nOrder=order;
	}
	else{
		fprintf(pErrorFile,
"TaylorReconstruct::Set() order arg (%d) exceeds max allowed (%d), setting to %d\n",
						order, TAYLORRECONSTRUCT_MAXORDER-1,TAYLORRECONSTRUCT_MAXORDER-1);
		m_nOrder=TAYLORRECONSTRUCT_MAXORDER-1;
		returnstatus=false;
	}

	m_fXOffset=xoffset;
	m_fYOffset=yoffset;
	m_fSOffset=soffset;

	return returnstatus;
}


/*
bool TaylorReconstruct::NearestPixel(	unsigned int x,unsigned int y,
									float &basis_x,float &basis_y,
									float &dx,float &dy)
{





}

*/

	//should also check that output and basis images of same type
bool TaylorReconstruct::ParameterCheck(	const AbstractImage &output,const CSteerBasis &basis,
										unsigned int &temporder)
{
	if(!output.IsValid()){
		fprintf(pErrorFile,"TaylorReconstruct::ParameterCheck(), invalid output img arg\n");
		return false;
	}

	if(m_nOrder>basis.GetOrder()){
		fprintf(pErrorFile,
"TaylorReconstruct::ParameterCheck(), member::order (%d) > basis arg order (%d), using %d, order unchanged\n",
		m_nOrder,basis.GetOrder(),basis.GetOrder());
		temporder=basis.GetOrder();
	}
	else
		temporder=m_nOrder;

	return true;
}


/*
bool TaylorReconstruct::GenerateWeightsLUT(unsigned int LUTsz,
										 const float *dx,const float *dy,
										 unsigned int sz)
{
	if(!dx||!dy){
		fprintf(pErrorFile,"TaylorReconstruct::GenerateWeightsLUT(), dx and/or dy arg==NULL\n");
		return false;
	}

	if(m_xLUT&&m_LUTsz!=LUTsz){
		delete [] m_xLUT;
		m+xLUT=NULL;
	}

	if(m_yLUT&&m_LUTsz!=LUTsz){
		delete [] m_yLUT;
		m_yLUT=NULL;
	}

	m_LUTsz=LUTsz;

	try{
		if(!xLUT)
			m_xLUT=new float[m_LUTsz];
		
	}
	catch(bad_alloc xa)
	{
		m_xLUT=NULL;
		return false;
	}

	try{
		if(!yLUT)
			m_yLUT=new float[m_LUTsz];
	}
	catch(bad_alloc xa)
	{
		m_yLUT=NULL;
		return false;
	}


	float max,min;

	maxminf(dx,sz,&max,&min);

	if(
	
	

	

	



}

*/

inline bool pixelAdd(	void *pDst,void *pSrc,float x,
						unsigned int channels,short int dataType)
{
	switch(channels)
	{
		case 1:
		{
			switch(dataType)
			{
				case IMG_UCHAR:
				{
					float val;
					val=*((unsigned char*)pSrc)+x;
					*((unsigned char*)pDst)=val>255 ? 255 : (unsigned char) val;
					return true;
				} break;
				case IMG_LONG:
				{
					float val;
					val=*((long*)pSrc)+x;
					*((long*)pDst)=(long)val;
					return true;
				} break;
				case IMG_FLOAT:
				{
					*((float*)pDst)=*((float*)pSrc)+x;
					return true;
				} break;
				default:
				{
				fprintf(pErrorFile,"pixelAdd(), Unsupported data type (label %d)\n",dataType);
				return false;
				} break;
			}
		} break;
		case 3:
		{
			switch(dataType)
			{
				case IMG_UCHAR:
				{
					float val;
					val=((unsigned char*)pSrc)[0]+x;
					((unsigned char*)pDst)[0]=val>255 ? 255 : (unsigned char) val;
					val=((unsigned char*)pSrc)[1]+x;
					((unsigned char*)pDst)[1]=val>255 ? 255 : (unsigned char) val;
					val=((unsigned char*)pSrc)[2]+x;
					((unsigned char*)pDst)[2]=val>255 ? 255 : (unsigned char) val;
					return true;
				} break;
				case IMG_LONG:	
				{
					float val;
					val=((long*)pSrc)[0]+x;
					((long*)pDst)[0]=(long)val;
					val=((long*)pSrc)[1]+x;
					((long*)pDst)[1]=(long)val;
					val=((long*)pSrc)[2]+x;
					((long*)pDst)[2]=(long)val;
					return true;
				} break;
				case IMG_FLOAT: 
				{
					((float*)pDst)[0]=((float*)pSrc)[0]+x;
					((float*)pDst)[1]=((float*)pSrc)[1]+x;
					((float*)pDst)[2]=((float*)pSrc)[2]+x;
					return true;
				}break;
				default:
				{
				fprintf(pErrorFile,"pixelAdd(), Unsupported data type (label %d)\n",dataType);
				return false;
				} break;
			}
		} break;
		default:
		{
		fprintf(pErrorFile,"pixelAdd(), Unsupported no channels (%d), 1 or 3 accepted\n",dataType);
		return false;
		}
	}
	return true;
}


inline bool pixelSubtract(void *pDst,void *pSrc,float x,
						  unsigned int channels,short int dataType)
{
	switch(channels)
	{
		case 1:
		{
			switch(dataType)
			{
				case IMG_UCHAR:
				{
					float val;
					val=*((unsigned char*)pSrc)-x;
					*((unsigned char*)pDst)=val<0 ? 0 : (unsigned char) val;
					return true;
				} break;
				case IMG_LONG:
				{
					float val;
					val=*((long*)pSrc)-x;
					*((long*)pDst)=(long)val;
					return true;
				} break;
				case IMG_FLOAT:
				{
					*((float*)pDst)=*((float*)pSrc)-x;
					return true;
				} break;
				default:
				{
				fprintf(pErrorFile,"pixelMultiply, Unsupported data type (label %d)\n",dataType);
				return false;
				} break;
			}
		} break;
		case 3:
		{
			switch(dataType)
			{
				case IMG_UCHAR:
				{
					float val;
					val=((unsigned char*)pSrc)[0]-x;
					((unsigned char*)pDst)[0]=val<0 ? 0 : (unsigned char) val;
					val=((unsigned char*)pSrc)[1]-x;
					((unsigned char*)pDst)[1]=val<0 ? 0 : (unsigned char) val;
					val=((unsigned char*)pSrc)[2]-x;
					((unsigned char*)pDst)[2]=val<0 ? 0 : (unsigned char) val;
					return true;
				} break;
				case IMG_LONG:	
				{
					float val;
					val=((long*)pSrc)[0]-x;
					((long*)pDst)[0]=(long)val;
					val=((long*)pSrc)[1]-x;
					((long*)pDst)[1]=(long)val;
					val=((long*)pSrc)[2]-x;
					((long*)pDst)[2]=(long)val;
					return true;
				} break;
				case IMG_FLOAT: 
				{
					((float*)pDst)[0]=((float*)pSrc)[0]-x;
					((float*)pDst)[1]=((float*)pSrc)[1]-x;
					((float*)pDst)[2]=((float*)pSrc)[2]-x;
					return true;
				}break;
				default:
				{
				fprintf(pErrorFile,"pixelMultiply, Unsupported data type (label %d)\n",dataType);
				return false;
				} break;
			}
		} break;
		default:
		{
		fprintf(pErrorFile,"pixelMultiply, Unsupported no channels (%d), 1 or 3 accepted\n",dataType);
		return false;
		}
	}
	return true;
}


	// x assumed to be in range 0 to 1
inline bool pixelMultiply(void *pDst,void *pSrc,float x,
						  unsigned int channels,short int dataType)
{
	switch(channels)
	{
		case 1:
		{
			switch(dataType)
			{
				case IMG_UCHAR:
				{
					float val;
					val=*((unsigned char*)pSrc)*x;
					*((unsigned char*)pDst)=val>255 ? 255 : (unsigned char) val;
					return true;
				} break;
				case IMG_LONG:
				{
					float val;
					val=*((long*)pSrc)*x;
					*((long*)pDst)=(long)val;
					return true;
				} break;
				case IMG_FLOAT:
				{
					*((float*)pDst)=*((float*)pSrc)*x;
					return true;
				} break;
				default:
				{
				fprintf(pErrorFile,"pixelMultiply, Unsupported data type (label %d)\n",dataType);
				return false;
				} break;
			}
		} break;
		case 3:
		{
			switch(dataType)
			{
				case IMG_UCHAR:
				{
					float val;
					val=((unsigned char*)pSrc)[0]*x;
					((unsigned char*)pDst)[0]=val>255 ? 255 : (unsigned char) val;
					val=((unsigned char*)pSrc)[1]*x;
					((unsigned char*)pDst)[1]=val>255 ? 255 : (unsigned char) val;
					val=((unsigned char*)pSrc)[2]*x;
					((unsigned char*)pDst)[2]=val>255 ? 255 : (unsigned char) val;
					return true;
				} break;
				case IMG_LONG:	
				{
					float val;
					val=((long*)pSrc)[0]*x;
					((long*)pDst)[0]=(long)val;
					val=((long*)pSrc)[1]*x;
					((long*)pDst)[1]=(long)val;
					val=((long*)pSrc)[2]*x;
					((long*)pDst)[2]=(long)val;
					return true;
				} break;
				case IMG_FLOAT: 
				{
					((float*)pDst)[0]=((float*)pSrc)[0]*x;
					((float*)pDst)[1]=((float*)pSrc)[1]*x;
					((float*)pDst)[2]=((float*)pSrc)[2]*x;
					return true;
				}break;
				default:
				{
				fprintf(pErrorFile,"pixelMultiply, Unsupported data type (label %d)\n",dataType);
				return false;
				} break;
			}
		} break;
		default:
		{
		fprintf(pErrorFile,"pixelMultiply, Unsupported no channels (%d), 1 or 3 accepted\n",dataType);
		return false;
		}
	}
	return true;
}



	// x assumed to be in range 0 to 1
inline bool pixelAdd(void *pDst,void *pSrc,void *pSrc2,
					unsigned int channels,short int dataType)
{
	switch(channels)
	{
		case 1:
		{
			switch(dataType)
			{
				case IMG_UCHAR:
				{
					float val;
					val=*((unsigned char*)pSrc)+ *((unsigned char*)pSrc2);
					*((unsigned char*)pDst)=val>255 ? 255 : (unsigned char) val;
					return true;
				} break;
				case IMG_LONG:
				{
					float val;
					val=*((long*)pSrc)+ *((long*)pSrc2);
					*((long*)pDst)=(long)val;
					return true;
				} break;
				case IMG_FLOAT:
				{
					*((float*)pDst)=*((float*)pSrc)+ *((float*)pSrc2);
					return true;
				} break;
				default:
				{
				fprintf(pErrorFile,"pixelMultiply, Unsupported data type (label %d)\n",dataType);
				return false;
				} break;
			}
		} break;
		case 3:
		{
			switch(dataType)
			{
				case IMG_UCHAR:
				{
					float val;
					val=((unsigned char*)pSrc)[0]+((unsigned char*)pSrc2)[0];
					((unsigned char*)pDst)[0]=val>255 ? 255 : (unsigned char) val;
					val=((unsigned char*)pSrc)[1]+((unsigned char*)pSrc2)[1];
					((unsigned char*)pDst)[1]=val>255 ? 255 : (unsigned char) val;
					val=((unsigned char*)pSrc)[2]+((unsigned char*)pSrc2)[2];
					((unsigned char*)pDst)[2]=val>255 ? 255 : (unsigned char) val;
					return true;
				} break;
				case IMG_LONG:	
				{
					float val;
					val=((long*)pSrc)[0]+((long*)pSrc2)[0];
					((long*)pDst)[0]=(long)val;
					val=((long*)pSrc)[1]+((long*)pSrc2)[1];
					((long*)pDst)[1]=(long)val;
					val=((long*)pSrc)[2]+((long*)pSrc2)[2];
					((long*)pDst)[2]=(long)val;
					return true;
				} break;
				case IMG_FLOAT: 
				{
					((float*)pDst)[0]=((float*)pSrc)[0]+((float*)pSrc2)[0];
					((float*)pDst)[1]=((float*)pSrc)[1]+((float*)pSrc2)[1];
					((float*)pDst)[2]=((float*)pSrc)[2]+((float*)pSrc2)[2];
					return true;
				}break;
				default:
				{
				fprintf(pErrorFile,"pixelAdd(), Unsupported data type (label %d)\n",dataType);
				return false;
				} break;
			}
		} break;
		default:
		{	
			fprintf(pErrorFile,"pixelAdd(), Unsupported no channels (%d), 1 or 3 currently accepted\n",dataType);
			return false;
		}
	}
	return true;
}

	// x assumed to be in range 0 to 1
inline bool pixelSubtract(void *pDst,void *pSrc,void *pSrc2,
						  unsigned int channels,short int dataType)
{
	switch(channels)
	{
		case 1:
		{
			switch(dataType)
			{
				case IMG_UCHAR:
				{
					float val;
					val=*((unsigned char*)pSrc)- *((unsigned char*)pSrc2);
					*((unsigned char*)pDst)=val<0 ? 0 : (unsigned char) val;
					return true;
				} break;
				case IMG_LONG:
				{
					float val;
					val=*((long*)pSrc)- *((long*)pSrc2);
					*((long*)pDst)=(long)val;
					return true;
				} break;
				case IMG_FLOAT:
				{
					*((float*)pDst)=*((float*)pSrc)-*((float*)pSrc2);
					return true;
				} break;
				default:
				{
				fprintf(pErrorFile,"pixelSubtract(), Unsupported data type (label %d)\n",dataType);
				return false;
				} break;
			}
		} break;
		case 3:
		{
			switch(dataType)
			{
				case IMG_UCHAR:
				{
					float val;
					val=((unsigned char*)pSrc)[0]-((unsigned char*)pSrc2)[0];
					((unsigned char*)pDst)[0]=val<0 ? 0 : (unsigned char) val;
					val=((unsigned char*)pSrc)[1]-((unsigned char*)pSrc2)[1];
					((unsigned char*)pDst)[1]=val<0 ? 0 : (unsigned char) val;
					val=((unsigned char*)pSrc)[2]-((unsigned char*)pSrc2)[2];
					((unsigned char*)pDst)[2]=val<0 ? 0 : (unsigned char) val;
					return true;
				} break;
				case IMG_LONG:	
				{
					float val;
					val=((long*)pSrc)[0]-((long*)pSrc2)[0];
					((long*)pDst)[0]=(long)val;
					val=((long*)pSrc)[1]-((long*)pSrc2)[1];
					((long*)pDst)[1]=(long)val;
					val=((long*)pSrc)[2]-((long*)pSrc2)[2];
					((long*)pDst)[2]=(long)val;
					return true;
				} break;
				case IMG_FLOAT: 
				{
					((float*)pDst)[0]=((float*)pSrc)[0]-((float*)pSrc2)[0];
					((float*)pDst)[1]=((float*)pSrc)[1]-((float*)pSrc2)[1];
					((float*)pDst)[2]=((float*)pSrc)[2]-((float*)pSrc2)[2];
					return true;
				}break;
				default:
				{
				fprintf(pErrorFile,"pixelMultiply(), Unsupported data type (label %d)\n",dataType);
				return false;
				} break;
			}
		} break;
		default:
		{
		fprintf(pErrorFile,"pixelMultiply, Unsupported no channels (%d), 1 or 3 currently accepted\n",dataType);
		return false;
		}
	}
	return true;
}


	// x assumed to be in range 0 to 1
inline bool pixelMultiply(void *pDst,void *pSrc,void *pSrc2,
						  unsigned int channels,short int dataType)
{
	switch(channels)
	{
		case 1:
		{
			switch(dataType)
			{
				case IMG_UCHAR:
				{
					float val;
					val=*((unsigned char*)pSrc)* *((unsigned char*)pSrc2);
					*((unsigned char*)pDst)=val>255 ? 255 : (unsigned char) val;
					return true;
				} break;
				case IMG_LONG:
				{
					float val;
					val=*((long*)pSrc)* *((long*)pSrc2);
					*((long*)pDst)=(long)val;
					return true;
				} break;
				case IMG_FLOAT:
				{
					*((float*)pDst)=*((float*)pSrc)* *((float*)pSrc2);
					return true;
				} break;
				default:
				{
				fprintf(pErrorFile,"pixelMultiply, Unsupported data type (label %d)\n",dataType);
				return false;
				} break;
			}
		} break;
		case 3:
		{
			switch(dataType)
			{
				case IMG_UCHAR:
				{
					float val;
					val=((unsigned char*)pSrc)[0]*((unsigned char*)pSrc2)[0];
					((unsigned char*)pDst)[0]=val>255 ? 255 : (unsigned char) val;
					val=((unsigned char*)pSrc)[1]*((unsigned char*)pSrc2)[1];
					((unsigned char*)pDst)[1]=val>255 ? 255 : (unsigned char) val;
					val=((unsigned char*)pSrc)[2]*((unsigned char*)pSrc2)[2];
					((unsigned char*)pDst)[2]=val>255 ? 255 : (unsigned char) val;
					return true;
				} break;
				case IMG_LONG:	
				{
					float val;
					val=((long*)pSrc)[0]*((long*)pSrc2)[0];
					((long*)pDst)[0]=(long)val;
					val=((long*)pSrc)[1]*((long*)pSrc2)[1];
					((long*)pDst)[1]=(long)val;
					val=((long*)pSrc)[2]*((long*)pSrc2)[2];
					((long*)pDst)[2]=(long)val;
					return true;
				} break;
				case IMG_FLOAT: 
				{
					((float*)pDst)[0]=((float*)pSrc)[0]*((float*)pSrc2)[0];
					((float*)pDst)[1]=((float*)pSrc)[1]*((float*)pSrc2)[1];
					((float*)pDst)[2]=((float*)pSrc)[2]*((float*)pSrc2)[2];
					return true;
				}break;
				default:
				{
				fprintf(pErrorFile,"pixelMultiply(), Unsupported data type (label %d)\n",dataType);
				return false;
				} break;
			}
		} break;
		default:
		{	
			fprintf(pErrorFile,"pixelMultiply, Unsupported no channels (%d), 1 or 3 currently accepted\n",dataType);
			return false;
		}
	}
	return true;
}



////////////////////////////////////////////////////
//Taylor Reconstruction with Scale-space stuff
bool TaylorReconstruct::SlowReconstruct(AbstractImage &dst,const CSteerBasis &basis)
{
	unsigned int temporder;

	if(!ParameterCheck(dst,basis,temporder)){
		fprintf(pErrorFile,"TaylorReconstruct::Reconstruct(), Invalid parameters, aborting\n");
		return false;
	}

	int xim,yim;
	int xderv,yderv;
	long nNonZeroPixels=0;
	long nZeroPixels=0;
	
	float x_zoom=(float)dst.Width()/basis.GetResponseWidth();
	float y_zoom=(float)dst.Height()/basis.GetResponseHeight();

	double basis_x=0,basis_y=0;
	double remainder_x=0,remainder_y=0;

	const unsigned int channels=dst.Channels();
	const short int dataType=dst.DataType();
	const unsigned int pixSz=channels*dst.Depth();
	void *pWVal,*pB,*pDstPix;
	try{
		pWVal=(void*) new char[pixSz];
		pB=(void*) new char[pixSz];
		pDstPix=(void*) new char[pixSz];
	}
	catch(bad_alloc xa){
		fprintf(pErrorFile,"TaylorReconstruct::SlowReconstruct(), Error allocating temp memory\n");
		return false;
	}

	dst.Zero();

	for(xim=0;xim<dst.Width();++xim){
		for(yim=0;yim<dst.Height();++yim){
			
			remainder_x=modf(xim/x_zoom,&basis_x);
			remainder_y=modf(yim/y_zoom,&basis_y);

			GenWeights(m_nOrder,(float)remainder_x+m_fXOffset,
								(float)remainder_y+m_fYOffset,m_fSOffset);

			for (yderv=0;yderv<=m_nOrder;yderv++){	
					for (xderv=0;xderv<=(m_nOrder-yderv);xderv++){
							
							basis.GetResponse(xderv,yderv)->Pixel(pB,(int)basis_x,(int)basis_y);
							pixelMultiply(pWVal,pB,m_fWeights[yderv][xderv],channels,dataType);
							dst.Pixel(pDstPix,xim,yim);
							pixelAdd(pDstPix,pDstPix,pWVal,channels,dataType);
							dst.SetPixel(pDstPix,xim,yim);
					}
			}
			
		}
	}
	delete [] pWVal; delete [] pB; delete [] pDstPix;
	
	return true;
}


////////////////////////////////////////////////////
//Taylor Reconstruction with Scale-space stuff
bool TaylorReconstruct::SlowReconstruct(AbstractImage &dst,
									  const CSteerBasis &basis,
									  const float *spvmap_x,const float *spvmap_y)
{
	unsigned int temporder;

	if(!ParameterCheck(dst,basis,temporder)){
		fprintf(pErrorFile,"TaylorReconstruct::Reconstruct(), Invalid parameters, aborting\n");
		return false;
	}

	int xim,yim;
	int xderv,yderv;
	long nNonZeroPixels=0;
	long nZeroPixels=0;
	
	const float basisW=basis.GetResponseWidth();
	const float basisH=basis.GetResponseHeight();

	double basis_x=0,basis_y=0;
	double remainder_x=0,remainder_y=0;
	float map_x=0,map_y=0;

	const short int dataType=dst.DataType();
	const unsigned int channels=dst.Channels();
	const unsigned int pixSz=channels*dst.Depth();
	void *pWVal,*pB,*pDstPix;
	try{
		pWVal=(void*) new char[pixSz];
		pB=(void*) new char[pixSz];
		pDstPix=(void*) new char[pixSz];
	}
	catch(bad_alloc xa){
		fprintf(pErrorFile,"TaylorReconstruct::SlowReconstruct(), Error allocating temp memory\n");
		return false;
	}

	dst.Zero();

	int i=0;
	for(yim=0;yim<dst.Height();++yim){
		for(xim=0;xim<dst.Width();++xim,++i){	
			map_x=spvmap_x[i];//(yim*dst.Width())+xim];
			map_y=spvmap_y[i];//(yim*dst.Width())+xim];

			if(map_x>=0&&map_y>=0&&map_x<basisW&&map_y<basisH)
			{
				remainder_x=modf(map_x,&basis_x);
				remainder_y=modf(map_y,&basis_y);

				GenWeights(m_nOrder,(float)remainder_x,
									(float)remainder_y,m_fSOffset);

				for (yderv=0;yderv<=m_nOrder;yderv++){	
					for (xderv=0;xderv<=(m_nOrder-yderv);xderv++){
							
						basis.GetResponse(xderv,yderv)->Pixel(pB,(int)basis_x,(int)basis_y);
						pixelMultiply(pWVal,pB,m_fWeights[yderv][xderv],channels,dataType);
						dst.Pixel(pDstPix,xim,yim);
						pixelAdd(pDstPix,pDstPix,pWVal,channels,dataType);
						dst.SetPixel(pDstPix,xim,yim);
					}
				}
			}
		}	
	}
	delete [] pWVal; delete [] pB; delete [] pDstPix;

	return true;
}


////////////////////////////////////////////////////
//Taylor Reconstruction with Scale-space stuff
bool TaylorReconstruct::SlowReconstructWarp(AbstractImage &dst,
											const CSteerBasis &basis,
											const float *warp_x,const float *warp_y)
{
	unsigned int temporder;

	if(!ParameterCheck(dst,basis,temporder)){
		fprintf(pErrorFile,"TaylorReconstruct::Reconstruct(), Invalid parameters, aborting\n");
		return false;
	}

	int xim,yim;
	int xderv,yderv;
	long nNonZeroPixels=0;
	long nZeroPixels=0;
	
	const float basisW=basis.GetResponseWidth();
	const float basisH=basis.GetResponseHeight();
	float x_zoom=(float)dst.Width()/basisW;
	float y_zoom=(float)dst.Height()/basisH;

	double basis_x=0,basis_y=0;
	double remainder_x=0,remainder_y=0;
	float map_x=0,map_y=0;

	const short int dataType=dst.DataType();
	const unsigned int channels=dst.Channels();
	const unsigned int pixSz=channels*dst.Depth();
	
	void *pWVal,*pB,*pDstPix;
	try{
		pWVal=(void*) new char[pixSz];
		pB=(void*) new char[pixSz];
		pDstPix=(void*) new char[pixSz];
	}
	catch(bad_alloc xa){
		fprintf(pErrorFile,"TaylorReconstruct::SlowReconstructWarp(), Error allocating temp memory\n");
		return false;
	}

	dst.Zero();

	unsigned int index=0;
	for(yim=0;yim<dst.Height();++yim){
		for(xim=0;xim<dst.Width();++xim,++index){
			//index=(yim*dst.Width())+xim;

			if(map_x>=0 && map_y>=0&& map_x<basisW&&map_y<basisH)
			{
				remainder_x=modf(xim/x_zoom,&basis_x);
				remainder_y=modf(yim/y_zoom,&basis_y);

				GenWeights(m_nOrder,(float)remainder_x+warp_x[index],
									(float)remainder_y+warp_y[index],m_fSOffset);

				for (yderv=0;yderv<=m_nOrder;yderv++){	
					for (xderv=0;xderv<=(m_nOrder-yderv);xderv++){
					
						basis.GetResponse(xderv,yderv)->Pixel(pB,(int)basis_x,(int)basis_y);
						pixelMultiply(pWVal,pB,m_fWeights[yderv][xderv],channels,dataType);
						dst.Pixel(pDstPix,xim,yim);
						pixelAdd(pDstPix,pDstPix,pWVal,channels,dataType);
						dst.SetPixel(pDstPix,xim,yim);
					}
				}
			}
		}	
	}
	delete [] pWVal; delete [] pB; delete [] pDstPix;

	return true;
}


////////////////////////////////////////////////////
//Taylor Reconstruction with Scale-space stuff
bool TaylorReconstruct::SlowReconstruct(AbstractImage &dst,
									  const CSteerBasis &basis,
									  const float *spvmap_x,const float *spvmap_y,
									  const float *warp_x,const float *warp_y)
{
	unsigned int temporder;

	if(!ParameterCheck(dst,basis,temporder)){
		fprintf(pErrorFile,"TaylorReconstruct::Reconstruct(), Invalid parameters, aborting\n");
		return false;
	}

	int xim,yim;
	int xderv,yderv;
	long nNonZeroPixels=0;
	long nZeroPixels=0;
	
	unsigned int basisW=basis.GetResponseWidth();
	unsigned int basisH=basis.GetResponseHeight();

	float x_zoom=(float)dst.Width()/basisW;
	float y_zoom=(float)dst.Height()/basisH;

	double basis_x=0,basis_y=0;
	double remainder_x=0,remainder_y=0;
	float map_x=0,map_y=0;

	const short int dataType=dst.DataType();
	const unsigned int channels=dst.Channels();
	const unsigned int pixSz=channels*dst.Depth();

	void *pWVal,*pB,*pDstPix;
	try{
		pWVal=(void*) new char[pixSz];
		pB=(void*) new char[pixSz];
		pDstPix=(void*) new char[pixSz];
	}
	catch(bad_alloc xa){
		fprintf(pErrorFile,"TaylorReconstruct::SlowReconstruct(), Error allocating temp memory\n");
		return false;
	}

	dst.Zero();

	unsigned int index=0;
	for(yim=0;yim<dst.Height();++yim){
		for(xim=0;xim<dst.Width();++xim,++index){
		
			//index=(yim*dst.Width())+xim;

			map_x=spvmap_x[index];
			map_y=spvmap_y[index];
			
			if(map_x>=0&&map_y>=0&&map_x<basisW&&map_y<basisH)
			{
				remainder_x=modf(map_x,&basis_x);
				remainder_y=modf(map_y,&basis_y);

				GenWeights(m_nOrder,(float)remainder_x+warp_x[index],
									(float)remainder_y+warp_y[index],m_fSOffset);

				for (yderv=0;yderv<=m_nOrder;yderv++){	
					for (xderv=0;xderv<=(m_nOrder-yderv);xderv++){
						basis.GetResponse(xderv,yderv)->Pixel(pB,(int)basis_x,(int)basis_y);
						pixelMultiply(pWVal,pB,m_fWeights[yderv][xderv],channels,dataType);
						dst.Pixel(pDstPix,xim,yim);
						pixelAdd(pDstPix,pDstPix,pWVal,channels,dataType);
						dst.SetPixel(pDstPix,xim,yim);
					}
				}
			}
		}	
	}
	delete [] pWVal; delete [] pB; delete [] pDstPix;

	return true;
}


////////////////////////////////////////////////////
//Taylor Reconstruction with Scale-space stuff
bool TaylorReconstruct::FastReconstruct(AbstractImage &dst,const CSteerBasis &basis)
{
	unsigned int temporder;

	if(!ParameterCheck(dst,basis,temporder)){
		fprintf(pErrorFile,"TaylorReconstruct::FastReconstruct(), Invalid parameters, aborting\n");
		return false;
	}

	float zoom_x=(float)dst.Width()/basis.GetResponseWidth();
	float zoom_y=(float)dst.Height()/basis.GetResponseHeight();


	int maskspace_x=zoom_x;
	int maskspace_y=zoom_y;

	if(maskspace_x<1) maskspace_x=1;
	if(maskspace_y<1) maskspace_y=1;

	if(maskspace_x%2==0) maskspace_x-=1;
	if(maskspace_y%2==0) maskspace_y-=1;

	int maskrange_x=(int)((maskspace_x-1)/2.0);
	int maskrange_y=(int)((maskspace_y-1)/2.0);

	int xminpos=maskrange_x;
	int yminpos=maskrange_y;

	int xim,yim,xbas,ybas,xmp,ymp,xderv,yderv;

	void *pWVal,*pB,*pDstPix;
	const unsigned int channels=dst.Channels();
	const short int dataType=dst.DataType();
	const unsigned int pixSz=channels*dst.Depth();

	try{
		pWVal=(void*) new char[pixSz];
		pB=(void*) new char[pixSz];
		pDstPix=(void*) new char[pixSz];
	}
	catch(bad_alloc xa){
		fprintf(pErrorFile,"TaylorReconstruct::SlowReconstruct(), Error allocating temp memory\n");
		return false;
	}

	dst.Zero();

	//Cycle through all extrapolation positions...
	for (xmp=-maskrange_x;xmp<=maskrange_x;xmp++){
		for (ymp=-maskrange_y;ymp<=maskrange_y;ymp++){
			//Generate a set of weights for this position for all orders

			GenWeights(temporder,(float)((xmp/zoom_x)+m_fXOffset),
								(float)((ymp/zoom_y)+m_fYOffset),m_fSOffset);
			
			for (yim=yminpos;yim+ymp<dst.Height();yim+=maskspace_y){
				for (xim=xminpos;xim+xmp<dst.Width();xim+=maskspace_x){

					xbas=xim/zoom_x; ybas=yim/zoom_y;
					//Cycle through all derivatives...
					for (yderv=0;yderv<=temporder;yderv++){	
						for (xderv=0;xderv<=(temporder-yderv);xderv++){
						
							basis.GetResponse(xderv,yderv)->Pixel(pB,(int)xbas,(int)ybas);
							//fprintf(pErrorFile,"%f %f %f\n", ((float*)pB)[0],((float*)pB)[1],((float*)pB)[2]);
							pixelMultiply(pWVal,pB,m_fWeights[yderv][xderv],channels,dataType);
							dst.Pixel(pDstPix,xim+xmp,yim+ymp);
							pixelAdd(pDstPix,pDstPix,pWVal,channels,dataType);
							dst.SetPixel(pDstPix,xim+xmp,yim+ymp);
						}
					}
					
			}}
	}}
	delete [] pWVal; delete [] pB; delete [] pDstPix;

	return true;
}


////////////////////////////////////////////////////
//Taylor Reconstruction with Scale-space stuff
bool TaylorReconstruct::FastReconstruct(AbstractImage &dst,const CSteerBasis &basis,
										unsigned int MaskSpace)
{
	unsigned int temporder;

	if(!ParameterCheck(dst,basis,temporder)){
		fprintf(pErrorFile,"TaylorReconstruct::FastReconstruct(), Invalid parameters, aborting\n");
		return false;
	}

	if(MaskSpace<1){
		fprintf(pErrorFile,
		"TaylorReconstruct::FastReconstruct() Mask space arg (%d) smaller than minimum allowed (1), setting to 1\n",
				MaskSpace );
		MaskSpace=1;
	}

	if(MaskSpace%2==0){
		fprintf(pErrorFile,
	"TaylorReconstruct::FastReconstruct(), MaskSpace=%d, Expecting odd value, changing to %d\n",
						MaskSpace,MaskSpace-1);
		MaskSpace-=1;
	}

	if(basis.GetResponseWidth()<MaskSpace||basis.GetResponseHeight()<MaskSpace){
		fprintf(pErrorFile,
"TaylorReconstruct::FastReconstruct(), Mask space (%d) too large for basis (%d*%d)\n",
					MaskSpace,basis.GetResponseWidth(),basis.GetResponseHeight());
		return false;
	}

	float zoom_x=(float)dst.Width()/basis.GetResponseWidth();
	float zoom_y=(float)dst.Height()/basis.GetResponseHeight();

	int maskspace_x=MaskSpace*zoom_x;
	int maskspace_y=MaskSpace*zoom_y;

	if(maskspace_x<1) maskspace_x=1;
	if(maskspace_y<1) maskspace_y=1;

	if(maskspace_x%2==0) maskspace_x-=1;
	if(maskspace_y%2==0) maskspace_y-=1;

	int maskrange_x=(int)((maskspace_x-1)/2.0);
	int maskrange_y=(int)((maskspace_y-1)/2.0);

	int xminpos=maskrange_x;
	int yminpos=maskrange_y;

	int xim,yim,xmax,ymax,xmp,ymp,xderv,yderv,xbas,ybas;

	xmax=dst.Width()-maskspace_x;
	ymax=dst.Height()-maskspace_y;
	
	const unsigned int channels=dst.Channels();
	const short int dataType=dst.DataType();
	const unsigned int pixSz=channels*dst.Depth();
	void *pWVal,*pB,*pDstPix;

	try{
		pWVal=(void*) new char[pixSz];
		pB=(void*) new char[pixSz];
		pDstPix=(void*) new char[pixSz];
	}
	catch(bad_alloc xa){
		fprintf(pErrorFile,"TaylorReconstruct::FastReconstruct(), Error allocating temp memory\n");
		return false;
	}

	dst.Zero();

	//Cycle through all extrapolation positions...
	for (xmp=-maskrange_x;xmp<=maskrange_x;xmp++){
		for (ymp=-maskrange_y;ymp<=maskrange_y;ymp++){
			//Generate a set of weights for this position for all orders
			
			GenWeights(temporder,(float)((xmp/zoom_x)+m_fXOffset),
								(float)((ymp/zoom_y)+m_fYOffset),m_fSOffset);
			
			for (yim=yminpos;yim<ymax;yim+=maskspace_y){
				for (xim=xminpos;xim<xmax;xim+=maskspace_x){

					xbas=xim/zoom_x; ybas=yim/zoom_y;
					//Cycle through all derivatives...
					for (yderv=0;yderv<=temporder;yderv++){	
						for (xderv=0;xderv<=(temporder-yderv);xderv++){
						
							basis.GetResponse(xderv,yderv)->Pixel(pB,(int)xbas,(int)ybas);
							pixelMultiply(pWVal,pB,m_fWeights[yderv][xderv],channels,dataType);
							dst.Pixel(pDstPix,xim+xmp,yim+ymp);
							pixelAdd(pDstPix,pDstPix,pWVal,channels,dataType);
							dst.SetPixel(pDstPix,xim+xmp,yim+ymp);
					}}
			}}
	}}
	delete [] pWVal; delete [] pB; delete [] pDstPix;

	return true;
}


/* Assuming basis and output same size

////////////////////////////////////////////////////
//Taylor Reconstruction with Scale-space stuff
bool TaylorReconstruct::FastReconstruct2(AbstractImage &output,const CSteerBasis &basis)
{
	unsigned int temporder;

	if(!ParameterCheck(output,basis,temporder)){
		fprintf(pErrorFile,"TaylorReconstruct::FastReconstruct(), Invalid parameters, aborting\n");
		return false;
	}

	if(output.Width()!=basis.GetResponseWidth()||output.Height()!=basis.GetResponseHeight()){
		fprintf(pErrorFile,
"TaylorReconstruct::FastReconstruct(), Expcting basis (%d*%d) and output (%d*%d) to have same dimension\n",
					basis.GetResponseWidth(),basis.GetResponseHeight(),output.Width(),output.Height());
		return false;
	}

	int xim,yim,xmp,ymp,xderv,yderv;
	float val,outval,wval;
	
	int maskrange=(int)((m_nMaskSpace-1)/2.0);
	//int xminpos=maskrange,xmaxpos=basis.GetResponseWidth()-maskrange;
	//int yminpos=maskrange,ymaxpos=basis.GetResponseHeight()-maskrange;

	int xminpos=maskrange,xmaxpos=output.Width()-maskrange;
	int yminpos=maskrange,ymaxpos=output.Height()-maskrange;

	output.Zero();

	//Cycle through all extrapolation positions...
	for (xmp=-maskrange;xmp<=maskrange;xmp++){
		for (ymp=-maskrange;ymp<=maskrange;ymp++){
			//Generate a set of weights for this position for all orders
			GenWeights(temporder,(float)(xmp+m_fXOffset),(float)(ymp+m_fYOffset),m_fSOffset);
			
			//cycle through all image positions...
			for (yim=yminpos;yim+ymp<output.Height();yim+=m_nMaskSpace){
				for (xim=xminpos;xim+xmp<output.Width();xim+=m_nMaskSpace){
							
					//Cycle through all derivatives...
					for (yderv=0;yderv<=temporder;yderv++){	
						for (xderv=0;xderv<=(temporder-yderv);xderv++){
						
							val=basis.GetResponse(xderv,yderv)->GetPixel(xim,yim).r;
												
							outval=output.GetPixel(xim+xmp,yim+ymp).r;
							
							wval=val*m_fWeights[yderv][xderv];
							
							outval+=wval;
							output.SetPixel(xim+xmp,yim+ymp,&outval);
					}}
			}}
	}}

	//TRACE("Reconstruct Time=%3.3fms\n",sw.Read());
	return true;
}
*/



/*


////////////////////////////////////////////////////
//Taylor Reconstruction with Scale-space stuff
bool TaylorReconstruct::FastReconstruct(	AbstractImage &output,const CSteerBasis &basis,
										unsigned int MaskSpace,
										float xoffset,float yoffset,float soffset)
{
	//TRACE("TaylorReconstruct::Reconstruct(%d, %3.1f, %3.1f)\n",MaskSpace,xoffset,yoffset);
	if(MaskSpace<1){MaskSpace=1;}
	

	if(!output.IsValid()){
		fprintf(pErrorFile,"TaylorReconstruct::Reconstruct(), invalid output img arg\n");
		return false;
	}

	if(basis.GetResponseWidth()<m_nMaskSpace||basis.GetResponseHeight()<m_nMaskSpace){
		fprintf(pErrorFile,
"TaylorReconstruct::Reconstruct(), Mask space (%d) too large for basis (%d*%d)\n",
					m_nMaskSpace,basis.GetResponseWidth(),basis.GetResponseHeight());
		return false;
	}

	//if (order<0){order=0;}
	//if (order>MAX_ORDER){TRACE("***Error:Order too high!\n");return false;}
	//if (stddev<0.1){stddev=0.1f;}
	//if (m_ImInput==NULL){TRACE("***Error:No Image\n");return false;}
	
			//Set Member Vars
	m_nOrder=basis.GetOrder();
	m_nMaskSpace=MaskSpace;
	m_fXOffset=xoffset;
	m_fYOffset=yoffset;
	m_fSOffset=soffset;
	
	int xim,yim;
	int xmp,ymp;
	int xderv,yderv;
	float val;
	float outval;
	float wval;
	long nNonZeroPixels=0;
	long nZeroPixels=0;
	int xminpos=m_nMaskSpace,xmaxpos=basis.GetResponseWidth()-m_nMaskSpace;
	int yminpos=m_nMaskSpace,ymaxpos=basis.GetResponseHeight()-m_nMaskSpace;
	int maskrange=(int)((m_nMaskSpace-1)/2.0);
	
	//CStopWatch sw;
	//sw.Start();

	//Basis params changed then make new basis
	//MakeTaylor(order,stddev);

	//Clear Output
	output.Zero();

	//Cycle through all extrapolation positions...
	for (xmp=-maskrange;xmp<=maskrange;xmp++){
		for (ymp=-maskrange;ymp<=maskrange;ymp++){
			//Generate a set of weights for this position for all orders
			GenWeights(m_nOrder,(float)(xmp+m_fXOffset),(float)(ymp+m_fYOffset),m_fSOffset);
			
			//cycle through all image positions...
			for (yim=yminpos;yim<ymaxpos;yim+=m_nMaskSpace){
				for (xim=xminpos;xim<xmaxpos;xim+=m_nMaskSpace){
							
					//Cycle through all derivatives...
					for (yderv=0;yderv<=m_nOrder;yderv++){	
						for (xderv=0;xderv<=(m_nOrder-yderv);xderv++){
						
							val=basis.GetResponse(xderv,yderv)->GetPixel(xim,yim).r;
												
							outval=output.GetPixel(xim+xmp,yim+ymp).r;
							
							wval=val*m_fWeights[yderv][xderv];
							
							outval+=wval;
							output.SetPixel(xim+xmp,yim+ymp,&outval);
					}}
			}}
	}}

	//TRACE("Reconstruct Time=%3.3fms\n",sw.Read());
	return true;
}
*/



/*
//////////////////////////////////////////////////
// Generate a Warp Image
//
bool TaylorReconstruct::MakeWarpField(void)
{
	int x,y;

	float amp=2.0;
	float wx=0,wy=0;
	float freq=10.0*3.14159/180.0;
	static long count=0;

	int xc=(int)(m_XWarp->Width()-1.0)/2.0;
	int yc=(int)(m_XWarp->Height()-1.0)/2.0;

	for (y=2;y<m_XWarp->Height()-2;y++){
		for (x=2;x<m_XWarp->Width()-2;x++){
			
			wx=amp*sin(count*freq+y*freq/2.0);
			wy=0;

			m_XWarp->SetPixel(x,y,&wx);
			m_YWarp->SetPixel(x,y,&wy);
		
		}
	}

	count++;
	return true;
}
*/

/*
///////////////////////////////////////////////
float TaylorReconstruct::ReconstructWarp(AbstractImage *XWarp, AbstractImage *YWarp)
{
	TRACE("TaylorReconstruct::ReconstructWarp\n");
	if (m_ImInput==NULL){TRACE("***Error:No Image\n");return false;}
	if (m_Basis==NULL){TRACE("***Error:No Basis\n");return false;}
	if (XWarp==NULL){XWarp=m_XWarp;}
	if (YWarp==NULL){YWarp=m_YWarp;}

	int xim,yim;
	int xmp,ymp;
	int xderv,yderv;
	float val;
	float outval;
	float wval;
	float xoffset,yoffset;
	long nNonZeroPixels=0;
	long nZeroPixels=0;
	int xminpos=m_nMaskSpace,xmaxpos=m_ImInput->Width()-m_nMaskSpace;
	int yminpos=m_nMaskSpace,ymaxpos=m_ImInput->Height()-m_nMaskSpace;
	int maskrange=(int)((m_nMaskSpace-1)/2.0);
	
	CStopWatch sw;
	sw.Start();


	//Clear Output
	m_ImOutput->Zero();

	//Cycle through all extrapolation positions...
	for (xmp=-maskrange;xmp<=maskrange;xmp++){
		for (ymp=-maskrange;ymp<=maskrange;ymp++){
			//Generate a set of weights for this position for all orders
				
			//cycle through all image positions...
			for (yim=yminpos;yim<ymaxpos;yim+=m_nMaskSpace){
				for (xim=xminpos;xim<xmaxpos;xim+=m_nMaskSpace){
					
					xoffset=XWarp->GetPixel(xim,yim).r;
					yoffset=YWarp->GetPixel(xim,yim).r;
					GenWeights(m_nOrder,(float)(xmp+xoffset),(float)(ymp+yoffset),m_fSOffset);

					//Cycle through all derivatives...
					for (yderv=0;yderv<=m_nOrder;yderv++){	
						for (xderv=0;xderv<=(m_nOrder-yderv);xderv++){
						
							val=m_Basis->GetResponse(xderv,yderv)->GetPixel(xim,yim).r;
												
							outval=m_ImOutput->GetPixel(xim+xmp,yim+ymp).r;
							
							wval=val*m_fWeights[yderv][xderv];
							
							outval+=wval;
							
							m_ImOutput->SetPixel(xim+xmp,yim+ymp,&outval);
					}}
			}}
	}}

	TRACE("Reconstruct Time=%3.3fms\n",sw.Read());
	return 0.0;
}


////////////////////////////////////////////////////////////////
// Visual System may not have access to luminance values so need
// to guess them from derivative information
bool TaylorReconstruct::ReconstructG0(int nMode)
{
	if (m_Basis==NULL){TRACE ("ReconstructG0:No Basis!\n");return false;}
	if (m_ImG0Recon!=NULL){delete m_ImG0Recon;m_ImG0Recon=NULL;}
	RGBA32F val[4];

	//Reconstruct from original data 
	if (m_nG0Mode==0)
	{
		m_ImG0Recon=m_Basis->GetResponse(0,0)->Clone();
	}
	if (m_nG0Mode==1)
	{
		m_ImG0Recon=m_Basis->GetResponse(0,0)->BlankCopy();
		m_ImG0Recon->Fill(127);
		for (int yim=2;yim<m_ImG0Recon->Height()-2;yim++){
			for (int xim=2;xim<m_ImG0Recon->Width()-2;xim++){
				val[0]=m_Basis->GetResponse(0,0)->GetPixel(xim-1,yim);
				val[1]=m_Basis->GetResponse(1,0)->GetPixel(xim+1,yim);
				val[2]=m_Basis->GetResponse(1,0)->GetPixel(xim-1,yim);
				val[3].r=val[0].r+val[2].r/1.5;
				val[3].r = val[3].r < 0 ? 0:val[3].r;
				val[3].r = val[3].r >255 ? 255:val[3].r;
				m_ImG0Recon->SetPixel(xim,yim,&val[3].r);
			}
		}
	}
	return true;
}

  */
/*
////////////////////////////////////////////////////////////////
//GenWeights() Calculate the weight for each derivative order
//
//The first weight is for the taylor expansion, and is standard.
//The second weight is the scale-space extrapolation weight and is 
//based on the diffusion equation, detailed in Floracks paper.



bool TaylorReconstruct::GenWeights(int maxorder,float xoffset,float yoffset,float soffset)
{
	if (maxorder>=TAYLORRECONSTRUCT_MAXORDER){fprintf(pErrorFile,"***Error:GenWeights() - Maxorder exceeded\n");return false;}
	float WX,WY,WS;
	float FX,FY,FS;

	float Weight=0.0f;
	int xd,yd,sd,sdx,sdy,i;
	float binsd;
	//CStopWatch sw;
	//sw.Start();

	//Clear Weights
	for (xd=0;xd<=maxorder;xd++){
	for (yd=0;yd<=maxorder;yd++){
		m_fWeights[yd][xd]=0.0f;
	}
	}

	//Generate Taylor Weights
	int sdm=m_nOrder/2;
	for (sd=0;sd<=m_nOrder/2;sd++){

		for (yd=0;yd<=m_nOrder;yd++){	
			for (xd=0;xd<=(m_nOrder-yd);xd++){
				
				WX=(float)pow(xoffset,(double)xd);
				WY=(float)pow(yoffset,(double)yd);
				WS=(float)pow(soffset,(double)sd);
				FX=(float)fac(xd);
				FY=(float)fac(yd);
				FS=(float)fac(sd);
				
				for (i=0;i<=sd;i++){
					sdx=2*i;
					sdy=2*(sd-i);
					binsd=(float)binomial(i,sd);
					//if (yd+sdy==4){TRACE("current m_fWeights[yd+sdy=%d][xd+sdx=%d] = %f\n",yd+sdy,xd+sdx,m_fWeights[yd+sdy][xd+sdx]);}
					m_fWeights[yd+sdy][xd+sdx] += binsd*(WX*WY*WS) / (FX*FY*FS);
				}
				//m_fWeights[yd][xd] += (WX*WY) / (FX*FY);
			}
		}

	}
	
//	TRACE("GenWeights %3.3f\n",sw.Read());
	return true;
}

*/


bool TaylorReconstruct::GenWeights(int maxorder,float xoffset,float yoffset,float soffset)
{
	if (maxorder>=TAYLORRECONSTRUCT_MAXORDER){
		fprintf(pErrorFile,"TaylorReconstruct::GenWeights(), Maxorder exceeded\n");return false;}
	float WX,WY,WS;
	float FX,FY,FS;

	float Weight=0.0f;
	int xd,yd,sd,sdx,sdy,i;
	float binsd;
//	CStopWatch sw;
//	sw.Start();

	
	//Clear Weights
	for (xd=0;xd<=maxorder;xd++){
	for (yd=0;yd<=maxorder;yd++){
		m_fWeights[yd][xd]=0.0f;
	}
	}
	
	//Generate Taylor Weights
	int sdm=m_nOrder/2;
	for (sd=0;sd<=m_nOrder/2;sd++){

		for (yd=0;yd<=m_nOrder;yd++){	
			for (xd=0;xd<=(m_nOrder-yd);xd++){
				
				WX=(float)pow(xoffset,(float)xd);
				WY=(float)pow(yoffset,(float)yd);
				WS=(float)pow(soffset,(float)sd);
				FX=(float)Fac(xd);
				FY=(float)Fac(yd);
				FS=(float)Fac(sd);
				
				for (i=0;i<=sd;i++){
					sdx=2*i;
					sdy=2*(sd-i);
					//aja change binomial
					binsd=(float)Binomial(sd,i);
					//if (yd+sdy==4){TRACE("current m_fWeights[yd+sdy=%d][xd+sdx=%d] = %f\n",yd+sdy,xd+sdx,m_fWeights[yd+sdy][xd+sdx]);}
					m_fWeights[yd+sdy][xd+sdx] += binsd*(WX*WY*WS) / (FX*FY*FS);
				}
				//m_fWeights[yd][xd] += (WX*WY) / (FX*FY);
			}
		}

	}
	
	//TRACE("GenWeights %3.3f\n",sw.Read());
	return true;
}

/*

//Drawing Routine
void TaylorReconstruct::OnDraw(CDC *pDC)
{
	if (pDC==NULL) {return;}	

	CRect r(0,0,1,1);
	int Drawmode=0;
	if (!m_bInitOK){return;}

	if (m_nViewMode==0){
	if (m_ImInput!=NULL){
		r.top=0;r.bottom=m_ImInput->Height()*m_nImScale;
		r.left=0;r.right=m_ImInput->Width()*m_nImScale;
		m_ImInput->MakeViewableImage(0,255);
		m_ImInput->OnDraw(pDC,&r);
	}
	
	if (m_Basis!=NULL){
	if (m_Basis->GetResponse(0,0)!=NULL){
		r.left=r.right+1;
		r.right=r.left+m_Basis->GetResponse(0,0)->Width()*m_nImScale;
		m_Basis->GetResponse(0,0)->MakeViewableImage(0,255);
		m_Basis->GetResponse(0,0)->OnDraw(pDC,&r);
	}
	}
	
	if (m_ImOutput!=NULL){
		r.left=0;r.right=m_ImOutput->Width()*m_nImScale;
		r.top=r.bottom+1;r.bottom=r.top+m_ImOutput->Height()*m_nImScale;
		m_ImOutput->MakeViewableImage(0,255);
		m_ImOutput->OnDraw(pDC,&r);
	}

	
	if (m_ImDiff!=NULL){
		r.left=r.right+1;
		r.right=r.left+m_ImDiff->Width()*m_nImScale;
		m_ImDiff->MakeViewableImage(0,32.0f);
		m_ImDiff->OnDraw(pDC,&r);
	}

	CString str;str.Format("Mean Diff=%3.3f ",m_fRecErr);
	pDC->TextOut(r.left+3,r.bottom+10,str);
	}

	if (m_nViewMode==1){
		if (m_Basis!=NULL){
			m_Basis->SetContrastRange(64.0);
			m_Basis->OnDraw(pDC);
		}
	}
	
	if (m_nViewMode==2)
	{
		if (m_ImOutput!=NULL){
			r.top=0;r.bottom=m_ImInput->Height()*m_nImScale;
			r.left=0;r.right=m_ImInput->Width()*m_nImScale;
			m_ImOutput->MakeViewableImage(0,255);
			m_ImOutput->OnDraw(pDC,&r);
		}
	}

	
}

*/


///////////////////////////////////////////////////////////////////////////////////
// McGM Version 2005______________________________________________________________
///////////////////////////////////////////////////////////////////////////////////
// Calculates optic flow from a sequence of abstract images
// Outputs representation of optic flow in terms of two floating point abstract images
// corresponding to the angle and speed of motion at each pixel in the input
// 
// New mcgm follows a similar structure to J.Dale's original IPL McGM class

#define MCGMTRACEFILE "debuglog.txt"
#define MCGMIPLCHK if (iplGetErrStatus()!=IPL_StsOk){fprintf(pErrorFile,"***IPL ERROR!\n");}

//Products
const short int MCGM_XX=0; 
const short int MCGM_XY=1; 
const short int MCGM_XT=2; 
const short int MCGM_YY=3; 
const short int MCGM_YT=4; 
const short int MCGM_TT=5; 
//Quotients
const short int MCGM_XT_XX=0; 
const short int MCGM_XT_TT=1; 
const short int MCGM_YT_YY=2; 
const short int MCGM_YT_TT=3; 
const short int MCGM_XY_XX=4; 
const short int MCGM_XY_YY=5; 

	// new products
const short int NXX=0;
const short int NXY=1;
const short int NXT=2;
const short int NYY=3;
const short int NYT=4;
const short int NTT=5;

const short int XT=0;
const short int YT=1;
const short int XY=2;

	// inner products (of outer product)
const short int XY_XY=0;
const short int XT_YT=1;
const short int YT_XY=2;
const short int XT_XY=3;
const short int XT_XT=4;
const short int YT_YT=5;

//Views
const short int MCGM_VIEW_INPUT=0;
const short int MCGM_VIEW_TFILT=1;
const short int MCGM_VIEW_BASIS=2;
const short int MCGM_VIEW_STEER=3;
const short int MCGM_VIEW_TPROD=4;
const short int MCGM_VIEW_TQUOT=5;
const short int MCGM_VIEW_SPMAT=6;
const short int MCGM_VIEW_SPDET=7;
const short int MCGM_VIEW_OPFLW=9;
const short int MCGM_VIEW_CURFRAME=10;

//Sub Views
const short int MCGM_VIEWEX_DEFAULT=0;
const short int MCGM_VIEWEX_TORDER_0=0;
const short int MCGM_VIEWEX_TORDER_1=1;
const short int MCGM_VIEWEX_TORDER_2=2;

const short int MCGM_VIEWEX_TPROD_ALL=10;
const short int MCGM_VIEWEX_TPROD_XX=0;
const short int MCGM_VIEWEX_TPROD_XY=1;
const short int MCGM_VIEWEX_TPROD_XT=2;
const short int MCGM_VIEWEX_TPROD_YY=3;
const short int MCGM_VIEWEX_TPROD_YT=4;
const short int MCGM_VIEWEX_TPROD_TT=5;

const short int MCGM_VIEWEX_TQUOT_ALL=10;
const short int MCGM_VIEWEX_TQUOT_XT_XX=0;
const short int MCGM_VIEWEX_TQUOT_XT_TT=1;
const short int MCGM_VIEWEX_TQUOT_YT_YY=2;
const short int MCGM_VIEWEX_TQUOT_YT_TT=3;
const short int MCGM_VIEWEX_TQUOT_XY_XX=4;
const short int MCGM_VIEWEX_TQUOT_XY_YY=5;

const short int MCGM_VIEWEX_SPMAT_TOP=0;
const short int MCGM_VIEWEX_SPMAT_BOTTOM=1;
const short int MCGM_VIEWEX_SPDET_TOP=0;
const short int MCGM_VIEWEX_SPDET_BOTTOM=1;

const short int MCGM_SYNTHETIC_GRATING=0;
const short int MCGM_SYNTHETIC_PLAID=1;
const short int MCGM_SYNTHETIC_IMPULSE=2;
const short int MCGM_SYNTHETIC_IMTRANS=3;

const short int MCGM_TIMER_PROCESS=0;
const short int MCGM_TIMER_TFILT=1;
const short int MCGM_TIMER_XFILT=2;
const short int MCGM_TIMER_STEER=3;
const short int MCGM_TIMER_TPROD=4;
const short int MCGM_TIMER_TQUOT=5;
const short int MCGM_TIMER_COMPV=6;
const short int MCGM_TIMER_SPMAT=7;
const short int MCGM_TIMER_CMASK=8;
const short int MCGM_TIMER_RNDOF=9;
const short int MCGM_TIMER_DRAW=10;
const short int MCGM_TIMER_GRAB=11;

/*
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#pragma message(MCGMVERSIONSTRING)
*/


//aja miscellanious static functions, included here to avoid too many headers
//function bodies at end of file

static double w_pow(double x, double y);
static int fac(int x);
static float dotlength(float *a, float *b, int length);
static bool GetFrameList(	list <AbstractImage*> &FrmList,
							int number,
							AbstractImage **ImgList);
static float format0_2pi_f(float angle_rad);
static float angle_rad_f( float x, float y );
static void anglesrelativetoradii(	float *radout_p, const float *radin_p, 
									unsigned int w, unsigned int h,
									float originx, float originy );
static void thresholdanglecolours(	unsigned char *angs_uc,const float *vels_f,
							unsigned int w,unsigned int h,
							float minvel);
static void thresholdanglecolours(	unsigned char *angs_uc,const unsigned char *vels,
									unsigned int w,unsigned int h,
									unsigned char minvel);
static bool addborder(unsigned char *img,
							unsigned int w,unsigned int h,
							unsigned int colours,
							unsigned int bordersz, BYTE val);
	//returns false if the label is not in the file
static bool ffwdtolineafterlabel(string &label,FILE *inFile_p);

	//for error checking, assumes floating point IPL image
static bool containsNan(AbstractImage *img);
static bool containsNegative(AbstractImage *img);


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

McGM2005::McGM2005()
{
	int i,j,a;
	m_bInitOK=false;
	/////////////////////////
	//Initialise the Critical Parameters structure with defaults....
	/////////////////////////
	McGMParams.m_nSize=sizeof(McGM2005CriticalParameters);
	//Input
	
	// aja removed
	//McGMParams.m_nSubSampleInput=1;		//Subsample before processing 
	
	//Set Model Default Parameters
	McGMParams.m_nAngles=24;
	McGMParams.m_nTSupportSize=23;
	McGMParams.m_nXSupportSize=23;
	McGMParams.m_nTOrders=2;
	McGMParams.m_nXOrders=5;
	McGMParams.m_nYOrders=2;
	McGMParams.m_fTau=0.2f;
	McGMParams.m_fAlpha=10.0f;
	McGMParams.m_fSigma=1.5;
	McGMParams.m_nIntegrationZone=11;
	//Thresholds
	McGMParams.m_fQuotThreshDen=0.1f;
	McGMParams.m_fQuotThreshNum=0.1f;
	//Angle Wheel border size
	McGMParams.m_nBorder=10;						//Angle Wheel Border
	McGMParams.m_bFastSpatialFiltering=false;		//Numerical Fitlering
	//Recursive Blur size
	m_fRecAlpha=5.0f;
	m_fMaskThresh=1.0f;
	
		//aja col wheel removed
	//m_ColWheel=NULL;

	/////////////////////////
	//Set pointers to NULL
	for (i=0;i<10;i++){m_fTimer[i]=0.0f;}
	rad_angles=NULL;
	sn=cn=n_sn=n_cn=NULL;
	m_ImTResponse[0]=m_ImTResponse[1]=m_ImTResponse[2]=NULL;
	m_TFilters[0]=m_TFilters[1]=m_TFilters[2]=NULL;
	
	for (a=0;a<McGM2005_MAXANGLES;a++){
		m_Basis[a][0]=m_Basis[a][1]=m_Basis[a][2]=NULL;

		for (i=0;i<6;i++){
			m_ImTaylorProd[a][i]=NULL;
			m_ImTaylorQuot[a][i]=NULL;
				
				//aja new
			m_ImTaylorOuterProd[a][i]=NULL;
		}
	}

	//aja new
	for(i=0;i<6;++i)
		m_ImTempTaylorProd[i]=NULL;

	for(i=0;i<3;++i)
		for(int j=0;j<3;++j)
			m_ImTempOuterProd[i][j]=NULL;

	m_ImTempNew=NULL;
	m_ImTempNew2=NULL;
	m_ImDimensionIndex=NULL;
	//end aja new

	for (i=0;i<2;i++){
		for (j=0;j<2;j++){
			m_ImMSpeed[i][j]=NULL;
			m_ImMSpeed[i+2][j]=NULL;

			m_ImMatrices[0][i][j]=NULL;
			m_ImMatrices[1][i][j]=NULL;
			m_ImMatrices[2][i][j]=NULL;
			m_ImMatrices[3][i][j]=NULL;
		}
	}

	for (i=0;i<8;i++){m_ImDetIms[i]=NULL;}
	m_ImOpFlow[0]=m_ImOpFlow[1]=NULL;
	//m_ImOpFlowRender=NULL;
	m_ImTempFP=NULL;
	m_ImMask=NULL;

	for (i=0;i<12;i++){m_fTimer[i]=0.0f;}//Zero Timers
}

McGM2005::McGM2005(const McGM2005 &mcgm)
{
		McGMParams=mcgm.McGMParams;
		m_fRecAlpha=mcgm.m_fRecAlpha;
		m_bDoMask=mcgm.m_bDoMask;
		m_fMaskThresh=mcgm.m_fMaskThresh;
		m_fRecAlpha=mcgm.m_fRecAlpha;
		m_bDoMask=mcgm.m_bDoMask;
		m_fMaskThresh=mcgm.m_fMaskThresh;
		m_bBlurQuotients=mcgm.m_bBlurQuotients;

		m_bInitOK=mcgm.m_bInitOK;

		if (mcgm.sn)
		{	sn=new float[McGMParams.m_nAngles];
			memcpy(sn,mcgm.sn,McGMParams.m_nAngles*sizeof(float));
		}
		else sn=NULL;

		if (mcgm.cn)
		{
			cn=new float[McGMParams.m_nAngles];
			memcpy(cn,mcgm.cn,McGMParams.m_nAngles*sizeof(float));
		}
		else cn=NULL;

		if (mcgm.n_sn)
		{
			n_sn=new float[McGMParams.m_nAngles];
			memcpy(n_sn,mcgm.n_sn,McGMParams.m_nAngles*sizeof(float));
		}
		else
			n_sn=NULL;
		
		if (mcgm.n_cn)
		{
			n_cn=new float[McGMParams.m_nAngles];
			memcpy(n_cn,mcgm.n_cn,McGMParams.m_nAngles*sizeof(float));
		}
		else
			n_cn=NULL;

		if (mcgm.rad_angles)
		{
			rad_angles=new float[McGMParams.m_nAngles];
			memcpy(rad_angles,mcgm.rad_angles,McGMParams.m_nAngles*sizeof(float));
		}
		else
			rad_angles=NULL;

		memcpy(&m_fWeight_Matrix,&mcgm.m_fWeight_Matrix,8000*sizeof(float));
	
		m_FrmBuf=mcgm.m_FrmBuf;

		//if(mcgm.m_ColWheel)
		//{
		//	m_ColWheel=new ColWheelLUT;
		//	*m_ColWheel=*mcgm.m_ColWheel;
		//}
		//else
		//	m_ColWheel=NULL;

		int i=0,j=0,k=0;
	
		for(i=0;i<3;++i)
			if(mcgm.m_ImTResponse[i])
				m_ImTResponse[i]=mcgm.m_ImTResponse[i]->NewCopy();
			else
				m_ImTResponse[i]=NULL;

		for(i=0;i<McGM2005_MAXANGLES;++i)
			for(j=0;j<6;++j)
				if(mcgm.m_ImTaylorProd[i][j])
					m_ImTaylorProd[i][j]=mcgm.m_ImTaylorProd[i][j]->NewCopy();
				else
					m_ImTaylorProd[i][j]=NULL;

		for(i=0;i<6;++i)
			if(mcgm.m_ImTempTaylorProd[i])
				m_ImTempTaylorProd[i]=mcgm.m_ImTempTaylorProd[i]->NewCopy();
			else
				m_ImTempTaylorProd[i]=NULL;

		if(mcgm.m_ImTempNew)
			m_ImTempNew=mcgm.m_ImTempNew->NewCopy();
		else
			m_ImTempNew=NULL;

		if(mcgm.m_ImTempNew2)
			m_ImTempNew2=mcgm.m_ImTempNew2->NewCopy();
		else
			m_ImTempNew2=NULL;

		for(i=0;i<3;++i)
			for(j=0;j<3;++j)
			if(mcgm.m_ImTempOuterProd[i][j])
				m_ImTempOuterProd[i][j]=mcgm.m_ImTempOuterProd[i][j]->NewCopy();
			else
				m_ImTempOuterProd[i][j]=NULL;
	
		for(i=0;i<McGM2005_MAXANGLES;++i)
			for(j=0;j<6;++j)
			if(mcgm.m_ImTaylorOuterProd[i][j])
				m_ImTaylorOuterProd[i][j]=mcgm.m_ImTaylorOuterProd[i][j]->NewCopy();
			else
				m_ImTaylorOuterProd[i][j]=NULL;
	
		if(mcgm.m_ImDimensionIndex)
			m_ImDimensionIndex=mcgm.m_ImDimensionIndex->NewCopy();
		else
			m_ImDimensionIndex=NULL;
		
		for(i=0;i<McGM2005_MAXANGLES;++i)
			for(j=0;j<6;++j)
			if(mcgm.m_ImTaylorQuot[i][j])
				m_ImTaylorQuot[i][j]=mcgm.m_ImTaylorQuot[i][j]->NewCopy();
			else
				m_ImTaylorQuot[i][j]=NULL;
		
		for(i=0;i<4;++i)
			for(j=0;j<2;++j)
				for(k=0;k<2;++k)
					if(mcgm.m_ImMatrices[i][j][k])
						m_ImMatrices[i][j][k]=mcgm.m_ImMatrices[i][j][k]->NewCopy();
					else
						m_ImMatrices[i][j][k]=NULL;
		
		
		for(i=0;i<4;++i)
			for(j=0;j<2;++j)
			if(mcgm.m_ImMSpeed[i][j])
				m_ImMSpeed[i][j]=mcgm.m_ImMSpeed[i][j]->NewCopy();
			else
				m_ImMSpeed[i][j]=NULL;

		for(i=0;i<2;++i)
			if(mcgm.m_ImOpFlow[i])
				m_ImOpFlow[i]=mcgm.m_ImOpFlow[i]->NewCopy();
			else
				m_ImOpFlow[i]=NULL;

		if(mcgm.m_ImTempFP)
			m_ImTempFP=mcgm.m_ImTempFP->NewCopy();
		else
			m_ImTempFP=NULL;

		if(mcgm.m_ImMask)
			m_ImMask=mcgm.m_ImMask->NewCopy();
		else
			m_ImMask=NULL;

		for(i=0;i<8;++i)
			if(mcgm.m_ImDetIms[i])
				m_ImDetIms[i]=mcgm.m_ImDetIms[i]->NewCopy();
			else
				m_ImDetIms[i]=NULL;

		for(i=0;i<3;++i)
			if(mcgm.m_TFilters[i]){
				m_TFilters[i]=mcgm.m_TFilters[i]->NewCopy();
				//*m_TFilters[i]=*mcgm.m_TFilters[i];
			}
		else
			m_TFilters[i]=NULL;

		for(i=0;i<McGM2005_MAXANGLES;++i)
			for(j=0;j<3;++j)
			if(mcgm.m_Basis[i][j])
			{
				m_Basis[i][j]=new CSteerBasis(*mcgm.m_Basis[i][j]);
			}
			else 
				m_Basis[i][j]=NULL;
	
}
	
McGM2005 &McGM2005::operator=(const McGM2005 &mcgm)
{

	if(this!=&mcgm)
	{
		
		McGMParams=mcgm.McGMParams;
		m_fRecAlpha=mcgm.m_fRecAlpha;
		m_bDoMask=mcgm.m_bDoMask;
		m_fMaskThresh=mcgm.m_fMaskThresh;
		m_fRecAlpha=mcgm.m_fRecAlpha;
		m_bDoMask=mcgm.m_bDoMask;
		m_fMaskThresh=mcgm.m_fMaskThresh;
		m_bBlurQuotients=mcgm.m_bBlurQuotients;

		m_bInitOK=mcgm.m_bInitOK;

		if (mcgm.sn)
		{	sn=new float[McGMParams.m_nAngles];
			memcpy(sn,mcgm.sn,McGMParams.m_nAngles*sizeof(float));
		}
		else sn=NULL;

		if (mcgm.cn)
		{
			cn=new float[McGMParams.m_nAngles];
			memcpy(cn,mcgm.cn,McGMParams.m_nAngles*sizeof(float));
		}
		else cn=NULL;

		if (mcgm.n_sn)
		{
			n_sn=new float[McGMParams.m_nAngles];
			memcpy(n_sn,mcgm.n_sn,McGMParams.m_nAngles*sizeof(float));
		}
		else
			n_sn=NULL;
		
		if (mcgm.n_cn)
		{
			n_cn=new float[McGMParams.m_nAngles];
			memcpy(n_cn,mcgm.n_cn,McGMParams.m_nAngles*sizeof(float));
		}
		else
			n_cn=NULL;

		if (mcgm.rad_angles)
		{
			rad_angles=new float[McGMParams.m_nAngles];
			memcpy(rad_angles,mcgm.rad_angles,McGMParams.m_nAngles*sizeof(float));
		}
		else
			rad_angles=NULL;

		memcpy(&m_fWeight_Matrix,&mcgm.m_fWeight_Matrix,8000*sizeof(float));
	
		m_FrmBuf=mcgm.m_FrmBuf;

		//if(mcgm.m_ColWheel)
		//{
		//	m_ColWheel=new ColWheelLUT;
		//	*m_ColWheel=*mcgm.m_ColWheel;
		//}
		//else
		//	m_ColWheel=NULL;

		int i=0,j=0,k=0;
	
		for(i=0;i<3;++i)
			if(mcgm.m_ImTResponse[i])
				m_ImTResponse[i]=mcgm.m_ImTResponse[i]->NewCopy();
			else
				m_ImTResponse[i]=NULL;

		for(i=0;i<McGM2005_MAXANGLES;++i)
			for(j=0;j<6;++j)
				if(mcgm.m_ImTaylorProd[i][j])
					m_ImTaylorProd[i][j]=mcgm.m_ImTaylorProd[i][j]->NewCopy();
				else
					m_ImTaylorProd[i][j]=NULL;

		for(i=0;i<6;++i)
			if(mcgm.m_ImTempTaylorProd[i])
				m_ImTempTaylorProd[i]=mcgm.m_ImTempTaylorProd[i]->NewCopy();
			else
				m_ImTempTaylorProd[i]=NULL;

		if(mcgm.m_ImTempNew)
			m_ImTempNew=mcgm.m_ImTempNew->NewCopy();
		else
			m_ImTempNew=NULL;

		if(mcgm.m_ImTempNew2)
			m_ImTempNew2=mcgm.m_ImTempNew2->NewCopy();
		else
			m_ImTempNew2=NULL;

		for(i=0;i<3;++i)
			for(j=0;j<3;++j)
			if(mcgm.m_ImTempOuterProd[i][j])
				m_ImTempOuterProd[i][j]=mcgm.m_ImTempOuterProd[i][j]->NewCopy();
			else
				m_ImTempOuterProd[i][j]=NULL;
	
		for(i=0;i<McGM2005_MAXANGLES;++i)
			for(j=0;j<6;++j)
			if(mcgm.m_ImTaylorOuterProd[i][j])
				m_ImTaylorOuterProd[i][j]=mcgm.m_ImTaylorOuterProd[i][j]->NewCopy();
			else
				m_ImTaylorOuterProd[i][j]=NULL;
	
		if(mcgm.m_ImDimensionIndex)
			m_ImDimensionIndex=mcgm.m_ImDimensionIndex->NewCopy();
		else
			m_ImDimensionIndex=NULL;
		
		for(i=0;i<McGM2005_MAXANGLES;++i)
			for(j=0;j<6;++j)
			if(mcgm.m_ImTaylorQuot[i][j])
				m_ImTaylorQuot[i][j]=mcgm.m_ImTaylorQuot[i][j]->NewCopy();
			else
				m_ImTaylorQuot[i][j]=NULL;
		
		for(i=0;i<4;++i)
			for(j=0;j<2;++j)
				for(k=0;k<2;++k)
					if(mcgm.m_ImMatrices[i][j][k])
						m_ImMatrices[i][j][k]=mcgm.m_ImMatrices[i][j][k]->NewCopy();
					else
						m_ImMatrices[i][j][k]=NULL;
		
		
		for(i=0;i<4;++i)
			for(j=0;j<2;++j)
			if(mcgm.m_ImMSpeed[i][j])
				m_ImMSpeed[i][j]=mcgm.m_ImMSpeed[i][j]->NewCopy();
			else
				m_ImMSpeed[i][j]=NULL;

		for(i=0;i<2;++i)
			if(mcgm.m_ImOpFlow[i])
				m_ImOpFlow[i]=mcgm.m_ImOpFlow[i]->NewCopy();
			else
				m_ImOpFlow[i]=NULL;

		if(mcgm.m_ImTempFP)
			m_ImTempFP=mcgm.m_ImTempFP->NewCopy();
		else
			m_ImTempFP=NULL;

		if(mcgm.m_ImMask)
			m_ImMask=mcgm.m_ImMask->NewCopy();
		else
			m_ImMask=NULL;

		for(i=0;i<8;++i)
			if(mcgm.m_ImDetIms[i])
				m_ImDetIms[i]=mcgm.m_ImDetIms[i]->NewCopy();
			else
				m_ImDetIms[i]=NULL;

		for(i=0;i<3;++i)
			if(mcgm.m_TFilters[i]){
				m_TFilters[i]=mcgm.m_TFilters[i]->NewCopy();
			}
		else
			m_TFilters[i]=NULL;

		for(i=0;i<McGM2005_MAXANGLES;++i)
			for(j=0;j<3;++j)
			if(mcgm.m_Basis[i][j])
			{
				m_Basis[i][j]=new CSteerBasis(*mcgm.m_Basis[i][j]);
			}
			else 
				m_Basis[i][j]=NULL;
	
	}
	return *this;
}

McGM2005::~McGM2005()
{
	//fprintf(pErrorFile,"McGM Destructor Called...\n");
	Destroy();
	//fprintf(pErrorFile,"McGM Destructor Finished.\n");
}

//////////////////////////////////////////////////////////////////////
// Initialise and reserve RAM
//////////////////////////////////////////////////////////////////////
void McGM2005::Init(McGM2005CriticalParameters *pCPInit,
					const AbstractFilter &prototypeFilter){
	
	//If already Initialised
	Destroy();

	//Change Defaults if a Parameters struct has been passed
	if (pCPInit!=NULL){
		memcpy(&McGMParams,pCPInit,sizeof(McGM2005CriticalParameters));
	}

	//Trig Tables
	if (sn==NULL){sn=new float[McGMParams.m_nAngles];}
	if (cn==NULL){cn=new float[McGMParams.m_nAngles];}
	if (n_sn==NULL){n_sn=new float[McGMParams.m_nAngles];}
	if (n_cn==NULL){n_cn=new float[McGMParams.m_nAngles];}
	if (rad_angles==NULL){rad_angles=new float[McGMParams.m_nAngles];}

	//Initialise the Trig LUTs
	Make_Trig_Tables(McGMParams.m_nAngles);
	//Initialise the Taylor Weights
	Make_Weight_Matrices(McGMParams.m_nIntegrationZone);

	//Allocate the Filters and the Basis sets
	for (int t=0;t<=McGMParams.m_nTOrders;t++){
		m_TFilters[t]=prototypeFilter.New();
		m_TFilters[t]->CreateDolgT(McGMParams.m_nTSupportSize,t,
												McGMParams.m_fAlpha,McGMParams.m_fTau);
		for(int a=0;a<McGMParams.m_nAngles/2;a++){
			m_Basis[a][t]=new CSteerBasis(prototypeFilter,
								McGMParams.m_nXOrders+McGMParams.m_nYOrders,
								McGMParams.m_nXSupportSize,McGMParams.m_fSigma);
		}
	}
	
	//aja
	m_bInitOK=true;
}

//////////////////////////////////////////////////////////////
//Get a copy of the current Critical Parameters.  These can then be
//modified then passed back to the init function
void McGM2005::GetCriticalParams(McGM2005CriticalParameters *cp)
{
	if (cp==NULL){return;}
	memcpy(cp,&McGMParams,sizeof(McGM2005CriticalParameters));
}

//////////////////////////////////////////////////////////////////////
// Deallocate Images and Destroy Objects
//////////////////////////////////////////////////////////////////////
void McGM2005::Destroy(void){

	//TRACE("McGM::Destroy...\n");
	int t;
	//m_bAutoSaveView=false;
	
		//aja
	//if(m_ColWheel)
	//{
	//	delete m_ColWheel;
	//	m_ColWheel=NULL;
	//}

	//Delete the Trig tables
	if (sn!=NULL){delete[] sn;sn=NULL;}
	if (cn!=NULL){delete[] cn;cn=NULL;}
	if (n_sn!=NULL){delete[] n_sn;n_sn=NULL;}
	if (n_cn!=NULL){delete[] n_cn;n_cn=NULL;}
	if (rad_angles!=NULL){delete[] rad_angles;rad_angles=NULL;}
	

	//Delete Filter responses
	for (t=0;t<=McGMParams.m_nTOrders;t++){
		if (m_ImTResponse[t]!=NULL) {delete m_ImTResponse[t];m_ImTResponse[t]=NULL;}
		if (m_TFilters[t]!=NULL) {delete m_TFilters[t];m_TFilters[t]=NULL;}
		
	}
	

	//Delete Taylor expansions and Quotients
	for (int a=0;a<McGM2005_MAXANGLES;a++){
		for (t=0;t<=McGMParams.m_nTOrders;t++){
			if (m_Basis[a][t]!=NULL){delete m_Basis[a][t];m_Basis[a][t]=NULL;}
		}
	
		for (int i=0;i<6;i++){
		if (m_ImTaylorProd[a][i]!=NULL){
			delete m_ImTaylorProd[a][i];
			m_ImTaylorProd[a][i]=NULL;
		}
		if (m_ImTaylorQuot[a][i]!=NULL){
			delete m_ImTaylorQuot[a][i];
			m_ImTaylorQuot[a][i]=NULL;
		}
		if(m_ImTaylorOuterProd[a][i]!=NULL){
			delete m_ImTaylorProd[a][i];
			m_ImTaylorProd[a][i]=NULL;
		}
	}
	}
	
	int i;
	for(i=0;i<6;++i)
	{
		if(m_ImTempTaylorProd[i]!=NULL){
			delete m_ImTempTaylorProd[i];
			m_ImTempTaylorProd[i]=NULL;
		}
	}

	for(i=0;i<3;++i)
	{
		for(int j=0;j<3;++j)
		{
			if(m_ImTempOuterProd[i][j]!=NULL){
				delete m_ImTempOuterProd[i][j];
				m_ImTempOuterProd[i][j]=NULL;
			}
		}
	}
	
	if( m_ImTempNew!=NULL){
		delete m_ImTempNew;
		m_ImTempNew=NULL;
	}

	if( m_ImTempNew2!=NULL){
		delete m_ImTempNew2;
		m_ImTempNew2=NULL;
	}

	if( m_ImDimensionIndex!=NULL){
		delete m_ImDimensionIndex;
		m_ImDimensionIndex=NULL;
	}


	//Delete McGM Matrices
	for (i=0;i<4;i++){
		for (int j=0;j<2;j++){
			for (int k=0;k<2;k++){
				if (m_ImMatrices[i][j][k]!=NULL){
					delete m_ImMatrices[i][j][k];m_ImMatrices[i][j][k]=NULL;	
				}	
			}
		}
	}
	
	for (i=0;i<4;i++){
		for (int j=0;j<2;j++){
			if (m_ImMSpeed[i][j]!=NULL){delete m_ImMSpeed[i][j];}
		}
	}

	//Delete Compute Velocity Temporary images.
	for (i=0;i<8;i++){delete m_ImDetIms[i];m_ImDetIms[i]=NULL;}

	//Delete O Flow
	for (i=0;i<2;i++){
		if (m_ImOpFlow[i]!=NULL){delete m_ImOpFlow[i];m_ImOpFlow[i]=NULL;}
	}

	//Delete Rendered RGB Image
//	if (m_ImOpFlowRender!=NULL){delete m_ImOpFlowRender;m_ImOpFlowRender=NULL;}

	//Delete Misc
	if (m_ImTempFP!=NULL){delete m_ImTempFP;m_ImTempFP=NULL;}
	if (m_ImMask!=NULL){delete m_ImMask;m_ImMask=NULL;}
	
	//TRACE("McGM::Destroy...OK\n");
}

	//aja 18/4/05
bool McGM2005::ReadCriticalParameters(const char *fnm,AbstractFilter &prototypeFilter)
{
	

	int nAngles=24;
	int nTSupportSize=23;
	int nXSupportSize=23;
	int nTOrders=2;
	int nXOrders=5;
	int nYOrders=2;
	float fTau=0.2f;
	float fAlpha=10.0f;
	float fSigma=1.5;
	int nIntegrationZone=11;
	float fQuotThreshDen=0.1f;
	float fQuotThreshNum=0.1f;
	int nBorder=10;					
	bool bFastSpatialFiltering=true;

	if(false){


		FILE *_f = NULL;
	const int LINELENGTH=100;
	char LINE[LINELENGTH];

	_f=fopen(fnm,"r");

	if ( !_f )	
	{
		fprintf(pErrorFile,"McGM2005::ReadCriticalParameters() Error opening %s\n",fnm);
		return false;
	}

	char ReadError[100];
	sprintf(ReadError,
		"McGM2005::ReadCriticalParameters(), Error reading parameters from %s\n",
		fnm);

	string dummy="";

	rewind( _f );
	if(!ffwdtolineafterlabel( dummy+"Angles", _f )){
		fprintf(pErrorFile,ReadError);
		return false;
	}	
	fgets( LINE, LINELENGTH, _f );
	if(sscanf(LINE," %d",&nAngles)!=1){
		fprintf(pErrorFile,ReadError);
		return false;
	}
	
	rewind( _f );
	if(!ffwdtolineafterlabel( dummy+"TSupportSize", _f )){
		fprintf(pErrorFile,ReadError);
		return false;
	}
	fgets( LINE, LINELENGTH, _f );
	if(sscanf(LINE," %d",&nTSupportSize)!=1){
		fprintf(pErrorFile,ReadError);
		return false;
	}

	rewind( _f );
	if(!ffwdtolineafterlabel( dummy+"XSupportSize", _f )){
		fprintf(pErrorFile,ReadError);
		return false;
	}
	fgets( LINE, LINELENGTH, _f );
	if(sscanf(LINE," %d",&nXSupportSize)!=1){
		fprintf(pErrorFile,ReadError);
		return false;
	}

	rewind( _f );
	if(!ffwdtolineafterlabel( dummy+"TOrders", _f )){
		fprintf(pErrorFile,ReadError);
		return false;
	}
	fgets( LINE, LINELENGTH, _f );
	if(sscanf(LINE," %d",&nTOrders)!=1){
		fprintf(pErrorFile,ReadError);
		return false;
	}

	rewind( _f );
	if(!ffwdtolineafterlabel( dummy+"XOrders", _f )){
		fprintf(pErrorFile,ReadError);
		return false;
	}
	fgets( LINE, LINELENGTH, _f );
	if(sscanf(LINE," %d",&nXOrders)!=1){
		fprintf(pErrorFile,ReadError);
		return false;
	}

	rewind( _f );
	if(!ffwdtolineafterlabel( dummy+"YOrders", _f )){
		fprintf(pErrorFile,ReadError);
		return false;
	}
	fgets( LINE, LINELENGTH, _f );
	if(sscanf(LINE," %d",&nYOrders)!=1){
		fprintf(pErrorFile,ReadError);
		return false;
	}

	rewind( _f );
	if(!ffwdtolineafterlabel( dummy+"Tau", _f )){
		fprintf(pErrorFile,ReadError);
		return false;
	}
	fgets( LINE, LINELENGTH, _f );
	if(sscanf(LINE," %f",&fTau)!=1){
		fprintf(pErrorFile,ReadError);
		return false;
	};

	rewind( _f );
	if(!ffwdtolineafterlabel( dummy+"Alpha", _f )){
		fprintf(pErrorFile,ReadError);
		return false;
	}
	fgets( LINE, LINELENGTH, _f );
	if(sscanf(LINE," %f",&fAlpha)!=1){
		fprintf(pErrorFile,ReadError);
		return false;
	}

	rewind( _f );
	if(!ffwdtolineafterlabel( dummy+"Sigma", _f )){
		fprintf(pErrorFile,ReadError);
		return false;
	}
	fgets( LINE, LINELENGTH, _f );
	if(sscanf(LINE," %f",&fSigma)!=1){
		fprintf(pErrorFile,ReadError);
		return false;
	}
	
	rewind( _f );
	if(!ffwdtolineafterlabel( dummy+"IntegrationZone", _f )){
		fprintf(pErrorFile,ReadError);
		return false;
	}
	fgets( LINE, LINELENGTH, _f );
	if(sscanf(LINE," %d",&nIntegrationZone)!=1){
		fprintf(pErrorFile,ReadError);
		return false;
	}
	
	rewind( _f );
	if(!ffwdtolineafterlabel( dummy+"QuotThreshDen", _f )){
		fprintf(pErrorFile,ReadError);
		return false;
	}
	fgets( LINE, LINELENGTH, _f );
	if(sscanf(LINE," %f",&fQuotThreshDen)!=1){
		fprintf(pErrorFile,ReadError);
		return false;
	}
	
	rewind( _f );
	if(!ffwdtolineafterlabel( dummy+"QuotThreshNum", _f )){
		fprintf(pErrorFile,ReadError);
		return false;
	}
	fgets( LINE, LINELENGTH, _f );
	if(sscanf(LINE," %f",&fQuotThreshNum)!=1){
		fprintf(pErrorFile,ReadError);
		return false;
	}

	rewind( _f );		//Angle Wheel border size
	if(!ffwdtolineafterlabel( dummy+"Border", _f )){
		fprintf(pErrorFile,ReadError);
		return false;
	}
	fgets( LINE, LINELENGTH, _f );
	if(sscanf(LINE," %d",&nBorder)!=1){
		fprintf(pErrorFile,ReadError);
		return false;
	}
	
	rewind( _f );
	if(!ffwdtolineafterlabel( dummy+"FastSpatialFiltering", _f )){
		fprintf(pErrorFile,ReadError);
		return false;
	}

	fgets( LINE, LINELENGTH, _f );
	
	if(strstr(LINE,"true"))
		bFastSpatialFiltering=true;
	else if(strstr(LINE,"false"))
		bFastSpatialFiltering=false;
	else{
		fprintf(pErrorFile,ReadError);
		return false;
	}

	fclose(_f);

	}

	McGMParams.m_nAngles=nAngles;
	McGMParams.m_nTSupportSize=nTSupportSize;
	McGMParams.m_nXSupportSize=nXSupportSize;
	McGMParams.m_nTOrders=nTOrders;
	McGMParams.m_nXOrders=nXOrders;
	McGMParams.m_nYOrders=nYOrders;
	McGMParams.m_fTau=fTau;
	McGMParams.m_fAlpha=fAlpha;
	McGMParams.m_fSigma=fSigma;
	McGMParams.m_nIntegrationZone=nIntegrationZone;
	
	McGMParams.m_fQuotThreshDen=fQuotThreshDen;
	McGMParams.m_fQuotThreshNum=fQuotThreshNum;
	
	McGMParams.m_nBorder=nBorder;
	McGMParams.m_bFastSpatialFiltering=bFastSpatialFiltering;

	//Recursive Blur size
	m_fRecAlpha=5.0f;
	m_fMaskThresh=1.0f;
	
	//m_ColWheel=NULL;

	Init(&McGMParams,prototypeFilter);

return true;
}

	//aja 4/4/05
bool McGM2005::Process(const AbstractImage &input)
{
	if(!m_bInitOK)
	{
		fprintf(pErrorFile,"McGM2005::Process(), McGM2005 uninitialised\n"); 
		return false;
	}

	if(!input.IsValid()){
		fprintf(pErrorFile,"McGM2005::Process(), invalid input\n"); 
		return false;
	}
	
	m_FrmBuf.push_back(input.New());

	if(input.Channels()==1&&input.DataType()==IMG_FLOAT)
	{
		m_FrmBuf.back()->Copy(input);
	}
	else{
		fprintf(pErrorFile,
	"McGM2005::Process(), image arg incorrect format, expecting (float,greyscale), Converting\n");
		m_FrmBuf.back()->Create(input.Width(),input.Height(),1,IMG_FLOAT);
		m_FrmBuf.back()->Convert(input);
	}

	unsigned int frmsz=m_FrmBuf.size();
		
	if(frmsz<McGMParams.m_nTSupportSize)
		return false;
	
	if(frmsz>McGMParams.m_nTSupportSize)
	{
		delete *m_FrmBuf.begin();
		m_FrmBuf.erase(m_FrmBuf.begin());
	}
	
	return Process();
}


/*
AbstractImage** McGM2005::OpFlow()
{
	return m_ImOpFlow;
}
*/

bool McGM2005::MaskedOpFlow(AbstractImage &angle,AbstractImage &speed,
							float minSpeed,float speedRange,float speedGreyRange)
{
	if(!m_bInitOK){
		fprintf(pErrorFile,"McGM2005::Process(), McGM2005 uninitialised\n"); 
		return false;
	}
	
	if(!Compute_Mask()){
		fprintf(pErrorFile,"McGM2005::MaskedOpFlow(), Error computing mask\n");
		return false;
	}

	if(!m_ImOpFlow[0]->IsValid()
		|| !m_ImOpFlow[1]->IsValid()){
		fprintf(pErrorFile,"McGM2005::Process(), OpFlow invalid\n"); 
		return false;
	}

	if(!angle.IsValid()||!m_ImOpFlow[1]->SameType(angle)){
		angle.Copy(*m_ImOpFlow[1]);
	}

	if(!speed.IsValid()||!m_ImOpFlow[0]->SameType(angle)){
		speed.BlankCopy(*m_ImOpFlow[0]);
	}

	unsigned int imsz=m_ImOpFlow[0]->Width()*m_ImOpFlow[0]->Height();
	float *pSpeedOut=(float*)speed.Data();
	float *pAngleOut=(float*)angle.Data();
	const float *speedf=(const float*) m_ImOpFlow[0]->Data();
	const float *maskf=(const float*) m_ImMask->Data();
	float tempf;


	float scale=speedGreyRange/speedRange;
	float A=-(scale*minSpeed);

	// fit speed vals within image intensity range
	for(int i=0;i<imsz;++i)
	{
		if(maskf[i]>m_fMaskThresh){
			tempf=A+(scale* speedf[i]);
			
			if(tempf>speedGreyRange) 
				tempf=speedGreyRange; 
			else if(tempf<0) tempf=0;
		}
		else
		{
			tempf=0;
			//pAngleOut[i]=0;
		}
		
		pSpeedOut[i]=tempf;
	}
	return true;
}


AbstractImage *McGM2005::GetTFilterResult(int n) const
{
	if (n<0){n=0;}
	if (n>McGMParams.m_nTOrders){n=McGMParams.m_nTOrders;}
	//ASSERT(m_ImTResponse[n]!=NULL);
	return (m_ImTResponse[n]);
}

AbstractImage *McGM2005::GetXFilterResult(int xorder, int yorder,int torder,int angle) const 
{
	//ASSERT (m_Basis[angle][torder]->GetResponse(xorder,yorder)!=NULL);
	return m_Basis[angle][torder]->GetResponse(xorder,yorder);
}


//#define ____________________MCGM2005_WRITE_ALL_STEPS_

//////////////////////////////////////////////////////////////////////
// Process(void)
//
// Main function to Process Sequence and Obtain single OF result
//////////////////////////////////////////////////////////////////////
bool McGM2005::Process()
{
	//TRACE("McGM::Process...\n");

	//CStopWatch sw,swsteer;
	float tempsteertime=0.0f;
	//sw.Start();
	int t=0;
	m_fTimer[MCGM_TIMER_STEER]=0.0f;
	
	//Do Temporal Filtering

	if(!TFilter())
		return false;

#ifdef ____________________MCGM2005_WRITE_ALL_STEPS_

	char fnm[30];
	CIplImg temp;

	fprintf(pErrorFile,"Printing T-Filter\n");
	for(int tf=0;tf<3;++tf)
	{
		sprintf(fnm,"TFilter/tresp[%d].bmp",tf);
		fprintf(pErrorFile,".");
		temp.ScaleF(*m_ImTResponse[tf],0,255);
		temp.WriteBMP(fnm);
	}
	fprintf(pErrorFile,"\n");
#endif

		//Make a Basis Set (spatial filtering)
	if(!XFilter())	
		return false;
	
	//Sleep(0);//Xfer to Other Threads...

	//Zero Accumulators from Last Run
	ZeroAccumulators();

	//Loop through all orientations
	for (int angle=0;angle<McGMParams.m_nAngles/2;angle++){
		
		//Steer Basis set to generate oriented set
		//swsteer.Start();
		if (angle>0)
		{
			for (t=0;t<=McGMParams.m_nTOrders;t++)
			{
				if(!m_Basis[0][t]->SteerBasis(rad_angles[angle],m_Basis[angle][t]))
					return false;

#ifdef ____________________MCGM2005_WRITE_ALL_STEPS_
				
					int order=m_Basis[angle][t]->GetOrder();
					fprintf(pErrorFile,"Printing basis[%d][%d]",angle,t);
					CIplImg temp;
					for (int yo=0;yo<=order;yo++)
					{
						for (int xo=0;xo<=(order-yo);xo++)
						{
							char fnm[50];
							fprintf(pErrorFile,".");
							sprintf(fnm,"Basis/Basis[%d][%d]Response[%d][%d].bmp",angle,t,xo,yo);
							temp.ScaleF(*m_Basis[angle][t]->GetResponse(xo,yo),0,255);
							temp.WriteBMP(fnm);
						}
					}
					fprintf(pErrorFile,"\n");
#endif
				//status=m_Basis[0][t]->SteerBasis((3.14159*165.0)/180.0);
			}
		//	Sleep(0);//Xfer to Other Threads...
		}
		//tempsteertime+=swsteer.Read();
		
		if(!Compute_Taylor_Products(angle))
			return false;


#ifdef ____________________MCGM2005_WRITE_ALL_STEPS_

		fprintf(pErrorFile,"Printing Taylor Prods");
		for(int i=0;i<6;i++)
		{
			fprintf(pErrorFile,".");
			sprintf(fnm,"TaylorProd/TaylorProd[%d][%d].bmp",angle,i);	
			temp.ScaleF(*m_ImTaylorProd[angle][i],0,255);
			temp.WriteBMP(fnm);
		}
		fprintf(pErrorFile,"\n");

#endif


		if(!Compute_Taylor_Quotients(angle))
			return false;

#ifdef ____________________MCGM2005_WRITE_ALL_STEPS_
		fprintf(pErrorFile,"Printing Taylor Quot");
		for (i=0;i<6;i++)
		{
			char fnm[50];
			fprintf(pErrorFile,".");
			sprintf(fnm,"TaylorProd_Quot/TaylorQuot[%d][%d].bmp",angle,i);
			temp.ScaleF(*m_ImTaylorQuot[angle][i],0,255);
			temp.WriteBMP(fnm);
		}
		fprintf(pErrorFile,"\n");
#endif

		if(!Compute_Speed_Matrices(angle))
			return false;


#ifdef ____________________MCGM2005_WRITE_ALL_STEPS_

		fprintf(pErrorFile,"Printing Inner speed");
		for(i=0;i<2;++i)
		{
			for(int j=0;j<2;++j)
			{
				fprintf(pErrorFile,".");
				sprintf(fnm,"Speed/InnerSpeed[%d][%d].bmp",i,j);
				temp.ScaleF(*m_ImMSpeed[i][j],0,255);
				temp.WriteBMP(fnm);
			}
		}
		fprintf(pErrorFile,"\n");
		
#endif

		// aja new1
		if(!New_3D_Vectors(angle))
			return false;

		if(!Compute_Outer_Taylor_Quotients(angle))
			return false;

		if(!Compute_Outer_Speed_Matrices(angle))
			return false;

#ifdef ____________________MCGM2005_WRITE_ALL_STEPS_
		fprintf(pErrorFile,"Printing Outer speed");
		for(i=2;i<4;++i){
			for(int j=0;j<2;++j){
				fprintf(pErrorFile,".");
				sprintf(fnm,"Speed/OuterSpeed[%d][%d].bmp",i,j);
				temp.ScaleF(*m_ImMSpeed[i][j],0,255);
				temp.WriteBMP(fnm);
			}
		}
		fprintf(pErrorFile,"\n");
#endif

	}

	m_fTimer[MCGM_TIMER_STEER]=tempsteertime;

	if(!Compute_Velocity())
		return false;

	//if(!Compute_Mask())
	//	return false;

	//m_fTimer[MCGM_TIMER_PROCESS]=sw.Read();

	//TRACE("McGM::Process...OK (%3.3f ms)\n",sw.Read());	
	return true;
}

//////////////////////////////////////////////////////////////////////
// ProcessOrthogonal(void)
//
// Main function to Process Sequence and Obtain single OF result
// but using only single X-Y basis set.  No orientation columns.
//////////////////////////////////////////////////////////////////////
bool McGM2005::ProcessOrthogonal()
{
	//TRACE("McGM::Process...\n");

	bool status;
	//CStopWatch sw,swsteer;
	float tempsteertime=0.0f;
	//sw.Start();
	int t=0;
	m_fTimer[MCGM_TIMER_STEER]=0.0f;
	
	//Do Temporal Filtering
	status=TFilter();
	if (status==false){return false;}
	
	//Make a Basis Set (spatial filtering)
	status=XFilter();		
	if (status==false){return false;}
//	Sleep(0);//Xfer to Other Threads...

	//Zero Accumulators from Last Run
	ZeroAccumulators();
	
	//Steer Basis by 90 degrees.  (Note this step could be avoided)
	for (t=0;t<=McGMParams.m_nTOrders;t++){
		status=m_Basis[0][t]->SteerBasis((float)(3.1415926/2.0),m_Basis[1][t]);
		if (status==false){return false;}
	}

	status=Compute_Taylor_Products(0);
	if (status==false){return false;}
	
	status=Compute_Taylor_Quotients(0);
	if (status==false){return false;}
	
	status=Compute_Speed_Matrices(0);
	if (status==false){return false;}
	
	//Sleep(0);//Xfer to Other Threads...
		
	status=Compute_Velocity();
	if (status==false){return false;}

	status=Compute_Mask();
	if (status==false){return false;}

//	m_fTimer[MCGM_TIMER_PROCESS]=sw.Read();

	//TRACE("McGM::Process...OK (%3.3f ms)\n",sw.Read());	
	return true;
}

//////////////////////////////////////////////////////////////////////
// Temporal Filtering
//////////////////////////////////////////////////////////////////////
bool McGM2005::TFilter(){
	//TRACE("McGM::TFilter...\n");
	
	if (m_FrmBuf.size()==0){fprintf(pErrorFile,"McGM2005::TFilter(), No Sequence to Filter!\n");return false;}
	
	bool bstatus;

	//CStopWatch sw;
	//sw.Start();

		//Get a list of pointers to IPLImages for Convolution
	AbstractImage **seq= new AbstractImage*[McGMParams.m_nTSupportSize];
	bstatus=GetFrameList(m_FrmBuf,McGMParams.m_nTSupportSize,seq);
	if (!bstatus) {
		fprintf(pErrorFile,"McGM2005::TFilter(), Failed to Get FrameList requested.\n");
		fprintf(stdout,"McGM2005::TFilter(), Temporal Filter Failed.  (Ran out of Frames?)\n");
		return false;
	}
	
		//Cycle through Temporal Derivatives
	for (int tf=0;tf<=McGMParams.m_nTOrders;tf++){
			//Allocate temporal derivative buffers if not already allocated
		if (!m_ImTResponse[tf]){
			//m_ImTResponse[tf]=m_FrmBuf.GetCurrentFrame()->BlankCopy();
			m_ImTResponse[tf]=(*m_FrmBuf.begin())->NewBlankCopy();
		}
		//Convolve
		
		//m_TFilters[tf]->TConvolve(seq,m_ImTResponse[tf]->GetpImg());
		m_ImTResponse[tf]->TConvolve((const AbstractImage**)seq,
									McGMParams.m_nTSupportSize,*m_TFilters[tf]);
	}

	delete[] seq;
	//m_fTimer[MCGM_TIMER_TFILT]=sw.Read();
	//TRACE("McGM::TFilter...(%3.3f ms) OK \n",m_fTimer[MCGM_TIMER_TFILT]);	
	return true;


}

//////////////////////////////////////////////////////////////////////
// Spatial Filtering
//////////////////////////////////////////////////////////////////////
bool McGM2005::XFilter()
{
	//TRACE("McGM::XFilter...\n");

	//CStopWatch sw;
	//sw.Start();

	for (int t=0;t<=McGMParams.m_nTOrders;t++){
		if (m_ImTResponse[t]!=NULL){
			if (!McGMParams.m_bFastSpatialFiltering){
			m_Basis[0][t]->MakeBasis(*m_ImTResponse[t]);
			}
			else{
			
			m_Basis[0][t]->MakeBasisFast(*m_ImTResponse[t]);
			}
		}
	}
	
	//m_fTimer[MCGM_TIMER_XFILT]=sw.Read();
	//TRACE("McGM::XFilter...(%3.3f ms) OK \n",sw.Read());	
	return true;
}

//////////////////////////////////////////////////////////////////////
// Compute Mask
// Masks out results if Temporal filters output is low.  Many images with
// noise become too cluttered if this is not done...
//////////////////////////////////////////////////////////////////////
bool McGM2005::Compute_Mask()
{
	//TRACE("ComputeMask...");

	//If we dont want to compute a mask just use a blank one
	if (!m_bDoMask){
		if (!m_ImMask){
		m_ImMask=m_ImTResponse[1]->NewCopy();
		m_ImMask->Zero();
		}
		return true;
	}

	//CStopWatch sw;
	//sw.Start();

	//Allocate Mask if required
	if (!m_ImMask){
		m_ImMask=m_ImTResponse[1]->NewCopy();
		if (!m_ImMask){
			fprintf(pErrorFile,"McGM2005::Compute_Mask(), Unable to create mask!\n");
			return false;
		}
	}
	else{
		m_ImMask->Copy(*m_ImTResponse[1]);
	}
	
	//Take absolute val of First Derivative....
	m_ImMask->Abs();

	//Add second derivative if exists
	if (McGMParams.m_nTOrders>1){
		AbstractImage *temp=m_ImTResponse[2]->NewCopy();
		temp->Abs();
		*m_ImMask+=*temp;
		delete temp;
	}

	//TRACE("McGM::Compute_Mask()...(%3.3f ms) OK \n",m_fTimer[MCGM_TIMER_CMASK]);	
	return true;
}


//////////////////////////////////////////////////////////////////////
// Taylor Derivative Product Sums
// Note - I have replaced the old 2 stage multiply accumulate
// with the new Mac1 function - its 20% faster.
//////////////////////////////////////////////////////////////////////
bool McGM2005::Compute_Taylor_Products(int th)
{
	//TRACE("McGM::Taylor_Products()...\n");
	//CStopWatch sw;
	//sw.Start();
	
	int i, x, y, t;
	
	//CString dbg;

	//Allocate Images if required (first time)
	for (i=0;i<6;i++){
		if (m_ImTaylorProd[th][i]==NULL){
			if (m_Basis[th][0]->GetResponse(0,0)!=NULL){
//				m_ImTaylorProd[th][i]=m_Basis[th][0]->GetResponse(0,0)->BlankCopy();
				m_ImTaylorProd[th][i]=m_Basis[th][0]->GetResponse(0,0)->NewBlankCopy();
			}
			else{
				fprintf(pErrorFile,"McGM2005::Compute_Taylor_Products(), Cannot create Taylor Products because no Basis exists\n");
				return false;
			}
		}
	}
	//Allocate Temp Buffer (if required)
	if (m_ImTempFP==NULL){
		m_ImTempFP=m_Basis[th][0]->GetResponse(0,0)->NewBlankCopy();
	}

	//Clear Taylor Accumulator
	for (i=0;i<6;i++){
		m_ImTaylorProd[th][i]->Zero();
	}

	float w;
		for (t = 0; t < McGMParams.m_nTOrders; t++) {
			for (y = 0; y < McGMParams.m_nYOrders;  y++) {
				for (x = 0; x < McGMParams.m_nXOrders; x++) {
					//TRACE("Taylor Term %dx,%dy,%dt\n",x,y,t);
					w=m_fWeight_Matrix[t][y][x];
					//X.X
					//iplSquare(m_Basis[th][t]->GetResponse(x+1,y)->GetpImg(),m_ImTempFP->GetpImg());
					m_ImTempFP->Sq(*m_Basis[th][t]->GetResponse(x+1,y));
					m_ImTaylorProd[th][MCGM_XX]->MultAcc(*m_ImTempFP,w);
					//X.Y
					//iplMultiply(m_Basis[th][t]->GetResponse(x+1,y)->GetpImg(),m_Basis[th][t]->GetResponse(x,y+1)->GetpImg(),m_ImTempFP->GetpImg());
					m_ImTempFP->Multiply(*m_Basis[th][t]->GetResponse(x+1,y),*m_Basis[th][t]->GetResponse(x,y+1));
					m_ImTaylorProd[th][MCGM_XY]->MultAcc(*m_ImTempFP,w);
					//X.T
					//iplMultiply(m_Basis[th][t]->GetResponse(x+1,y)->GetpImg(),m_Basis[th][t+1]->GetResponse(x,y)->GetpImg(),m_ImTempFP->GetpImg());
					m_ImTempFP->Multiply(*m_Basis[th][t]->GetResponse(x+1,y),*m_Basis[th][t+1]->GetResponse(x,y));
					m_ImTaylorProd[th][MCGM_XT]->MultAcc(*m_ImTempFP,w);
					//Y.Y
					//iplSquare(m_Basis[th][t]->GetResponse(x,y+1)->GetpImg(),m_ImTempFP->GetpImg());
					m_ImTempFP->Sq(*m_Basis[th][t]->GetResponse(x,y+1));
					m_ImTaylorProd[th][MCGM_YY]->MultAcc(*m_ImTempFP,w);
					//Y.T
					//iplMultiply(m_Basis[th][t]->GetResponse(x,y+1)->GetpImg(),m_Basis[th][t+1]->GetResponse(x,y)->GetpImg(),m_ImTempFP->GetpImg());
					m_ImTempFP->Multiply(*m_Basis[th][t]->GetResponse(x,y+1),*m_Basis[th][t+1]->GetResponse(x,y));
					m_ImTaylorProd[th][MCGM_YT]->MultAcc(*m_ImTempFP,w);
					//T.T
					//iplSquare(m_Basis[th][t+1]->GetResponse(x,y)->GetpImg(),m_ImTempFP->GetpImg());
					m_ImTempFP->Sq(*m_Basis[th][t+1]->GetResponse(x,y));
					m_ImTaylorProd[th][MCGM_TT]->MultAcc(*m_ImTempFP,w);
					}
			}
		}

		//m_fTimer[MCGM_TIMER_TPROD]=sw.Read();

	//TRACE("McGM::Taylor_Products()...(%3.3f ms) OK \n",m_fTimer[MCGM_TIMER_TPROD]);	
	return true;
}//End of Function

bool McGM2005::New_3D_Vectors(int th)
{
	int i;
	//Allocate Images if required (first time)
	for (i=0;i<6;i++){
		if (!m_ImTaylorOuterProd[th][i]){
			if (m_Basis[th][0]->GetResponse(0,0)){
				m_ImTaylorOuterProd[th][i]=m_Basis[th][0]->GetResponse(0,0)->NewBlankCopy();
			}
			else{
				fprintf(pErrorFile,"McGM2005::New_3D_Vectors(), Cannot create Taylor Products because no Basis exists\n");
				return false;
			}
		}

		if (!m_ImTempTaylorProd[i]){
			if (m_Basis[th][0]->GetResponse(0,0)){
				m_ImTempTaylorProd[i]=m_Basis[th][0]->GetResponse(0,0)->NewBlankCopy();
			}
			else{
				fprintf(pErrorFile,"McGM2005::New_3D_Vectors(), Cannot create Taylor Products because no Basis exists\n");
				return false;
			}
		}
	}

	

	for (i=0;i<3;i++){
		for (int j=0;j<3;j++){
			if (!m_ImTempOuterProd[i][j]){
				if (m_Basis[th][0]->GetResponse(0,0)){
					m_ImTempOuterProd[i][j]=m_Basis[th][0]->GetResponse(0,0)->NewBlankCopy();
				}
				else{
					fprintf(pErrorFile,"McGM2005::New_3D_Vectors(), Cannot create Taylor Products because no Basis exists\n");
					return false;
				}
			}
		}
	}


	if(!m_ImTempNew){
		if (m_Basis[th][0]->GetResponse(0,0)){
				m_ImTempNew=m_Basis[th][0]->GetResponse(0,0)->NewBlankCopy();
		}
		else{
			fprintf(pErrorFile,"McGM2005::New_3D_Vectors(), Cannot create Taylor Products because no Basis exists\n");
			return false;
		}
	}
	if(!m_ImTempNew2){
		if (m_Basis[th][0]->GetResponse(0,0)){
				m_ImTempNew2=m_Basis[th][0]->GetResponse(0,0)->NewBlankCopy();
		}
		else{
			fprintf(pErrorFile,"McGM2005::New_3D_Vectors(), Cannot create Taylor Products because no Basis exists\n");
			return false;
		}
	}

	if(!m_ImDimensionIndex){
		if (m_Basis[th][0]->GetResponse(0,0)){
				m_ImDimensionIndex=m_Basis[th][0]->GetResponse(0,0)->NewBlankCopy();
		}
		else{
			fprintf(pErrorFile,"McGM2005::New_3D_Vectors(), Cannot create Taylor Products because no Basis exists\n");
			return false;
		}
	}

	
	//square

	InnerProd(*m_ImTaylorProd[th][MCGM_XX],*m_ImTaylorProd[th][MCGM_XY],*m_ImTaylorProd[th][MCGM_XT],*m_ImTaylorProd[th][MCGM_XX],*m_ImTaylorProd[th][MCGM_XY],*m_ImTaylorProd[th][MCGM_XT],*m_ImTempTaylorProd[NXX]);
	InnerProd(*m_ImTaylorProd[th][MCGM_XY],*m_ImTaylorProd[th][MCGM_YY],*m_ImTaylorProd[th][MCGM_YT],*m_ImTaylorProd[th][MCGM_XY],*m_ImTaylorProd[th][MCGM_YY],*m_ImTaylorProd[th][MCGM_YT],*m_ImTempTaylorProd[NYY]);
	InnerProd(*m_ImTaylorProd[th][MCGM_XT],*m_ImTaylorProd[th][MCGM_YT],*m_ImTaylorProd[th][MCGM_TT],*m_ImTaylorProd[th][MCGM_XT],*m_ImTaylorProd[th][MCGM_YT],*m_ImTaylorProd[th][MCGM_TT],*m_ImTempTaylorProd[NTT]);

	InnerProd(*m_ImTaylorProd[th][MCGM_XX],*m_ImTaylorProd[th][MCGM_XY],*m_ImTaylorProd[th][MCGM_XT],*m_ImTaylorProd[th][MCGM_XT],*m_ImTaylorProd[th][MCGM_YT],*m_ImTaylorProd[th][MCGM_TT],*m_ImTempTaylorProd[NXT]);
	InnerProd(*m_ImTaylorProd[th][MCGM_XY],*m_ImTaylorProd[th][MCGM_YY],*m_ImTaylorProd[th][MCGM_YT],*m_ImTaylorProd[th][MCGM_XT],*m_ImTaylorProd[th][MCGM_YT],*m_ImTaylorProd[th][MCGM_TT],*m_ImTempTaylorProd[NYT]);
	InnerProd(*m_ImTaylorProd[th][MCGM_XX],*m_ImTaylorProd[th][MCGM_XY],*m_ImTaylorProd[th][MCGM_XT],*m_ImTaylorProd[th][MCGM_XY],*m_ImTaylorProd[th][MCGM_YY],*m_ImTaylorProd[th][MCGM_YT],*m_ImTempTaylorProd[NXY]);

	//outer products
	
	OuterProdThresh(*m_ImTaylorProd[th][MCGM_XX],*m_ImTaylorProd[th][MCGM_XY],*m_ImTaylorProd[th][MCGM_XT],*m_ImTaylorProd[th][MCGM_XT],*m_ImTaylorProd[th][MCGM_YT],*m_ImTaylorProd[th][MCGM_TT],*m_ImTempOuterProd[XT][0],*m_ImTempOuterProd[XT][1],*m_ImTempOuterProd[XT][2],0.001f);
	OuterProdThresh(*m_ImTaylorProd[th][MCGM_XY],*m_ImTaylorProd[th][MCGM_YY],*m_ImTaylorProd[th][MCGM_YT],*m_ImTaylorProd[th][MCGM_XT],*m_ImTaylorProd[th][MCGM_YT],*m_ImTaylorProd[th][MCGM_TT],*m_ImTempOuterProd[YT][0],*m_ImTempOuterProd[YT][1],*m_ImTempOuterProd[YT][2],0.001f);
	OuterProdThresh(*m_ImTaylorProd[th][MCGM_XX],*m_ImTaylorProd[th][MCGM_XY],*m_ImTaylorProd[th][MCGM_XT],*m_ImTaylorProd[th][MCGM_XY],*m_ImTaylorProd[th][MCGM_YY],*m_ImTaylorProd[th][MCGM_YT],*m_ImTempOuterProd[XY][0],*m_ImTempOuterProd[XY][1],*m_ImTempOuterProd[XY][2],0.001f);
	
	
	//OuterProd(*m_ImTaylorProd[th][MCGM_XX],*m_ImTaylorProd[th][MCGM_XY],*m_ImTaylorProd[th][MCGM_XT],*m_ImTaylorProd[th][MCGM_XT],*m_ImTaylorProd[th][MCGM_YT],*m_ImTaylorProd[th][MCGM_TT],*m_ImTempOuterProd[XT][0],*m_ImTempOuterProd[XT][1],*m_ImTempOuterProd[XT][2]);
	//OuterProd(*m_ImTaylorProd[th][MCGM_XY],*m_ImTaylorProd[th][MCGM_YY],*m_ImTaylorProd[th][MCGM_YT],*m_ImTaylorProd[th][MCGM_XT],*m_ImTaylorProd[th][MCGM_YT],*m_ImTaylorProd[th][MCGM_TT],*m_ImTempOuterProd[YT][0],*m_ImTempOuterProd[YT][1],*m_ImTempOuterProd[YT][2]);
	//OuterProd(*m_ImTaylorProd[th][MCGM_XX],*m_ImTaylorProd[th][MCGM_XY],*m_ImTaylorProd[th][MCGM_XT],*m_ImTaylorProd[th][MCGM_XY],*m_ImTaylorProd[th][MCGM_YY],*m_ImTaylorProd[th][MCGM_YT],*m_ImTempOuterProd[XY][0],*m_ImTempOuterProd[XY][1],*m_ImTempOuterProd[XY][2]);
	
	  // inner products
	
	InnerProd(*m_ImTempOuterProd[XY][0],*m_ImTempOuterProd[XY][1],*m_ImTempOuterProd[XY][2],*m_ImTempOuterProd[XY][0],*m_ImTempOuterProd[XY][1],*m_ImTempOuterProd[XY][2],*m_ImTaylorOuterProd[th][XY_XY]);
	InnerProd(*m_ImTempOuterProd[XT][0],*m_ImTempOuterProd[XT][1],*m_ImTempOuterProd[XT][2],*m_ImTempOuterProd[XT][0],*m_ImTempOuterProd[XT][1],*m_ImTempOuterProd[XT][2],*m_ImTaylorOuterProd[th][XT_XT]);
	InnerProd(*m_ImTempOuterProd[YT][0],*m_ImTempOuterProd[YT][1],*m_ImTempOuterProd[YT][2],*m_ImTempOuterProd[YT][0],*m_ImTempOuterProd[YT][1],*m_ImTempOuterProd[YT][2],*m_ImTaylorOuterProd[th][YT_YT]);

	InnerProd(*m_ImTempOuterProd[YT][0],*m_ImTempOuterProd[YT][1],*m_ImTempOuterProd[YT][2],*m_ImTempOuterProd[XY][0],*m_ImTempOuterProd[XY][1],*m_ImTempOuterProd[XY][2],*m_ImTaylorOuterProd[th][YT_XY]);
	InnerProd(*m_ImTempOuterProd[XT][0],*m_ImTempOuterProd[XT][1],*m_ImTempOuterProd[XT][2],*m_ImTempOuterProd[XY][0],*m_ImTempOuterProd[XY][1],*m_ImTempOuterProd[XY][2],*m_ImTaylorOuterProd[th][XT_XY]);
	InnerProd(*m_ImTempOuterProd[XT][0],*m_ImTempOuterProd[XT][1],*m_ImTempOuterProd[XT][2],*m_ImTempOuterProd[YT][0],*m_ImTempOuterProd[YT][1],*m_ImTempOuterProd[YT][2],*m_ImTaylorOuterProd[th][XT_YT]);

	//iplSquare(m_ImTempTaylorProd[NXY]->GetpImg(),m_ImTempNew->GetpImg());
	m_ImTempNew->Sq(*m_ImTempTaylorProd[NXY]);
	//m_ImTaylorOuterProd[th][XY_XY]->TestDivideF(*m_ImTempNew,
				//McGMParams.m_fQuotThreshNum,McGMParams.m_fQuotThreshDen,*m_ImTempNew2);
	m_ImTempNew2->TestDivideF(*m_ImTaylorOuterProd[th][XY_XY],*m_ImTempNew,
						McGMParams.m_fQuotThreshNum,McGMParams.m_fQuotThreshDen);
    
		//this necessary?
	m_ImTempNew2->Sqrt();
	*m_ImDimensionIndex+=*m_ImTempNew2;

	for( i=0; i<6; ++i )
	{
		m_ImTaylorProd[th][i]->Copy(*m_ImTempTaylorProd[i]);
	}
	
	return true;
}




//////////////////////////////////////////////////////////////////////
// Taylor Outer Quotients
// Note: I have sped quotients up by using TestDivide - but left the old code
// commented out. New code is 10% faster.  
//////////////////////////////////////////////////////////////////////
bool McGM2005::Compute_Outer_Taylor_Quotients(int th)
{
	//TRACE("McGM::Taylor_Quotients(%d)...\n",th);
	//CStopWatch sw;
	//sw.Start();

	float nNorm= (float)(sqrt(2.0 / McGMParams.m_nAngles));
	float mnNorm= (float)(sqrt(2.0 / McGMParams.m_nAngles)*(-1.0));
	int i;
	

	//Allocate Images if required (first time)
	for (i=0;i<6;i++){
		if (!m_ImTaylorQuot[th][i]){
			if (m_ImTaylorProd[th][0]){
				m_ImTaylorQuot[th][i]=m_ImTaylorProd[th][0]->NewBlankCopy();
			}
			else{
				fprintf(pErrorFile,
"McGM2005::Compute_Outer_Taylor_Quotients(), Cannot create Taylor Quotients because no Taylor Products exist\n");
				return false;
			}
		}
	}


	// speed YTXY / XYXY
	//m_ImTaylorOuterProd[th][YT_XY]->TestDivideF(*m_ImTaylorOuterProd[th][XY_XY],
	//	McGMParams.m_fQuotThreshNum,McGMParams.m_fQuotThreshDen,*m_ImTaylorQuot[th][MCGM_XT_XX]);
	m_ImTaylorQuot[th][MCGM_XT_XX]->TestDivideF(*m_ImTaylorOuterProd[th][YT_XY],*m_ImTaylorOuterProd[th][XY_XY],
													McGMParams.m_fQuotThreshNum,McGMParams.m_fQuotThreshDen);
	// inv speed YTXY / YTYT
	//m_ImTaylorOuterProd[th][YT_XY]->TestDivideF(*m_ImTaylorOuterProd[th][YT_YT],
	//				McGMParams.m_fQuotThreshNum,McGMParams.m_fQuotThreshDen,*m_ImTaylorQuot[th][MCGM_XT_TT]);
	m_ImTaylorQuot[th][MCGM_XT_TT]->TestDivideF(*m_ImTaylorOuterProd[th][YT_XY],*m_ImTaylorOuterProd[th][YT_YT],
													McGMParams.m_fQuotThreshNum,McGMParams.m_fQuotThreshDen);
	// orthogonal speed XTXY /XYXY
	//m_ImTaylorOuterProd[th][XT_XY]->TestDivideF(*m_ImTaylorOuterProd[th][XY_XY],
	//					McGMParams.m_fQuotThreshNum,McGMParams.m_fQuotThreshDen,*m_ImTaylorQuot[th][MCGM_YT_YY]);
	m_ImTaylorQuot[th][MCGM_YT_YY]->TestDivideF(*m_ImTaylorOuterProd[th][XT_XY],*m_ImTaylorOuterProd[th][XY_XY],
													McGMParams.m_fQuotThreshNum,McGMParams.m_fQuotThreshDen);
	
	// orthogonal inv speed XTXY / XTXT
	//m_ImTaylorOuterProd[th][XT_XY]->TestDivideF(*m_ImTaylorOuterProd[th][XT_XT],
	//					McGMParams.m_fQuotThreshNum,McGMParams.m_fQuotThreshDen,*m_ImTaylorQuot[th][MCGM_YT_TT]);
	m_ImTaylorQuot[th][MCGM_YT_TT]->TestDivideF(*m_ImTaylorOuterProd[th][XT_XY],*m_ImTaylorOuterProd[th][XT_XT],
													McGMParams.m_fQuotThreshNum,McGMParams.m_fQuotThreshDen);
	// Y Structure XTYT / YTYT
	//m_ImTaylorOuterProd[th][XT_YT]->TestDivideF(*m_ImTaylorOuterProd[th][YT_YT],
	//					McGMParams.m_fQuotThreshNum,McGMParams.m_fQuotThreshDen,*m_ImTaylorQuot[th][MCGM_XY_XX]);
	m_ImTaylorQuot[th][MCGM_XY_XX]->TestDivideF(*m_ImTaylorOuterProd[th][XT_YT],*m_ImTaylorOuterProd[th][YT_YT],
													McGMParams.m_fQuotThreshNum,McGMParams.m_fQuotThreshDen);
	// X structure XTYT / XTXT
	//m_ImTaylorOuterProd[th][XT_YT]->TestDivideF(*m_ImTaylorOuterProd[th][XT_XT],
	//			McGMParams.m_fQuotThreshNum,McGMParams.m_fQuotThreshDen,*m_ImTaylorQuot[th][MCGM_XY_YY]);
	m_ImTaylorQuot[th][MCGM_XY_YY]->TestDivideF(*m_ImTaylorOuterProd[th][XT_YT],*m_ImTaylorOuterProd[th][XT_XT],
													McGMParams.m_fQuotThreshNum,McGMParams.m_fQuotThreshDen);

	//Adjustments and Normalisation/ AJ This is a bit of a hack to make sure the directions fitted in with the old model

	m_ImTaylorQuot[th][MCGM_XT_TT]->MultiplyS(&mnNorm);	//Inv Speed
	m_ImTaylorQuot[th][MCGM_XT_XX]->MultiplyS(&mnNorm);	//Speed	 
	
	m_ImTaylorQuot[th][MCGM_YT_TT]->MultiplyS(&nNorm);	//Inv Orth Speed
	m_ImTaylorQuot[th][MCGM_YT_YY]->MultiplyS(&nNorm);	//Orth Speed 

	const float one=1;

	m_ImTaylorQuot[th][MCGM_XY_XX]->Sq();	//X Structure
	m_ImTaylorQuot[th][MCGM_XY_YY]->Sq();	//Y Structure
	(m_ImTaylorQuot[th][MCGM_XY_XX])->AddS(&one);//1.0f;
	(m_ImTaylorQuot[th][MCGM_XY_YY])->AddS(&one);//1.0f;
	
	
	m_ImTaylorQuot[th][MCGM_XT_TT]->Divide(*m_ImTaylorQuot[th][MCGM_XY_XX],McGMParams.m_fQuotThreshNum,McGMParams.m_fQuotThreshDen); 
	m_ImTaylorQuot[th][MCGM_YT_TT]->Divide(*m_ImTaylorQuot[th][MCGM_XY_YY],McGMParams.m_fQuotThreshNum,McGMParams.m_fQuotThreshDen); 
	
	//Blur the quotients before putting into speed matrices
	if(m_bBlurQuotients){
		if (!m_ImTaylorQuot[th][MCGM_XT_XX]->BlurF(5)){fprintf(pErrorFile,"Error:Unable to Blur Quotient\n");}	
		if (!m_ImTaylorQuot[th][MCGM_XT_TT]->BlurF(5)){fprintf(pErrorFile,"Error:Unable to Blur Quotient\n");}	
		if (!m_ImTaylorQuot[th][MCGM_YT_YY]->BlurF(5)){fprintf(pErrorFile,"Error:Unable to Blur Quotient\n");}	
		if (!m_ImTaylorQuot[th][MCGM_YT_TT]->BlurF(5)){fprintf(pErrorFile,"Error:Unable to Blur Quotient\n");}	
		if (!m_ImTaylorQuot[th][MCGM_XY_XX]->BlurF(5)){fprintf(pErrorFile,"Error:Unable to Blur Quotient\n");}	
		if (!m_ImTaylorQuot[th][MCGM_XY_YY]->BlurF(5)){fprintf(pErrorFile,"Error:Unable to Blur Quotient\n");}	
	}

	//m_fTimer[MCGM_TIMER_TQUOT]=sw.Read();
	//TRACE("McGM::Taylor_Quotients()...(%3.3f ms) OK \n",m_fTimer[MCGM_TIMER_TQUOT]);	
	return true;
}



//////////////////////////////////////////////////////////////////////
// Taylor Quotients
// Note: I have sped quotients up by using TestDivide - but left the old code
// commented out. New code is 10% faster.  
//////////////////////////////////////////////////////////////////////
bool McGM2005::Compute_Taylor_Quotients(int th)
{
	//TRACE("McGM::Taylor_Quotients(%d)...\n",th);
	//CStopWatch sw;
	//sw.Start();

	float nNorm= (float)(sqrt(2.0 / McGMParams.m_nAngles));

	int i;

	//Allocate Images if required (first time)
	for (i=0;i<6;i++){
		if (!m_ImTaylorQuot[th][i]){
			if (m_ImTaylorProd[th][0]){
				m_ImTaylorQuot[th][i]=m_ImTaylorProd[th][0]->NewBlankCopy();
			}
			else{
				fprintf(pErrorFile,"McGM2005::Compute_Taylor_Quotients(), Cannot create Taylor Quotients because no Taylor Products exists\n");
				return false;
			}
		}
	}
	
	//X.T/X.X	Speed
	//m_ImTaylorProd[th][MCGM_XT]->TestDivideF(*m_ImTaylorProd[th][MCGM_XX],
	//	McGMParams.m_fQuotThreshNum,McGMParams.m_fQuotThreshDen,*m_ImTaylorQuot[th][MCGM_XT_XX]);
	m_ImTaylorQuot[th][MCGM_XT_XX]->TestDivideF(*m_ImTaylorProd[th][MCGM_XT],*m_ImTaylorProd[th][MCGM_XX],
													McGMParams.m_fQuotThreshNum,McGMParams.m_fQuotThreshDen);
	
	//X.T/T.T	Inv Speed
	//m_ImTaylorProd[th][MCGM_XT]->TestDivideF(*m_ImTaylorProd[th][MCGM_TT],
	//	McGMParams.m_fQuotThreshNum,McGMParams.m_fQuotThreshDen,*m_ImTaylorQuot[th][MCGM_XT_TT]);
	m_ImTaylorQuot[th][MCGM_XT_TT]->TestDivideF(*m_ImTaylorProd[th][MCGM_XT],*m_ImTaylorProd[th][MCGM_TT],
													McGMParams.m_fQuotThreshNum,McGMParams.m_fQuotThreshDen);

	//Y.T/Y.Y	Orth Speed
	//m_ImTaylorProd[th][MCGM_YT]->TestDivideF(*m_ImTaylorProd[th][MCGM_YY],
	//	McGMParams.m_fQuotThreshNum,McGMParams.m_fQuotThreshDen,*m_ImTaylorQuot[th][MCGM_YT_YY]);
	m_ImTaylorQuot[th][MCGM_YT_YY]->TestDivideF(*m_ImTaylorProd[th][MCGM_YT],*m_ImTaylorProd[th][MCGM_YY],
													McGMParams.m_fQuotThreshNum,McGMParams.m_fQuotThreshDen);

	//Y.T/T.T	Orth Inv Speed
	//m_ImTaylorProd[th][MCGM_YT]->TestDivideF(*m_ImTaylorProd[th][MCGM_TT],
	//	McGMParams.m_fQuotThreshNum,McGMParams.m_fQuotThreshDen,*m_ImTaylorQuot[th][MCGM_YT_TT]);
	m_ImTaylorQuot[th][MCGM_YT_TT]->TestDivideF(*m_ImTaylorProd[th][MCGM_YT],*m_ImTaylorProd[th][MCGM_TT],
													McGMParams.m_fQuotThreshNum,McGMParams.m_fQuotThreshDen);


	//X.Y/X.X	X Structure
	//m_ImTaylorProd[th][MCGM_XY]->TestDivideF(*m_ImTaylorProd[th][MCGM_XX],
	//	McGMParams.m_fQuotThreshNum,McGMParams.m_fQuotThreshDen,*m_ImTaylorQuot[th][MCGM_XY_XX]);
	m_ImTaylorQuot[th][MCGM_XY_XX]->TestDivideF(*m_ImTaylorProd[th][MCGM_XY],*m_ImTaylorProd[th][MCGM_XX],
													McGMParams.m_fQuotThreshNum,McGMParams.m_fQuotThreshDen);

	//X.Y/Y.Y	Y Structure
	//m_ImTaylorProd[th][MCGM_XY]->TestDivideF(*m_ImTaylorProd[th][MCGM_YY],
	//	McGMParams.m_fQuotThreshNum,McGMParams.m_fQuotThreshDen,*m_ImTaylorQuot[th][MCGM_XY_YY]);
	m_ImTaylorQuot[th][MCGM_XY_YY]->TestDivideF(*m_ImTaylorProd[th][MCGM_XY],*m_ImTaylorProd[th][MCGM_YY],
													McGMParams.m_fQuotThreshNum,McGMParams.m_fQuotThreshDen);

	//Adjustments and Normalisation
	m_ImTaylorQuot[th][MCGM_XT_TT]->MultiplyS(&nNorm);	//Inv Speed
	m_ImTaylorQuot[th][MCGM_YT_TT]->MultiplyS(&nNorm);	//Inv Orth Speed
	m_ImTaylorQuot[th][MCGM_XT_XX]->MultiplyS(&nNorm);	//Speed
	m_ImTaylorQuot[th][MCGM_YT_YY]->MultiplyS(&nNorm);	//Orth Speed

	m_ImTaylorQuot[th][MCGM_XY_XX]->Sq();	//X Structure
	m_ImTaylorQuot[th][MCGM_XY_YY]->Sq();	//Y Structure

	const float one=1;
	(m_ImTaylorQuot[th][MCGM_XY_XX])->AddS(&one); //1.0f;
	(m_ImTaylorQuot[th][MCGM_XY_YY])->AddS(&one); //1.0f;

	m_ImTaylorQuot[th][MCGM_XT_XX]->Divide(*m_ImTaylorQuot[th][MCGM_XY_XX],
								McGMParams.m_fQuotThreshNum,McGMParams.m_fQuotThreshDen);
	m_ImTaylorQuot[th][MCGM_YT_YY]->Divide(*m_ImTaylorQuot[th][MCGM_XY_YY],
								McGMParams.m_fQuotThreshNum,McGMParams.m_fQuotThreshDen);
	
	//Blur the quotients before putting into speed matrices
	if(m_bBlurQuotients){
		if (!m_ImTaylorQuot[th][MCGM_XT_XX]->BlurF(5)){fprintf(pErrorFile,"Error:Unable to Blur Quotient\n");}	
		if (!m_ImTaylorQuot[th][MCGM_XT_TT]->BlurF(5)){fprintf(pErrorFile,"Error:Unable to Blur Quotient\n");}	
		if (!m_ImTaylorQuot[th][MCGM_YT_YY]->BlurF(5)){fprintf(pErrorFile,"Error:Unable to Blur Quotient\n");}	
		if (!m_ImTaylorQuot[th][MCGM_YT_TT]->BlurF(5)){fprintf(pErrorFile,"Error:Unable to Blur Quotient\n");}	
		if (!m_ImTaylorQuot[th][MCGM_XY_XX]->BlurF(5)){fprintf(pErrorFile,"Error:Unable to Blur Quotient\n");}	
		if (!m_ImTaylorQuot[th][MCGM_XY_YY]->BlurF(5)){fprintf(pErrorFile,"Error:Unable to Blur Quotient\n");}	
	}

	//m_fTimer[MCGM_TIMER_TQUOT]=sw.Read();
	//TRACE("McGM::Taylor_Quotients()...(%3.3f ms) OK \n",m_fTimer[MCGM_TIMER_TQUOT]);	
	return true;
}

//////////////////////////////////////////////////////////////////////
// Compute Speed Matrices
//////////////////////////////////////////////////////////////////////
bool McGM2005::Compute_Speed_Matrices(int th)
{
	//TRACE("McGM::Compute_Speed_Matrices...\n");
	//CStopWatch sw;
	//sw.Start();
	int i,j,k;

	//////////////
	//Allocate Matrices
	for (i=0;i<4;i++){
		for (j=0;j<2;j++){
			for (k=0;k<2;k++){
				if (!m_ImMatrices[i][j][k]){
					m_ImMatrices[i][j][k]=m_ImTaylorQuot[th][0]->NewBlankCopy();
				}
			}
		}
	}

	//Bottom Matrix
	//Speed * Inv Speed
	m_ImTempFP->Copy(*m_ImTaylorQuot[th][MCGM_XT_XX]);
	*m_ImTempFP *= *m_ImTaylorQuot[th][MCGM_XT_TT];
	*m_ImMatrices[1][0][0] +=*m_ImTempFP; 

	//aja 27/4. m_ImTaylorQuot[th][MCGM_XT_XX] has negative values

	//Speed * Inv Orth Speed
	m_ImTempFP->Copy(*m_ImTaylorQuot[th][MCGM_XT_XX]);
	*m_ImTempFP *= *m_ImTaylorQuot[th][MCGM_YT_TT];
	*m_ImMatrices[1][0][1] +=*(m_ImTempFP);

	//Orth Speed * Inv Speed
	m_ImTempFP->Copy(*m_ImTaylorQuot[th][MCGM_YT_YY]);
	*m_ImTempFP *= *(m_ImTaylorQuot[th][MCGM_XT_TT]);
	*m_ImMatrices[1][1][0] +=*(m_ImTempFP);	
	//Orth Speed * Inv Orth Speed
	m_ImTempFP->Copy(*m_ImTaylorQuot[th][MCGM_YT_YY]);
	*m_ImTempFP *= *m_ImTaylorQuot[th][MCGM_YT_TT];
	*m_ImMatrices[1][1][1] +=*m_ImTempFP;
	
	//Projected Top Matrix
	m_ImTempFP->Copy(*m_ImTaylorQuot[th][MCGM_XT_XX]);	
	m_ImTempFP->MultiplyS(&n_cn[th]);
	*m_ImMatrices[0][0][0] +=*m_ImTempFP;

	m_ImTempFP->Copy(*m_ImTaylorQuot[th][MCGM_XT_XX]);	
	m_ImTempFP->MultiplyS(&n_sn[th]);
	*m_ImMatrices[0][0][1] +=*m_ImTempFP;

	m_ImTempFP->Copy(*m_ImTaylorQuot[th][MCGM_YT_YY]);	
	m_ImTempFP->MultiplyS(&n_cn[th]);
	*m_ImMatrices[0][1][0] +=*m_ImTempFP;

	m_ImTempFP->Copy(*m_ImTaylorQuot[th][MCGM_YT_YY]);	
	m_ImTempFP->MultiplyS(&n_sn[th]);
	*m_ImMatrices[0][1][1] +=*m_ImTempFP;

	//Allocate Angle Matrices
	for (j=0;j<4;j++){
		for (k=0;k<2;k++){
			if (!m_ImMSpeed[j][k]){
				m_ImMSpeed[j][k]=m_ImTaylorQuot[th][0]->NewBlankCopy();
			}
		}
	}

	m_ImTempFP->Copy(*m_ImTaylorQuot[th][MCGM_XT_XX]);
	*m_ImTempFP+=*m_ImTaylorQuot[th][MCGM_XT_TT];
	m_ImTempFP->MultiplyS(&cn[th]);
	*m_ImMSpeed[0][0]+=*m_ImTempFP;

	m_ImTempFP->Copy(*m_ImTaylorQuot[th][MCGM_XT_XX]);
	*m_ImTempFP+=*m_ImTaylorQuot[th][MCGM_XT_TT];
	m_ImTempFP->MultiplyS(&sn[th]);
	*m_ImMSpeed[0][1]+=*m_ImTempFP;

	m_ImTempFP->Copy(*m_ImTaylorQuot[th][MCGM_YT_YY]);
	*m_ImTempFP+=*m_ImTaylorQuot[th][MCGM_YT_TT];
	m_ImTempFP->MultiplyS(&cn[th]);
	*m_ImMSpeed[1][0]+=*m_ImTempFP;

	m_ImTempFP->Copy(*m_ImTaylorQuot[th][MCGM_YT_YY]);
	*m_ImTempFP+=*m_ImTaylorQuot[th][MCGM_YT_TT];
	m_ImTempFP->MultiplyS(&sn[th]);

	*m_ImMSpeed[1][1]+=*m_ImTempFP;

	//m_fTimer[MCGM_TIMER_SPMAT]=sw.Read();
	//TRACE("McGM::Compute_Speed_Matrices()...(%3.3f ms) OK \n",m_fTimer[MCGM_TIMER_SPMAT]);
	return true;
}

//////////////////////////////////////////////////////////////////////
// Compute Outer Speed Matrices
//////////////////////////////////////////////////////////////////////
bool McGM2005::Compute_Outer_Speed_Matrices(int th)
{
	//TRACE("McGM::Compute_Outer_Speed_Matrices...\n");
	//CStopWatch sw;
	//sw.Start();
	//int i;

	//Bottom Matrix
	//Speed * Inv Speed
	m_ImTempFP->Copy(*m_ImTaylorQuot[th][MCGM_XT_XX]);
	*m_ImTempFP *= *m_ImTaylorQuot[th][MCGM_XT_TT];
	*m_ImMatrices[3][0][0] += *m_ImTempFP; 
	//Speed * Inv Orth Speed
	m_ImTempFP->Copy(*m_ImTaylorQuot[th][MCGM_XT_XX]);
	*m_ImTempFP *= *m_ImTaylorQuot[th][MCGM_YT_TT];
	*m_ImMatrices[3][0][1] += *m_ImTempFP;
	//Orth Speed * Inv Speed
	m_ImTempFP->Copy(*m_ImTaylorQuot[th][MCGM_YT_YY]);
	*(m_ImTempFP) *= *(m_ImTaylorQuot[th][MCGM_XT_TT]);
	*(m_ImMatrices[3][1][0]) += *m_ImTempFP;	
	//Orth Speed * Inv Orth Speed
	m_ImTempFP->Copy(*m_ImTaylorQuot[th][MCGM_YT_YY]);
	*m_ImTempFP *= *(m_ImTaylorQuot[th][MCGM_YT_TT]);
	*m_ImMatrices[3][1][1] += *m_ImTempFP;
	
	//Projected Top Matrix
	m_ImTempFP->Copy(*m_ImTaylorQuot[th][MCGM_XT_XX]);	
	m_ImTempFP->MultiplyS(&n_cn[th]);
	*m_ImMatrices[2][0][0] += *m_ImTempFP;

	m_ImTempFP->Copy(*m_ImTaylorQuot[th][MCGM_XT_XX]);	
	m_ImTempFP->MultiplyS(&n_sn[th]);
	*m_ImMatrices[2][0][1] += *m_ImTempFP;

	m_ImTempFP->Copy(*m_ImTaylorQuot[th][MCGM_YT_YY]);	
	m_ImTempFP->MultiplyS(&n_cn[th]);
	*m_ImMatrices[2][1][0] += *m_ImTempFP;

	m_ImTempFP->Copy(*m_ImTaylorQuot[th][MCGM_YT_YY]);	
	m_ImTempFP->MultiplyS(&n_sn[th]);
	*m_ImMatrices[2][1][1] += *m_ImTempFP;

	m_ImTempFP->Copy(*(m_ImTaylorQuot[th][MCGM_XT_XX]));
	*(m_ImTempFP)+=*(m_ImTaylorQuot[th][MCGM_XT_TT]);
	m_ImTempFP->MultiplyS(&cn[th]);
	*(m_ImMSpeed[2][0])+= *(m_ImTempFP);

	m_ImTempFP->Copy(*(m_ImTaylorQuot[th][MCGM_XT_XX]));
	*(m_ImTempFP)+=*(m_ImTaylorQuot[th][MCGM_XT_TT]);
	m_ImTempFP->MultiplyS(&sn[th]);
	*(m_ImMSpeed[2][1])+= *(m_ImTempFP);

	m_ImTempFP->Copy(*m_ImTaylorQuot[th][MCGM_YT_YY]);
	*m_ImTempFP+=*m_ImTaylorQuot[th][MCGM_YT_TT];
	m_ImTempFP->MultiplyS(&cn[th]);
	*m_ImMSpeed[3][0]+= *m_ImTempFP;

	m_ImTempFP->Copy(*m_ImTaylorQuot[th][MCGM_YT_YY]);
	*m_ImTempFP+=*m_ImTaylorQuot[th][MCGM_YT_TT];
	m_ImTempFP->MultiplyS(&sn[th]);

	*m_ImMSpeed[3][1]+= *m_ImTempFP;

	//m_fTimer[MCGM_TIMER_SPMAT]=sw.Read();
	//TRACE("McGM::Compute_Speed_Matrices()...(%3.3f ms) OK \n",m_fTimer[MCGM_TIMER_SPMAT]);
	return true;
}


//////////////////////////////////////////////////////////////////////
// Compute Velocity
//////////////////////////////////////////////////////////////////////

bool McGM2005::Compute_Velocity(void)
{
	//TRACE("McGM::Compute_Velocity...\n");
	//CStopWatch sw;
	//sw.Start();

	//First time make images
	for (int i=0;i<8;i++){
		if (!m_ImDetIms[i]){
			m_ImDetIms[i]=m_ImMatrices[0][0][0]->NewBlankCopy();
		}
	}

		// m_ImTempNew stores inverted dimension index
	//m_ImDimensionIndex->TestInvertF(McGMParams.m_fQuotThreshDen,*m_ImTempNew);
	m_ImTempNew->TestInvertF(*m_ImDimensionIndex,McGMParams.m_fQuotThreshDen);

	//Top Matrix Determinant
	//iplMultiply(m_ImMatrices[0][0][0]->pImg , m_ImMatrices[0][1][1]->pImg, m_ImDetIms[0]->pImg);	
	//iplMultiply(m_ImMatrices[0][0][1]->pImg , m_ImMatrices[0][1][0]->pImg, m_ImDetIms[1]->pImg);
	m_ImDetIms[0]->Multiply(*m_ImMatrices[0][0][0],*m_ImMatrices[0][1][1]);
	m_ImDetIms[1]->Multiply(*m_ImMatrices[0][0][1],*m_ImMatrices[0][1][0]);
	*(m_ImDetIms[0])-=*(m_ImDetIms[1]);

	//Top Matrix Determinant Outer
	//iplMultiply(m_ImMatrices[2][0][0]->pImg , m_ImMatrices[2][1][1]->pImg, m_ImDetIms[2]->pImg);	
	//iplMultiply(m_ImMatrices[2][0][1]->pImg , m_ImMatrices[2][1][0]->pImg, m_ImDetIms[3]->pImg);
	m_ImDetIms[2]->Multiply(*m_ImMatrices[2][0][0],*m_ImMatrices[2][1][1]);
	m_ImDetIms[3]->Multiply(*m_ImMatrices[2][0][1],*m_ImMatrices[2][1][0]);
	*(m_ImDetIms[2])-=*(m_ImDetIms[3]);
	
	//iplAdd(m_ImDetIms[0]->GetpImg(),m_ImDetIms[2]->GetpImg(),m_ImDetIms[0]->GetpImg());

	//iplMultiplySFP(m_ImDetIms[0]->GetpImg(),m_ImDetIms[0]->GetpImg(),1.0f); //inv dem ind
	//m_ImDetIms[0]->MultAcc(*m_ImDetIms[2],0.0f);// dem ind

	//iplMultiply(m_ImDetIms[0]->GetpImg(),m_ImTempNew->GetpImg(),m_ImDetIms[0]->GetpImg());
	//iplMultiply(m_ImDetIms[2]->GetpImg(),m_ImDimensionIndex->GetpImg(),m_ImDetIms[2]->GetpImg());
	m_ImDetIms[0]->Multiply(*m_ImTempNew);
	m_ImDetIms[2]->Multiply(*m_ImDimensionIndex);
	//iplAdd(m_ImDetIms[0]->GetpImg(),m_ImDetIms[2]->GetpImg(),m_ImDetIms[0]->GetpImg());
	m_ImDetIms[0]->Add(*m_ImDetIms[2]);
	
	//Bottom Matrix Determinant
	//iplMultiply(m_ImMatrices[1][0][0]->pImg , m_ImMatrices[1][1][1]->pImg, m_ImDetIms[4]->pImg);	
	//iplMultiply(m_ImMatrices[1][0][1]->pImg , m_ImMatrices[1][1][0]->pImg, m_ImDetIms[5]->pImg);	
	m_ImDetIms[4]->Multiply(*m_ImMatrices[1][0][0],*m_ImMatrices[1][1][1]);
	m_ImDetIms[5]->Multiply(*m_ImMatrices[1][0][1],*m_ImMatrices[1][1][0]);
	
	*(m_ImDetIms[4])-=*(m_ImDetIms[5]);

	//Bottom Matrix Determinant Outer
	//iplMultiply(m_ImMatrices[3][0][0]->pImg , m_ImMatrices[3][1][1]->pImg, m_ImDetIms[6]->pImg);	
	//iplMultiply(m_ImMatrices[3][0][1]->pImg , m_ImMatrices[3][1][0]->pImg, m_ImDetIms[7]->pImg);	
	m_ImDetIms[6]->Multiply(*m_ImMatrices[3][0][0],*m_ImMatrices[3][1][1]);
	m_ImDetIms[7]->Multiply(*m_ImMatrices[3][0][1],*m_ImMatrices[3][1][0]);
	
	*(m_ImDetIms[6])-=*(m_ImDetIms[7]);

	//iplAdd(m_ImDetIms[4]->GetpImg(),m_ImDetIms[6]->GetpImg(),m_ImDetIms[4]->GetpImg());

	//iplMultiplySFP(m_ImDetIms[4]->GetpImg(),m_ImDetIms[4]->GetpImg(),1.0f);
	//m_ImDetIms[4]->MultAcc(*m_ImDetIms[6],0.0f);

	//iplMultiply(m_ImDetIms[4]->GetpImg(),m_ImTempNew->GetpImg(),m_ImDetIms[4]->GetpImg());
	//iplMultiply(m_ImDetIms[6]->GetpImg(),m_ImDimensionIndex->GetpImg(),m_ImDetIms[6]->GetpImg());
	m_ImDetIms[4]->Multiply(*m_ImTempNew);
	m_ImDetIms[6]->Multiply(*m_ImDimensionIndex);
	//iplAdd(m_ImDetIms[4]->GetpImg(),m_ImDetIms[6]->GetpImg(),m_ImDetIms[4]->GetpImg());
	m_ImDetIms[4]->Add(*m_ImDetIms[6]);

	//Division of Determinants
	*(m_ImDetIms[0]) /= *(m_ImDetIms[4]);

	//Square Root
	m_ImDetIms[0]->Sqrt();

	//Copy result into Optic Flow Image (Allocate if Required)
	if (!m_ImOpFlow[0]){
		m_ImOpFlow[0]=m_ImDetIms[0]->NewCopy();
	}
	else
	{
		m_ImOpFlow[0]->Copy(*m_ImDetIms[0]);
	}

	//Determine Angle
	*m_ImMSpeed[0][0]+=*m_ImMSpeed[1][1];
	*m_ImMSpeed[0][1]-=*m_ImMSpeed[1][0];
	
	*m_ImMSpeed[2][0]+=*m_ImMSpeed[3][1];
	*m_ImMSpeed[2][1]-=*m_ImMSpeed[3][0];

	//
	//(m_ImMSpeed[0][0])+=*(m_ImMSpeed[2][0]);
	//(m_ImMSpeed[0][1])+=*(m_ImMSpeed[2][1]);
	//

	//
	//iplMultiplySFP(m_ImMSpeed[0][0]->GetpImg(),m_ImMSpeed[0][0]->GetpImg(),1.0f);
	//iplMultiplySFP(m_ImMSpeed[0][1]->GetpImg(),m_ImMSpeed[0][1]->GetpImg(),1.0f);

	//m_ImMSpeed[0][0]->MultAcc(*m_ImMSpeed[2][0],0.0f);
	//m_ImMSpeed[0][1]->MultAcc(*m_ImMSpeed[2][1],0.0f);
	//

	//iplMultiply(m_ImMSpeed[0][0]->GetpImg(),m_ImTempNew->GetpImg(),m_ImMSpeed[0][0]->GetpImg());
	//iplMultiply(m_ImMSpeed[2][0]->GetpImg(),m_ImDimensionIndex->GetpImg(),m_ImMSpeed[2][0]->GetpImg());

	m_ImMSpeed[0][0]->Multiply(*m_ImTempNew);
	m_ImMSpeed[2][0]->Multiply(*m_ImDimensionIndex);

	//iplMultiply(m_ImMSpeed[0][1]->GetpImg(),m_ImTempNew->GetpImg(),m_ImMSpeed[0][1]->GetpImg());
	//iplMultiply(m_ImMSpeed[2][1]->GetpImg(),m_ImDimensionIndex->GetpImg(),m_ImMSpeed[2][1]->GetpImg());
	m_ImMSpeed[0][1]->Multiply(*m_ImTempNew);
	m_ImMSpeed[2][1]->Multiply(*m_ImDimensionIndex);

	//iplAdd(m_ImMSpeed[0][0]->GetpImg(),m_ImMSpeed[2][0]->GetpImg(),m_ImMSpeed[0][0]->GetpImg());
	//iplAdd(m_ImMSpeed[0][1]->GetpImg(),m_ImMSpeed[2][1]->GetpImg(),m_ImMSpeed[0][1]->GetpImg());
	m_ImMSpeed[0][0]->Add(*m_ImMSpeed[2][0]);
	m_ImMSpeed[0][1]->Add(*m_ImMSpeed[2][1]);


	if (!m_ImOpFlow[1]){
		m_ImOpFlow[1]=m_ImDetIms[0]->NewBlankCopy();
	}
	
	m_ImOpFlow[1]->ATan2(*(m_ImMSpeed[0][1]),*(m_ImMSpeed[0][0]));

	//m_fTimer[MCGM_TIMER_COMPV]=sw.Read();
	//TRACE("McGM::ComputeVelocity...(%3.3f ms) OK \n",m_fTimer[MCGM_TIMER_COMPV]);	
	return true;
}

/*

//////////////////////////////////////////////////////////////////////
//Adds a nice, clean border around the final Opticl Flow output display
bool McGM::AddBorders(int size)
{
	TRACE("AddBorders size %d.\n",size);
	if (m_ImOpFlow[0]==NULL){return false;}
	if (m_ImOpFlow[1]==NULL){return false;}
	
	double PI2=3.14159/2.0;
	int x,y,b;
	float val=1.0f;
	float maskval=100.0;
	//Clear the border around the velocity result
	for (b=0;b<size;b++){
		for (x=0;x<m_ImOpFlow[0]->Width();x++){
			m_ImOpFlow[0]->SetPixel(x,b,&val);
			m_ImOpFlow[0]->SetPixel(x,m_ImOpFlow[0]->Height()-b-1,&val);
			m_ImMask->SetPixel(x,b,&maskval);
			m_ImMask->SetPixel(x,m_ImMask->Height()-b-1,&maskval);
		}
		for (y=0;y<m_ImOpFlow[0]->Height();y++){
			m_ImOpFlow[0]->SetPixel(b,y,&val);
			m_ImOpFlow[0]->SetPixel(m_ImOpFlow[0]->Width()-b-1,y,&val);
			m_ImMask->SetPixel(b,y,&maskval);
			m_ImMask->SetPixel(m_ImMask->Width()-b-1,y,&maskval);
		}
	}
	
	//Paste the angle border around the direction result
	int w=m_ImOpFlow[1]->Width();
	int h=m_ImOpFlow[1]->Height();
	double xctr=(w-1.0)/2.0;
	double yctr=(h-1.0)/2.0;
	double xf,yf;
	for (b=0;b<size;b++){
		for (x=0;x<m_ImOpFlow[1]->Width();x++){
			xf=(double)x;yf=double(y);
			val=(float)(atan2((xf-xctr),(b-yctr))-PI2);
			m_ImOpFlow[1]->SetPixel(x,b,&val);
			val=(float)(atan2((xf-xctr),(yctr-b))-PI2);
			m_ImOpFlow[1]->SetPixel(x,m_ImOpFlow[1]->Height()-b-1,&val);
		}
		for (y=0;y<m_ImOpFlow[1]->Height();y++){
			val=(float)(atan2(b-xctr,y-yctr)-PI2);
			m_ImOpFlow[1]->SetPixel(b,y,&val);
			val=(float)(atan2(xctr-b,y-yctr)-PI2);
			m_ImOpFlow[1]->SetPixel(m_ImOpFlow[1]->Width()-b-1,y,&val);
		}
	}
	return true;
}
*/

//////////////////////////////////////////////////////////////////////
// Zero the matrices used to compute velocity.
//////////////////////////////////////////////////////////////////////
bool McGM2005::ZeroAccumulators(void)
{
	//TRACE("McGM::ZeroAccumulators...\n");
	int i,j,k;
	//Zero McGM Matrices
	for (i=0;i<4;i++){
		for (j=0;j<2;j++){
			for (k=0;k<2;k++){
				if (m_ImMatrices[i][j][k]){
					m_ImMatrices[i][j][k]->Zero();
				}	
			}
		}
	}
	
	for (i=0;i<4;i++){
		for (j=0;j<2;j++){
			if (m_ImMSpeed[i][j]){m_ImMSpeed[i][j]->Zero();}
		}
	}

	if(m_ImDimensionIndex)
		m_ImDimensionIndex->Zero();

	//TRACE("McGM::ZeroAccumulators...OK\n");
	return true;
}


//////////////////////////////////////////////////////////////////////
// Mathematics
//////////////////////////////////////////////////////////////////////
void McGM2005::Make_Trig_Tables(int Nangles) {
	int th=0;
	double isn=0.0,icn=0.0;
	double PI_2=UCL_VRL_PI/2.0;

	//Do Tables
	//Note - The rotation HAS to be the same as the filter rotation.
	for (th = 0; th < Nangles; ++th) {
		rad_angles[th]=(float)((th * UCL_VRL_PI  * 2.0f / (float)Nangles));
		sn[th] = (float)(sqrt(2.0 / Nangles) * sin(UCL_VRL_PI-rad_angles[th]));
		cn[th] = (float)(sqrt(2.0 / Nangles) * cos(UCL_VRL_PI-rad_angles[th]));
	}
	// Determine sum of squared
	isn = 1.0/(dotlength(&sn[0], &sn[0], Nangles));   /*Sum of Sin squared*/
	icn = 1.0/(dotlength(&cn[0], &cn[0], Nangles));   /*Sum of Cos squared*/
	//Normalise
	for (th = 0; th < Nangles; ++th) {
		n_sn[th] = (float)(sn[th]/isn);
		n_cn[th] = (float)(cn[th]/icn);
		//TRACE("n_sn[%3.2f]=%f  n_cn[%3.1f]=%f\n",rad_angles[th],n_sn[th],rad_angles[th],n_cn[th]);
	}
}

//////////////////////////////////////////////////////////////////////
//Make Taylor Expansion Weight Matrices
bool McGM2005::Make_Weight_Matrices(int nIntegration_Zone,bool bUnityWeights) 
{
	int ix,iy,it;
	int px,py,pt;
	double mult1,mult2,mult3;
	double cx,ct,cy;
	double weight;
	double izone=(double)nIntegration_Zone;

	mult1 = 0.5/sqrt( 0.5 *tan(UCL_VRL_PI/(double)McGMParams.m_nAngles));
	mult2 = 1.0/mult1;
	mult3 = (double) McGMParams.m_nAngles;

	//mult1=mult2=izone;

	//Zero Array
	for (pt=0; pt<=McGMParams.m_nTOrders; pt++){
		for (py=0; py<=McGMParams.m_nYOrders; py++){
			for (px=0; px<=McGMParams.m_nXOrders; px++){
					m_fWeight_Matrix[pt][py][px] =0.0;
			}
		}
	}
	//Generate Weights Squared
	for (pt=0; pt<=McGMParams.m_nTOrders; pt++){
		for (py=0; py<=McGMParams.m_nYOrders; py++){
			for (px=0; px<=McGMParams.m_nXOrders; px++){
				
				for (it=0;it< izone; it++){
					ct = (double)it - (izone - 1.0)/2.0; 
					for (iy=0;iy< izone; iy++){
						cy = (double)iy - (izone - 1.0)/2.0;
						for (ix=0;ix< izone; ix++){
							cx=(double)ix- (izone - 1.0)/2.0;
							
							weight =  w_pow( cx/mult1, (double) px); 
							weight *= w_pow( cy/mult2,(double) py);
							weight *= w_pow(mult3*ct, (double) pt);
							weight/= (double)(fac(px) * fac(py)* fac(pt));
							m_fWeight_Matrix[pt][py][px] += (float)(weight*weight);
						}
					}
				}
			}
		}
	}


	//Normalise Weights (This step makes no difference to results)
	double fMaxWeight=1.0f;
	for (pt=0; pt<=McGMParams.m_nTOrders; pt++){
		for (py=0; py<=McGMParams.m_nYOrders; py++){
			for (px=0; px<=McGMParams.m_nXOrders; px++){
			//TRACE("W[t:%d][y:%d][x:%d] = %f\n",pt,py,px,m_fWeight_Matrix[pt][py][px]);			
			m_fWeight_Matrix[pt][py][px]/=m_fWeight_Matrix[0][0][0];
			if (bUnityWeights){m_fWeight_Matrix[pt][py][px]=1.0;}
			if (m_fWeight_Matrix[pt][py][px]>fMaxWeight){fMaxWeight=m_fWeight_Matrix[pt][py][px];}
			//TRACE("W[t:%d][y:%d][x:%d] = %f\n",pt,py,px,m_fWeight_Matrix[pt][py][px]);			
	}}}
//	TRACE1("RATIO of SMALLEST TO LARGEST WEIGHT = %f\n",fMaxWeight);

	return true;
}

static double w_pow(double x, double y)
{
	return(y==0 ? 1.0:(x==0 ? 0.0:pow(x,y)));
}

static int fac(int x)
{
	int i, h = 1;
	if (x==0) return(1); 
	else {
		x++;
		for (i=1; i < x ;i++) h *= i;
		return(h);
	}
}

static float dotlength(float *a, float *b, int length)
{
	int i;
	float result = 0.0;

	for (i = 0; i < length; ++i) {
		result += *(a++) * *(b++);
	}

	return result;
}


	//aja 4/4/05 function to complement new mcgm
static bool GetFrameList(	list <AbstractImage*> &FrmList,
							int number,AbstractImage **ImgList)
{
	if(FrmList.empty())
		return false;
	
	list<AbstractImage*>::iterator _i;

	_i=FrmList.begin();
	int i=0;

	while(_i!=FrmList.end())
	{	
		ImgList[i]=*_i;
		//cout<<*(ImgList+i)<<endl;
		_i++;i++;
	}
	
	return true;
}

static float format0_2pi_f(float angle_rad)
{
	return angle_rad-(float)(floor(angle_rad/(2*UCL_VRL_PI))*UCL_VRL_PI*2);
}

static float formatnegpi_pi_f(float angle_rad)
{
	angle_rad-=(float)(floor(angle_rad/UCL_VRL_TWO_PI)*UCL_VRL_TWO_PI);

	if( angle_rad>UCL_VRL_PI )
		angle_rad-=(float)UCL_VRL_TWO_PI;
	
	return angle_rad;
}

static float angle_rad_f(float x,float y)
{
	
	//if( fabs(x) > 0.000001 && fabs(y) > 0.000001 )
	//	return atan2( y, x );
	//else
	//	return 0;
	
	
	if( x > 0 && y >= 0 )
		return (float) atan( y/x );
	else if( x < 0 && y > 0 )
		return (float) (UCL_VRL_PI - atan( y/-x ));
	else if( x < 0 && y <= 0 )
		return (float) (UCL_VRL_PI + atan( y/x ));
	else if( x > 0 && y < 0 )
		return (float) (UCL_VRL_TWO_PI - atan( -y/x ));
	else if( x == 0 && y > 0 )
		return (float) UCL_VRL_HALF_PI;
	else if( x < 0 && y == 0 )
		return (float) UCL_VRL_PI;
	else if ( x == 0 && y < 0 )
		return (float) UCL_VRL_THREE_OVER_TWO_PI;
	else
		return 0;
}

static void anglesrelativetoradii(	float *radout_p,const float *radin_p, 
									unsigned int w, unsigned int h,
									float originx, float originy )
{
	float radial_ang, diff;

	for(unsigned int x=0; x<w; ++x )
	{
		for(unsigned int y=0; y<h; ++y)
		{
			radial_ang=angle_rad_f(x-originx,-(y-originy));
			
			diff=*(radin_p+(y*w)+x)-radial_ang;
			*(radout_p+(y*w)+x)=format0_2pi_f(diff);
		}
	}
}

static void thresholdanglecolours(	unsigned char *angs_uc,const float *vels_f,
							unsigned int w,unsigned int h,
							float minvel)
{
	struct RGB
	{
		unsigned char r,g,b;
	};
	RGB *angs_rgb;

	unsigned int sz=w*h;
	angs_rgb=(RGB*)angs_uc;
	
	for(unsigned int i=0;i<sz;++i)
		if(vels_f[i]<minvel)
			angs_rgb[i].r=angs_rgb[i].g=angs_rgb[i].b=0;
}

static void thresholdanglecolours(	unsigned char *angs_uc,const unsigned char *vels,
									unsigned int w,unsigned int h,
									unsigned char minvel)
{
	struct RGB
	{
		unsigned char r,g,b;
	};
	RGB *angs_rgb;

	unsigned int sz=w*h;
	angs_rgb=(RGB*)angs_uc;
	
	for(unsigned int i=0;i<sz;++i)
		if(vels[i]<minvel||vels[i]==0)
			angs_rgb[i].r=angs_rgb[i].g=angs_rgb[i].b=0;
}


	//returns false if the label is not in the file
static bool ffwdtolineafterlabel(string &label,FILE *inFile_p)
{
	const unsigned int _READ_LINE_LENGTH_=200;
		//recycled c code
	static char line_p[_READ_LINE_LENGTH_];
	const char *label_p;
	char *start_p;
	
	label_p = label.c_str();
	start_p = NULL;
	
	if( strlen( label_p ) > _READ_LINE_LENGTH_-1 )
	{
		fprintf(pErrorFile,"ajaio.cpp:ffwd_lineafterlabel");
		fprintf(pErrorFile, "strlen( label_p ) (%d) for %s > line length to be read in (%d)",
								 strlen( label_p ),
								 label_p,
								 _READ_LINE_LENGTH_-1 );
		fflush( pErrorFile );
		return false;					 
	}
	
	while(  !start_p && feof( inFile_p ) == 0 )
	{
		fgets( line_p, _READ_LINE_LENGTH_, inFile_p );
		start_p = strstr( line_p, label_p );
	}
		//if the label is not in the file
	if( !start_p )
		return false;
	else
		return true;
}

/*
	//assumes floating point image
bool containsNan(AbstractImage *img)
{
	unsigned int ind;
	bool returnval=false;

	float *data=(float*)img->GetpData();
	unsigned int w=img->Width();
	unsigned int h=img->Height();

	for(int x=0;x<w;++x){
		for(int y=0;y<h;++y)
		{
			ind=(y*w)+x;
			if(data[ind]!=data[ind]){
				//fprintf(pErrorFile,"[%d][%d]\n",x,y);
				returnval=true;
			}
		}
	}
return returnval;
}

//assumes floating point image
bool containsNegative(AbstractImage *img)
{
	bool returnval=false;

	float *data=(float*)img->GetpData();
	unsigned int w=img->Width();
	unsigned int h=img->Height();

	for(int x=0;x<w;++x){
		for(int y=0;y<h;++y)
		{
			if(data[(y*w)+x]<0){
				//fprintf(pErrorFile,"[%d][%d]=%f\n",x,y,data[(y*w)+x]);
				returnval=true;
			}
		}
	}
return returnval;
}

*/





////////////////////////////////////////////////////////////////////////////
// COLOUR WHEEL_____________________________________________________________
////////////////////////////////////////////////////////////////////////////
// Maps angular data to a colour image representation (e.g. AbstractImage)
//

const double fPI=3.14159265358979323846f;
const float f2PI=6.28318530717959f;

//static float format0_2pi_f(float angle_rad);
//static float angle_rad_f( float x, float y );


ColWheelLUT::ColWheelLUT()
{
	RadToRGBLUT_p=NULL;
	LUTsize=0;
}

ColWheelLUT::ColWheelLUT(unsigned int lookuptablesize, bool invertcols)
{
	RadToRGBLUT_p=NULL;
	LUTsize=0;
	Init(lookuptablesize,invertcols);
}

ColWheelLUT &ColWheelLUT::operator=(const ColWheelLUT &cw)
{
	if (this != &cw) 
	{
		LUTsize=cw.LUTsize;
		
		if( RadToRGBLUT_p )
			delete [] RadToRGBLUT_p;

		try{
			RadToRGBLUT_p = new ColwheelRGB[LUTsize+1];
			memcpy(RadToRGBLUT_p,cw.RadToRGBLUT_p,(LUTsize+1)*sizeof(ColwheelRGB));
		}
		catch( bad_alloc xa )
		{
			puts("ColWheelLUT::operator= error allocating memory"); fflush( stdout );
			RadToRGBLUT_p=NULL;
			LUTsize=0;
		}	
	}
  return *this;
}

	
bool ColWheelLUT::Init(unsigned int lookuptablesize, bool invertcols)
{
	if(RadToRGBLUT_p)
	{
		delete [] RadToRGBLUT_p;
		LUTsize=0;
	}

	int nAngleMult=0;
	if(invertcols)
		nAngleMult=-1;
	else
		nAngleMult=1;

	///////////////////////////////////////////////////////////////////
//Initialise the LUT for fast RadToRGB conversions.
	//TRACE1("RadToRadToRGBLUTInit Size=%d  ",LUTsize);

	//Allocate one extra so we include last entry (e.g. 0-360 inclusive)
	try{
		LUTsize=lookuptablesize;
		RadToRGBLUT_p=new ColwheelRGB[LUTsize+1];
	}
	catch( bad_alloc xa )
	{
		puts("ColWheelLUT::ColWheelLUT() error allocating memory"); fflush( stdout ); 
		RadToRGBLUT_p=NULL;
		LUTsize=0;
		return false;
	}	

		//Init the Variables
	float fAngleStep=(float)(f2PI/(float)LUTsize);

	ColwheelRGB fCol;

	for (unsigned int c=0;c<=LUTsize;c++)
	{
		fCol=RadToColour(c*fAngleStep*nAngleMult);
		RadToRGBLUT_p[c].r=fCol.r;
		RadToRGBLUT_p[c].g=fCol.g;
		RadToRGBLUT_p[c].b=fCol.b;
	}

	//TRACE("Done\n");
	return true;
}



ColWheelLUT::~ColWheelLUT(){
	if(RadToRGBLUT_p)
		delete[]RadToRGBLUT_p;
	RadToRGBLUT_p=NULL;
	LUTsize=0;
}


//////////////////////////////////////////////////////////////////////
// Convert an angle to a colour
ColwheelRGB ColWheelLUT::RadToColour(float rad) const{

	double r=0,g=0,b=0;
	double degc=180.0*rad/fPI;

	ColwheelRGB col;col.r=0;col.g=0;col.b=0;
	
	double wholedeg=floor(degc/360.0);
	double deg=degc-(360.0*wholedeg);
	int ideg=(int)floor(deg);
	
	if (deg<0 || deg>360)
	{
		//TRACE("RadToColour Error\n");
		puts("ColWheelLUT::RadToColour() angle out of bounds");
		fflush(stdout);
		return col;
	}

	if (ideg>=0  && ideg<90)	{r=255;g=deg*255/90.0;b=0;}
	else if (ideg>=90 && ideg<180)	{r=(180-deg)*255/90.0;g=255;b=0;}
	else if (ideg>=180&& ideg<270)	{r=0;g=(270-deg)*255/90.0;b=(deg-180)*255/90.0;}
	else if (ideg>=270&& ideg<=360)	{r=(deg-270)*255/90.0;g=0;b=(360-deg)*255/90.0;}

	
	col.r = (unsigned char) r+0.5;
	col.g = (unsigned char) g+0.5;
	col.b = (unsigned char) b+0.5;

	return col;
}

//////////////////////////////////////////////////////////////////////
// Convert a colour to an angle
// if black arbitrarily converts to 0
float ColWheelLUT::ColourToRad(const ColwheelRGB &col) const
{
	float deg=0;
	ColwheelRGB c=col;

	if(c.r==0&&c.g==0&&c.b==0)
		return 0;

	if(c.r==255&&c.b==0)
	{	deg=c.g*(float)90/255; }
	else if(c.g==255&&c.b==0)
	{	deg=-((c.r*(float)90/255)-180); }
	else if(c.r==0)
	{	deg=(c.b*(float)90/255)+180;	}
	else if(c.g==0)
	{	deg=(c.r*(float)90/255)+270; }
	else
	{
		puts("ColWheelLUT::ColourToRad() colour code unrecognised");
		fflush(stdout);
		deg=0;
	}

	return fPI*deg/180.0f;
}

bool ColWheelLUT::RGBtoRad( const unsigned char *pSrcRgb, 
							float *pDstRadf,unsigned int size) const
{
	if(!pSrcRgb)
	{
		puts("ColWheelLUT::RGBToRad(), rgb img input=NULL");
		fflush(stdout);
		return false;
	}
	if(!pDstRadf)
	{
		puts("ColWheelLUT::RGBToRad(), rad output=NULL");
		fflush(stdout);
		return false;
	}

	for(unsigned int i=0; i<size; ++i )
	{
		pDstRadf[i]=ColourToRad( *(((ColwheelRGB*)pSrcRgb)+i) );
	}
	return true;
}


//////////////////////////////////////////////////////////////////////
//Convert a float RAD to a RGB bytes using the LUT
bool ColWheelLUT::RadToRGB(float rad, unsigned char &r, unsigned char &g, unsigned char &b) const
{
	ColwheelRGB col={0};
	if (!RadToRGBLUT_p)
	{
		r=g=b=0;
		return false;
	}

	int wholerads=(int)(floor(rad/f2PI));
	rad=rad-(f2PI*wholerads);

	int QuantisedRad=(int)((LUTsize+1)*rad/f2PI);

	col=RadToRGBLUT_p[QuantisedRad];
	r=col.r; g=col.g; b=col.b;
	
	return true;
}

ColwheelRGB ColWheelLUT::RadToRGB(float rad) const
{
	ColwheelRGB col={0};
	if (!RadToRGBLUT_p)
	{
		col.r=col.g=col.b=0;
		return col;
	}

	int wholerads=(int)(floor(rad/f2PI));
	rad=rad-(f2PI*wholerads);

	int QuantisedRad=(int)((LUTsize+1)*rad/f2PI);

	return RadToRGBLUT_p[QuantisedRad];
}

bool ColWheelLUT::RadToRGB(	const float *pSrcRadf,unsigned char *pDstRgbUchar, 
							unsigned int size) const
{
	if(!RadToRGBLUT_p)
	{
		fprintf(pErrorFile,"ColWheelLUT::RadToRGB(), Colourwheel Look-up-table=NULL");
		return false;
	}
	if(!pSrcRadf)
	{
		fprintf(pErrorFile,"ColWheelLUT::RadToRGB(), rad input=NULL");
		return false;
	}
	if(!pDstRgbUchar)
	{
		fprintf(pErrorFile,"ColWheelLUT::RadToRGB(), rgb output img=NULL");
		return false;
	}

	for(unsigned int i=0; i<size; ++i )
	{
		*(((ColwheelRGB*)pDstRgbUchar)+i)=RadToRGB(pSrcRadf[i]);
	}
	
	return true;
}

bool ColWheelLUT::RadToRGB(	const float *pSrcRadf,unsigned char *pDstRgbUchar,
							const float *pTestf,unsigned int size,float fThresh) const
{
	if(!RadToRGBLUT_p)
	{
		fprintf(pErrorFile,"ColWheelLUT::RadToRGB(), Colourwheel Look-up-table=NULL");
		return false;
	}
	if(!pSrcRadf)
	{
		fprintf(pErrorFile,"ColWheelLUT::RadToRGB(), rad input=NULL");
		return false;
	}
	if(!pDstRgbUchar)
	{
		fprintf(pErrorFile,"ColWheelLUT::RadToRGB(), rgb output img=NULL");
		return false;
	}
	if(!pTestf)
	{
		fprintf(pErrorFile,"ColWheelLUT::RadToRGB(), test img=NULL");
		return false;
	}

	ColwheelRGB black;
	black.r=black.g=black.b=0;

	for(unsigned int i=0; i<size; ++i )
	{
		if(pTestf[i]<fThresh)
			*(((ColwheelRGB*)pDstRgbUchar)+i)=black;
		else
			*(((ColwheelRGB*)pDstRgbUchar)+i)=RadToRGB(pSrcRadf[i]);
	}
	
	return true;
}

bool ColWheelLUT::RadToRGB(const double *pSrcRadd, 
						   unsigned char *pDstRgbUchar,unsigned int size) const
{
	if(!RadToRGBLUT_p)
	{
		puts("ColWheelLUT::RadToRGB(), Colourwheel Look-up-table=NULL");
		fflush(stdout);
		return false;
	}
	if(!pSrcRadd)
	{
		puts("ColWheelLUT::RadToRGB(), rad input=NULL");
		fflush(stdout);
		return false;
	}
	if(!pDstRgbUchar)
	{
		puts("ColWheelLUT::RadToRGB(), rgb output img=NULL");
		fflush(stdout);
		return false;
	}

	for(unsigned int i=0; i<size; ++i )
	{
		*(((ColwheelRGB*)pDstRgbUchar)+i)=RadToRGB((float)pSrcRadd[i]);
	}
	return true;
}

bool ColWheelLUT::AddBorder(unsigned char *pDstRgbUChar,unsigned int w,unsigned int h,
							unsigned int borderSz) const
{
	if(!pDstRgbUChar)
	{
		fprintf(pErrorFile,"ColWheelLUT::AddBorder(), pDstRgbUChar arg=NULL\n");
		return false;
	}

	if(borderSz*2>w||borderSz*2>h)
	{
		fprintf(pErrorFile,
	"ColWheelLUT::AddBorder(), borders (%d) too large for img arg (%d*%d)\n",
							borderSz,w,h);
		return false;
	}

	unsigned int i,j;
	float x,y;
	float mid_x,mid_y;

	mid_x=w/2.0f;
	mid_y=h/2.0f;

	
	for(j=0;j<borderSz;++j)
	{
		for(i=0;i<w;++i)
		{
			x=i-mid_x; y=j-mid_y;
			y=-y;
			*( ((ColwheelRGB*)pDstRgbUChar)+(j*w)+i )=RadToRGB(format0_2pi_f(angle_rad_f(x,y)));
			
			y=(h-borderSz+j)-mid_y;
			y=-y;
			*( ((ColwheelRGB*)pDstRgbUChar)+((h-borderSz+j)*w)+i )
						=RadToRGB(format0_2pi_f(angle_rad_f(x,y)));
		}
	}
	for(i=0;i<borderSz;++i)
	{
		for(j=borderSz;j<h-borderSz;++j)
		{
			x=i-mid_x; y=j-mid_y;
			y=-y;
			*( ((ColwheelRGB*)pDstRgbUChar)+(j*w)+i )=RadToRGB(format0_2pi_f(angle_rad_f(x,y)));

			x=(w-borderSz+i)-mid_x;
			*( ((ColwheelRGB*)pDstRgbUChar)+((j+1)*w)-borderSz+i )=RadToRGB(format0_2pi_f(angle_rad_f(x,y)));
		}
	}

	return true;
}

bool ColWheelLUT::RadToRGB(AbstractImage &dstRgbUchar,
						   const AbstractImage &srcF,
						   unsigned int borderSz) const
{
	if(!srcF.IsValid()||srcF.DataType()!=IMG_FLOAT||srcF.Channels()!=1){
		fprintf(pErrorFile,"ColWheelLUT::RadToRGB(), incompatible arg 'srcF', invalid or not float\n");
		return false;
	}

	if(!dstRgbUchar.IsValid()||dstRgbUchar.Channels()!=3||dstRgbUchar.DataType()!=IMG_UCHAR){
		if(!dstRgbUchar.Create(srcF.Width(),srcF.Height(),3,IMG_UCHAR)){
			fprintf(pErrorFile,"ColWheelLUT::RadToRGB(), error creating dst image\n");
		}
	}

	if(!RadToRGB((float*)srcF.Data(),(unsigned char*)dstRgbUchar.Data(),srcF.Width()*srcF.Height())){
		fprintf(pErrorFile,"ColWheelLUT::RadToRGB(), error converting to colour\n");
		return false;
	}
	if(borderSz>0){
		if(!AddBorder((unsigned char*)dstRgbUchar.Data(),dstRgbUchar.Width(),dstRgbUchar.Height(),borderSz)){
			fprintf(pErrorFile,"ColWheelLUT::RadToRGB(), error adding border\n");
		return false;
		}
	}
	return true;
}


bool ColWheelLUT::RadToRGB(	AbstractImage &dstRgbUchar,const AbstractImage &srcF,
							const AbstractImage &testF,float fThresh,
							unsigned int borderSz) const
{
	if(!srcF.IsValid()||srcF.DataType()!=IMG_FLOAT||srcF.Channels()!=1){
		fprintf(pErrorFile,
	"ColWheelLUT::RadToRGB(), incompatible arg 'srcF', invalid, not float or not greyscale\n");
		return false;
	}

	if(!dstRgbUchar.IsValid()||dstRgbUchar.Channels()!=3||dstRgbUchar.DataType()!=IMG_UCHAR){
		if(!dstRgbUchar.Create(srcF.Width(),srcF.Height(),3,IMG_UCHAR)){
			fprintf(pErrorFile,"ColWheelLUT::RadToRGB(), error creating dst image\n");
		}
	}

	if(!testF.IsValid()||testF.Channels()!=1||testF.DataType()!=IMG_FLOAT){
		fprintf(pErrorFile,"ColWheelLUT::RadToRGB(), incompatible arg 'testF', invalid, not float or not greyscale\n");
		return false;
	}

	if(!RadToRGB(	(float*)srcF.Data(),(unsigned char*)dstRgbUchar.Data(),
					(float*)testF.Data(),srcF.Width()*srcF.Height(),fThresh)){
		fprintf(pErrorFile,"ColWheelLUT::RadToRGB(), error converting to colour\n");
		return false;
	}

	if(borderSz>0){
		if(!AddBorder((unsigned char*)dstRgbUchar.Data(),dstRgbUchar.Width(),dstRgbUchar.Height(),borderSz)){
			fprintf(pErrorFile,"ColWheelLUT::RadToRGB(), error adding border\n");
		return false;
		}
	}
	return true;
}


bool ColWheelLUT::RGBtoRad(AbstractImage &dstF,const AbstractImage &srcRgbUchar) const
{
	if(!srcRgbUchar.IsValid()||srcRgbUchar.DataType()!=IMG_UCHAR||srcRgbUchar.Channels()!=3){
		fprintf(pErrorFile,
		"ColWheelLUT::RGBToRad(), incompatible arg 'srcRgbUchar', invalid, not uchar or not 3 channels\n");
		return false;
	}

	if(!dstF.IsValid()||dstF.Channels()!=1||dstF.DataType()!=IMG_FLOAT){
		if(!dstF.Create(srcRgbUchar.Width(),srcRgbUchar.Height(),1,IMG_FLOAT)){
			fprintf(pErrorFile,"ColWheelLUT::RGBToRad(), error creating dst image\n");
			return false;
		}
	}

	if(!RGBtoRad((unsigned char*)srcRgbUchar.Data(),(float*)dstF.Data(),dstF.Width()*dstF.Height())){
		fprintf(pErrorFile,"ColWheelLUT::RGBToRad(), error converting from colour\n");
		return false;
	}
	
	return true;
}


////////////////////////////////////////////////////////////////////////////
// SPACE VARIANT MAPPINGS__________________________________________________________
////////////////////////////////////////////////////////////////////////////
//
// supports log polar transforms and Alan Johnston's Conic model

const int SPACE_VAR_MAP_OUT_OF_RANGE=-1;


////////////////////////////////////////////////////////////////////////////
// LOG POLAR MAGNIFICATION_____________________________________________________________
////////////////////////////////////////////////////////////////////////////
//
//	aja 2005

LogPolarMap::LogPolarMap()
{
	m_hdr.srcW=m_hdr.srcH=m_hdr.dstW=m_hdr.dstH=0;
	m_hdr.alpha=1;
	m_hdr.beta=1.4;

	m_pSpatialMap=NULL;
	m_pSpatialMap2=NULL;
}

LogPolarMap::LogPolarMap(float alpha,float beta)
{
	m_hdr.srcW=m_hdr.srcH=m_hdr.dstW=m_hdr.dstH=0;
	m_hdr.alpha=alpha;
	m_hdr.beta=beta;

	m_pSpatialMap=NULL;
	m_pSpatialMap2=NULL;
}

LogPolarMap::LogPolarMap(	float alpha,float beta,
					unsigned int dstWidth,unsigned int dstHeight,
					unsigned int srcWidth,unsigned int srcHeight
					)
{
	if(!Create(alpha,beta,dstWidth,dstHeight,srcWidth,srcHeight)){
		fprintf(pErrorFile,"LogPolarMap::LogPolarMap(), Error creating log polar maps\n");
	}
}

LogPolarMap::LogPolarMap(const LogPolarMap &src)
{
	if(!Copy(src)){
		fprintf(pErrorFile,"LogPolarMap::LogPolarMap(), error copying\n");
	}
}

LogPolarMap &LogPolarMap::operator=(const LogPolarMap &src)
{
	if (this!=&src) 
	{
		if(!Copy(src)){
			fprintf(pErrorFile,"LogPolarMap::operator=(), error copying maps\n");	
		}
	}
	return *this;
}

LogPolarMap::~LogPolarMap()
{
	if(!Destroy()){
		fprintf(pErrorFile,"LogPolarMap::~LogPolarMap(), error destroying object\n");	
	}	
}

bool LogPolarMap::Create(	float alpha,float beta,
						unsigned int dstWidth,unsigned int dstHeight,
						unsigned int srcWidth,unsigned int srcHeight
						)
{
	LogPolarMapHdr tempHdr;

	tempHdr.alpha=alpha;
	tempHdr.beta=beta;
	tempHdr.srcW=srcWidth;
	tempHdr.srcH=srcHeight;
	tempHdr.dstW=dstWidth;
	tempHdr.dstH=dstHeight;

	if(memcmp(&tempHdr,&m_hdr,sizeof(LogPolarMapHdr))==0)
	{
		CalcMaps();
		return true;
	}
	else
	{
		Destroy();
		memcpy(&m_hdr,&tempHdr,sizeof(LogPolarMapHdr));
	}
	
	if(!AllocMaps())
	{
		fprintf(pErrorFile,"LogPolarMap::Create() memory allocation failure\n");
		Destroy();
		
		return false;
	}

	CalcMaps();

	return true;
}

bool LogPolarMap::Copy(const LogPolarMap &src)
{
	if(this==&src)
	{
		fprintf(pErrorFile,"LogPolarMap::Copy(), arg 'src' == this\n");
		return false;
	}

	unsigned int sz=src.m_hdr.dstW*src.m_hdr.dstH;

	if(memcmp(&m_hdr,&src.m_hdr,sizeof(LogPolarMap))==0)
	{
		if(!m_pSpatialMap)
			memcpy(m_pSpatialMap,src.m_pSpatialMap,sz*sizeof(Coordinate));

		if(!m_pSpatialMap2)
			memcpy(m_pSpatialMap2,src.m_pSpatialMap2,sz*sizeof(Coordinate));

		return true;
	}

	memcpy(&m_hdr,&src.m_hdr,sizeof(LogPolarMapHdr));

	if(!AllocMaps())
	{
		fprintf(pErrorFile,"LogPolarMap::Copy() memory allocation failure");
		Destroy();
		return false;
	}
	
	memcpy(m_pSpatialMap,src.m_pSpatialMap,sz*sizeof(Coordinate));
	memcpy(m_pSpatialMap2,src.m_pSpatialMap2,sz*sizeof(Coordinate));
	
	return true;
}

bool LogPolarMap::Destroy()
{
	if(m_pSpatialMap){
		delete [] m_pSpatialMap;
		m_pSpatialMap=NULL;
	}

	if(m_pSpatialMap2){
		delete [] m_pSpatialMap2;
		m_pSpatialMap2=NULL;
	}

	memset(&m_hdr,0,sizeof(LogPolarMapHdr));
	m_hdr.alpha=1;
	m_hdr.beta=1.4;

	return true;
}

bool LogPolarMap::AllocMaps()
{
	if( m_pSpatialMap )
		delete [] m_pSpatialMap;

	if( m_pSpatialMap2 )
		delete [] m_pSpatialMap2;
	
	try{
		m_pSpatialMap = new Coordinate[m_hdr.dstW*m_hdr.dstH];
		m_pSpatialMap2 = new Coordinate[m_hdr.dstW*m_hdr.dstH];
	}
	catch( bad_alloc xa )
	{
		fprintf(pErrorFile,"LogPolarMap::AllocMaps(), Error allocating memory");
		return false;
	}
	return true;
}

bool LogPolarMap::ToFArraysMap(float *x,float *y,unsigned int sz) const
{
	if(!x||!y){
		fprintf(pErrorFile,"LogPolarMap::ToFArraysMap(), arg x and/or y ==NULL");
		return false;
	}

	if(sz!=m_hdr.dstW*m_hdr.dstH){
		fprintf(pErrorFile,
	"LogPolarMap::ToFArraysMap(), arg 'sz' (%d) doesn't match map size (%d)",
				sz,m_hdr.dstW*m_hdr.dstH);
		return false;
	}

	for(int i=0;i<sz;++i)
	{
		x[i]=m_pSpatialMap[i].x;
		y[i]=m_pSpatialMap[i].y;
	}

	return true;
}

bool LogPolarMap::ToFArraysMap2(float *x,float *y,unsigned int sz) const
{
	if(!x||!y){
		fprintf(pErrorFile,"LogPolarMap::ToFArraysMap2(), arg x and/or y ==NULL");
		return false;
	}

	if(sz!=m_hdr.dstW*m_hdr.dstH){
		fprintf(pErrorFile,
	"LogPolarMap::ToFArraysMap2(), arg 'sz' (%d) doesn't match map size (%d)",
				sz,m_hdr.dstW*m_hdr.dstH);
		return false;
	}

	for(int i=0;i<sz;++i)
	{
		x[i]=m_pSpatialMap2[i].x;
		y[i]=m_pSpatialMap2[i].y;
	}
	return true;
}

void LogPolarMap::CalcMaps()
{
	const float srcW=m_hdr.srcW;
	const float srcH=m_hdr.srcH;

	const float dstW=m_hdr.dstW;
	const float dstH=m_hdr.dstH;

	const float dst_mid_x=dstW/2.0f;
	const float dst_mid_y=dstH/2.0f;

	const float src_mid_x=srcW/2.0f;
	const float src_mid_y=srcH/2.0f;

	float X,Y,theta,srcRadius,dstRadius;
	const float max_src_radius=sqrt((srcW*srcW)+(srcH*srcH))/2.0f;
	const float max_dst_radius=sqrt((dstW*dstW)+(dstH*dstH))/2.0f;
	
	unsigned int index;

	float alpha=m_hdr.alpha;
	float beta=m_hdr.beta;

	float inc=(UCL_VRL_TWO_PI)/dstH;

	for (int _x=0;_x<dstW;++_x){
		for (int _y=0;_y<dstH;++_y){
			index=(_y*dstW)+_x;
			X=_x-dst_mid_x; Y=_y-dst_mid_y;
			dstRadius = (float)sqrt( (X*X)+(Y*Y) );
			theta = (float)atan2(Y,X);
			
			dstRadius/=max_dst_radius;
			srcRadius=(float)exp(dstRadius*alpha)-1.0f;
			srcRadius/=(float)exp(alpha/beta);
			srcRadius*=max_src_radius;

			m_pSpatialMap[index].x=src_mid_x+(srcRadius*cos(theta));
			m_pSpatialMap[index].y=src_mid_y+(srcRadius*sin(theta));

			if(m_pSpatialMap[index].x>=srcW||m_pSpatialMap[index].y>=srcH)
			{
				m_pSpatialMap[index].x=SPACE_VAR_MAP_OUT_OF_RANGE;
				m_pSpatialMap[index].y=SPACE_VAR_MAP_OUT_OF_RANGE;
			}

				//map2

			theta=_y*inc;
			dstRadius=_x;
		
			dstRadius/=(float)dstW;
			srcRadius=(float)exp(dstRadius*alpha)-1.0f;
			srcRadius/=(float)exp(alpha/beta);
			srcRadius*=(float)max_src_radius;

			m_pSpatialMap2[index].x=src_mid_x+(srcRadius*cos(theta));
			m_pSpatialMap2[index].y=src_mid_y+(srcRadius*sin(theta));

			if(m_pSpatialMap2[index].x>=srcW||m_pSpatialMap2[index].y>=srcH)
			{
				m_pSpatialMap2[index].x=SPACE_VAR_MAP_OUT_OF_RANGE;
				m_pSpatialMap2[index].y=SPACE_VAR_MAP_OUT_OF_RANGE;
			}
		}
	}
}


bool LogPolarMap::Magnify(AbstractImage &dst,const AbstractImage &src) const
{
	unsigned int x,y;

	switch(dst.DataType())
	{
	case IMG_UCHAR:	{
						unsigned char *pSrcData, *pDstPix;
						pSrcData=(unsigned char*)src.Data();
						pDstPix=(unsigned char*)dst.Data();
						Coordinate *pMapPix=m_pSpatialMap;
						
						for (y=0;y<m_hdr.dstH;++y){
							for(x=0;x<m_hdr.dstW;++x){
							
								if(pMapPix->x!=SPACE_VAR_MAP_OUT_OF_RANGE)
								{
									BilinearInterpolatePixel(	pDstPix, //+(dst.Channels()*((y*m_hdr.dstW)+x)),
																pSrcData,
																src.Channels(),src.Width(),
																src.Height(),
																pMapPix->x,
																pMapPix->y);
								}
								else
									memset(pDstPix,//+(y*m_hdr.dstW)+x,
											0,dst.Channels()*sizeof(unsigned char));

								pDstPix+=dst.Channels();
								pMapPix++;
							}
						}
					} break;
	case IMG_LONG:	{
						long *pSrcData, *pDstPix;
						pSrcData=(long*)src.Data();
						pDstPix=(long*)dst.Data();
						Coordinate *pMapPix=m_pSpatialMap;
						
						for (y=0;y<m_hdr.dstH;++y){
							for(x=0;x<m_hdr.dstW;++x){
							
								if(pMapPix->x!=SPACE_VAR_MAP_OUT_OF_RANGE)
								{
									BilinearInterpolatePixel(	pDstPix, //+(dst.Channels()*((y*m_hdr.dstW)+x)),
																pSrcData,
																src.Channels(),src.Width(),
																src.Height(),
																pMapPix->x,
																pMapPix->y);
								}
								else
									memset(pDstPix,//+(y*m_hdr.dstW)+x,
											0,dst.Channels()*sizeof(long));

								pDstPix+=dst.Channels();
								pMapPix++;
							}
						}
					} break;
	case IMG_FLOAT: {
						float *pSrcData, *pDstPix;
						pSrcData=(float*)src.Data();
						pDstPix=(float*)dst.Data();
						Coordinate *pMapPix=m_pSpatialMap;
						
						for (y=0;y<m_hdr.dstH;++y){
							for(x=0;x<m_hdr.dstW;++x){
							
								if(pMapPix->x!=SPACE_VAR_MAP_OUT_OF_RANGE)
								{
									BilinearInterpolatePixel(	pDstPix, //+(dst.Channels()*((y*m_hdr.dstW)+x)),
																pSrcData,
																src.Channels(),src.Width(),
																src.Height(),
																pMapPix->x,
																pMapPix->y);
								}
								else
									memset(pDstPix,//+(y*m_hdr.dstW)+x,
											0,dst.Channels()*sizeof(float));

								pDstPix+=dst.Channels();
								pMapPix++;
							}
						}
					} break;
	default:		{
						fprintf(pErrorFile,"LogPolarMap::Magnify(), unrecognised datatype\n");
						return false;
					}
	}

	return true;
}


bool LogPolarMap::Magnify2(AbstractImage &dst,const AbstractImage &src) const
{
	unsigned int x,y;

	switch(dst.DataType())
	{
	case IMG_UCHAR:	{
						unsigned char *pSrcData, *pDstPix;
						pSrcData=(unsigned char*)src.Data();
						pDstPix=(unsigned char*)dst.Data();
						Coordinate *pMapPix=m_pSpatialMap2;
						
						for (y=0;y<m_hdr.dstH;++y){
							for(x=0;x<m_hdr.dstW;++x){
							
								if(pMapPix->x!=SPACE_VAR_MAP_OUT_OF_RANGE)
								{
									BilinearInterpolatePixel(	pDstPix, //+(dst.Channels()*((y*m_hdr.dstW)+x)),
																pSrcData,
																src.Channels(),src.Width(),
																src.Height(),
																pMapPix->x,
																pMapPix->y);
								}
								else
									memset(pDstPix,//+(y*m_hdr.dstW)+x,
											0,dst.Channels()*sizeof(unsigned char));

								pDstPix+=dst.Channels();
								pMapPix++;
							}
						}
					} break;
	case IMG_LONG:	{
						long *pSrcData, *pDstPix;
						pSrcData=(long*)src.Data();
						pDstPix=(long*)dst.Data();
						Coordinate *pMapPix=m_pSpatialMap2;
						
						for (y=0;y<m_hdr.dstH;++y){
							for(x=0;x<m_hdr.dstW;++x){
							
								if(pMapPix->x!=SPACE_VAR_MAP_OUT_OF_RANGE)
								{
									BilinearInterpolatePixel(	pDstPix, //+(dst.Channels()*((y*m_hdr.dstW)+x)),
																pSrcData,
																src.Channels(),src.Width(),
																src.Height(),
																pMapPix->x,
																pMapPix->y);
								}
								else
									memset(pDstPix,//+(y*m_hdr.dstW)+x,
											0,dst.Channels()*sizeof(long));

								pDstPix+=dst.Channels();
								pMapPix++;
							}
						}
					} break;
	case IMG_FLOAT: {
						float *pSrcData, *pDstPix;
						pSrcData=(float*)src.Data();
						pDstPix=(float*)dst.Data();
						Coordinate *pMapPix=m_pSpatialMap2;
						
						for (y=0;y<m_hdr.dstH;++y){
							for(x=0;x<m_hdr.dstW;++x){
							
								if(pMapPix->x!=SPACE_VAR_MAP_OUT_OF_RANGE)
								{
									BilinearInterpolatePixel(	pDstPix, //+(dst.Channels()*((y*m_hdr.dstW)+x)),
																pSrcData,
																src.Channels(),src.Width(),
																src.Height(),
																pMapPix->x,
																pMapPix->y);
								}
								else
									memset(pDstPix,//+(y*m_hdr.dstW)+x,
											0,dst.Channels()*sizeof(float));

								pDstPix+=dst.Channels();
								pMapPix++;
							}
						}
					} break;
	default:		{
						fprintf(pErrorFile,"LogPolarMap::Magnify2(), unrecognised datatype\n");
						return false;
					}
	}

	return true;
}


bool LogPolarMap::Process(	AbstractImage &dst,const AbstractImage &src)
{
	if(!dst.IsValid()||!src.IsValid()){
		fprintf(pErrorFile,"LogPolarMap::Process(), arg 'src' or arg 'dst' invalid\n");
		return false;
	}

	if(dst.Channels()!=src.Channels()){
		fprintf(pErrorFile,"LogPolarMap::Process(), args 'src' and arg 'dst' have different no. channels\n");
		return false;
	}

	if(dst.DataType()!=src.DataType()){
		fprintf(pErrorFile,"LogPolarMap::Process(), args 'src' and arg 'dst' different datatypes\n");
		return false;
	}

		// if the img arg sizes don't match those stored
		// recalculate parameters and maps
	if( src.Width()!=m_hdr.srcW||src.Height()!=m_hdr.srcH 
		||dst.Width()!=m_hdr.dstW||dst.Height()!=m_hdr.dstH )
	{
		m_hdr.srcW=src.Width(); m_hdr.srcH=src.Height();
		m_hdr.dstW=dst.Width(); m_hdr.dstH=dst.Height();

		if(!AllocMaps())
		{
			fprintf(pErrorFile,"LogPolarMap::Process(), memory allocation failure");
			Destroy();		
			return false;
		}
		CalcMaps();
	}

	if(!Magnify(dst,src)){
		fprintf(pErrorFile,"LogPolarMap::Process(), error magnifying image\n");
		return false;
	}
	
	return true;
}

bool LogPolarMap::Process2(	AbstractImage &dst,const AbstractImage &src)
{
	if(!dst.IsValid()||!src.IsValid()){
		fprintf(pErrorFile,"LogPolarMap::Process2(), arg 'src' or arg 'dst' invalid\n");
		return false;
	}

	if(dst.Channels()!=src.Channels()){
		fprintf(pErrorFile,"LogPolarMap::Process2(), args 'src' and arg 'dst' have different no. channels\n");
		return false;
	}

	if(dst.DataType()!=src.DataType()){
		fprintf(pErrorFile,"LogPolarMap::Process2(), args 'src' and arg 'dst' different datatypes\n");
		return false;
	}

		// if the img arg sizes don't match those stored
		// recalculate parameters and maps
	if( src.Width()!=m_hdr.srcW||src.Height()!=m_hdr.srcH 
		||dst.Width()!=m_hdr.dstW||dst.Height()!=m_hdr.dstH )
	{
		m_hdr.srcW=src.Width(); m_hdr.srcH=src.Height();
		m_hdr.dstW=dst.Width(); m_hdr.dstH=dst.Height();

		if(!AllocMaps())
		{
			fprintf(pErrorFile,"LogPolarMap::Process2(), memory allocation failure");
			Destroy();		
			return false;
		}
		CalcMaps();
	}

	if(!Magnify2(dst,src)){
		fprintf(pErrorFile,"LogPolarMap::Process2(), error magnifying image\n");
		return false;
	}
	
	return true;
}



////////////////////////////////////////////////////////////////////////////
// LOG POLAR MAGNIFICATION_____________________________________________________________
// Implementation of log(z+a) model
////////////////////////////////////////////////////////////////////////////
//
//	aja 2005



LogPolarMap_ZPlusA::LogPolarMap_ZPlusA()
{
	m_hdr.srcW=m_hdr.srcH=m_hdr.dstW=m_hdr.dstH=0;
	m_hdr.a=0;

	m_pSpatialMap=NULL;
	m_pSpatialButterflyMap=NULL;
	m_pScaleMap=NULL;
}

LogPolarMap_ZPlusA::LogPolarMap_ZPlusA(float a)
{
	m_hdr.srcW=m_hdr.srcH=m_hdr.dstW=m_hdr.dstH=0;
	m_hdr.a=a;

	m_pSpatialMap=NULL;
	m_pSpatialButterflyMap=NULL;
	m_pScaleMap=NULL;
}

LogPolarMap_ZPlusA::LogPolarMap_ZPlusA(	float a,
					unsigned int dstWidth,unsigned int dstHeight,
					unsigned int srcWidth,unsigned int srcHeight
					)
{
	if(!Create(a,dstWidth,dstHeight,srcWidth,srcHeight)){
		fprintf(pErrorFile,"LogPolarMap_ZPlusA::LogPolarMap_ZPlusA(), Error creating log polar maps\n");
	}
}

LogPolarMap_ZPlusA::LogPolarMap_ZPlusA(const LogPolarMap_ZPlusA &src)
{
	if(!Copy(src)){
		fprintf(pErrorFile,"LogPolarMap_ZPlusA::LogPolarMap_ZPlusA(), error copying\n");
	}
}

LogPolarMap_ZPlusA &LogPolarMap_ZPlusA::operator=(const LogPolarMap_ZPlusA &src)
{
	if (this!=&src) 
	{
		if(!Copy(src)){
			fprintf(pErrorFile,"LogPolarMap_ZPlusA::operator=(), error copying maps\n");	
		}
	}
	return *this;
}

LogPolarMap_ZPlusA::~LogPolarMap_ZPlusA()
{
	if(!Destroy()){
		fprintf(pErrorFile,"LogPolarMap_ZPlusA::~LogPolarMap_ZPlusA(), error destroying object\n");	
	}	
}

bool LogPolarMap_ZPlusA::Create(	float a,
						unsigned int dstWidth,unsigned int dstHeight,
						unsigned int srcWidth,unsigned int srcHeight
						)
{
	LogPolarMap_ZPlusAHdr tempHdr;

	tempHdr.a=a;
	tempHdr.srcW=srcWidth;
	tempHdr.srcH=srcHeight;
	tempHdr.dstW=dstWidth;
	tempHdr.dstH=dstHeight;

	if(memcmp(&tempHdr,&m_hdr,sizeof(LogPolarMap_ZPlusAHdr))==0)
	{
		CalcMaps();
		return true;
	}
	else
	{
		Destroy();
		memcpy(&m_hdr,&tempHdr,sizeof(LogPolarMap_ZPlusAHdr));
	}
	
	if(!AllocMaps())
	{
		fprintf(pErrorFile,"LogPolarMap_ZPlusA::Create() memory allocation failure\n");
		Destroy();
		
		return false;
	}

	CalcMaps();

	return true;
}

bool LogPolarMap_ZPlusA::Copy(const LogPolarMap_ZPlusA &src)
{
	if(this==&src)
	{
		fprintf(pErrorFile,"LogPolarMap_ZPlusA::Copy(), arg 'src' == this\n");
		return false;
	}

	unsigned int sz=src.m_hdr.dstW*src.m_hdr.dstH;

	if(memcmp(&m_hdr,&src.m_hdr,sizeof(LogPolarMap_ZPlusA))==0)
	{
		if(m_pSpatialMap&&m_pSpatialButterflyMap&&m_pScaleMap)
		{
			memcpy(m_pSpatialMap,src.m_pSpatialMap,sz*sizeof(Coordinate));
			memcpy(m_pSpatialButterflyMap,src.m_pSpatialButterflyMap,sz*sizeof(Coordinate));
			memcpy(m_pScaleMap,src.m_pScaleMap,sz*sizeof(float));

			return true;
		}
	}

	memcpy(&m_hdr,&src.m_hdr,sizeof(LogPolarMap_ZPlusAHdr));

	if(!AllocMaps())
	{
		fprintf(pErrorFile,"LogPolarMap_ZPlusA::Copy() memory allocation failure");
		Destroy();
		return false;
	}
	
	memcpy(m_pSpatialMap,src.m_pSpatialMap,sz*sizeof(Coordinate));
	memcpy(m_pSpatialButterflyMap,src.m_pSpatialButterflyMap,sz*sizeof(Coordinate));
	memcpy(m_pScaleMap,src.m_pScaleMap,sz*sizeof(float));
	
	return true;
}

bool LogPolarMap_ZPlusA::Destroy()
{
	if(m_pSpatialMap){
		delete [] m_pSpatialMap;
		m_pSpatialMap=NULL;
	}

	if(m_pSpatialButterflyMap){
		delete [] m_pSpatialButterflyMap;
		m_pSpatialButterflyMap=NULL;
	}

	if(m_pSpatialButterflyMap){
		delete [] m_pScaleMap;
		m_pScaleMap=NULL;
	}

	memset(&m_hdr,0,sizeof(LogPolarMap_ZPlusAHdr));
	m_hdr.a=0;

	return true;
}

bool LogPolarMap_ZPlusA::AllocMaps()
{
	if( m_pSpatialMap )
		delete [] m_pSpatialMap;

	if( m_pSpatialButterflyMap )
		delete [] m_pSpatialButterflyMap;

	if( m_pScaleMap )
		delete [] m_pScaleMap;
	
	try{
		m_pSpatialMap = new Coordinate[m_hdr.dstW*m_hdr.dstH];
		m_pSpatialButterflyMap = new Coordinate[m_hdr.dstW*m_hdr.dstH];
		m_pScaleMap = new float[m_hdr.dstW*m_hdr.dstH];
	}
	catch( bad_alloc xa )
	{
		fprintf(pErrorFile,"LogPolarMap_ZPlusA::AllocMaps(), Error allocating memory");
		return false;
	}
	return true;
}

bool LogPolarMap_ZPlusA::ToFArraysMap(float *x,float *y,unsigned int sz) const
{
	if(!x||!y){
		fprintf(pErrorFile,"LogPolarMap_ZPlusA::ToFArraysMap(), arg x and/or y ==NULL");
		return false;
	}

	if(sz!=m_hdr.dstW*m_hdr.dstH){
		fprintf(pErrorFile,
	"LogPolarMap_ZPlusA::ToFArraysMap(), arg 'sz' (%d) doesn't match map size (%d)",
				sz,m_hdr.dstW*m_hdr.dstH);
		return false;
	}

	for(int i=0;i<sz;++i)
	{
		x[i]=m_pSpatialMap[i].x;
		y[i]=m_pSpatialMap[i].y;
	}

	return true;
}

bool LogPolarMap_ZPlusA::ToFArraysButterflyMap(float *x,float *y,unsigned int sz) const
{
	if(!x||!y){
		fprintf(pErrorFile,"LogPolarMap_ZPlusA::ToFArraysMap2(), arg x and/or y ==NULL");
		return false;
	}

	if(sz!=m_hdr.dstW*m_hdr.dstH){
		fprintf(pErrorFile,
	"LogPolarMap_ZPlusA::ToFArraysMap2(), arg 'sz' (%d) doesn't match map size (%d)",
				sz,m_hdr.dstW*m_hdr.dstH);
		return false;
	}

	for(int i=0;i<sz;++i)
	{
		x[i]=m_pSpatialButterflyMap[i].x;
		y[i]=m_pSpatialButterflyMap[i].y;
	}
	return true;
}

void LogPolarMap_ZPlusA::CalcMaps()
{
	const float srcW=m_hdr.srcW;
	const float srcH=m_hdr.srcH;
	const float dstW=m_hdr.dstW;
	const float dstH=m_hdr.dstH;

	const float dst_mid_x=dstW/2.0f;
	const float dst_mid_y=dstH/2.0f;
	const float src_mid_x=srcW/2.0f;
	const float src_mid_y=srcH/2.0f;

	float X,Y,theta,srcRadius,dstRadius;
	const float src_max_radius=sqrt((srcW*srcW)+(srcH*srcH))/2.0f;
	const float dst_max_radius=sqrt((dstW*dstW)+(dstH*dstH))/2.0f;
	
	const float a=m_hdr.a;

	unsigned int index;
	const float inc=(UCL_VRL_PI)/dstH;
	float scale=0.04;
	
	for (int _y=0;_y<dstH;++_y){
		for (int _x=0;_x<dstW;++_x){
		
			index=(_y*dstW)+_x;
			X=_x-dst_mid_x; Y=_y-dst_mid_y;
			dstRadius = (float)sqrt( (X*X)+(Y*Y) );
			theta = (float)atan2(Y,X);
			
			//radius=(float)exp(radius*z)-1.0f;
			//radius/=(float)exp(z/a);

			dstRadius*=scale;
			srcRadius=exp(dstRadius)-a;
			
			//srcRadius*=src_max_radius;
			//printf("%f ",srcRadius);
		
			m_pSpatialMap[index].x=src_mid_x+(srcRadius*cos(theta));
			m_pSpatialMap[index].y=src_mid_y+(srcRadius*sin(theta));
			
			m_pScaleMap[index]=(dstRadius/dst_max_radius)/(srcRadius/src_max_radius);

			if(m_pSpatialMap[index].x>=srcW||m_pSpatialMap[index].y>=srcH)
			{
				m_pSpatialMap[index].x=SPACE_VAR_MAP_OUT_OF_RANGE;
				m_pSpatialMap[index].y=SPACE_VAR_MAP_OUT_OF_RANGE;
				m_pScaleMap[index]=0;
			}

				//map2

			if(X>=0)
				theta=-UCL_VRL_HALF_PI+(_y*inc);
			else
				theta=UCL_VRL_THREE_OVER_TWO_PI-(_y*inc);
		
			dstRadius=fabs(X);

			//radius=(float)exp(radius*z)-1.0f;
			//radius/=(float)exp(z/a);
			
			//srcRadius=dstRadius;
			dstRadius*=scale;
			srcRadius=exp(dstRadius)-a;
			
			//srcRadius*=(float)src_max_radius;

			
			m_pSpatialButterflyMap[index].x=src_mid_x+(srcRadius*cos(theta));
			m_pSpatialButterflyMap[index].y=src_mid_y+(srcRadius*sin(theta));

			if(srcRadius<0){
				m_pSpatialButterflyMap[index].x=SPACE_VAR_MAP_OUT_OF_RANGE;
				m_pSpatialButterflyMap[index].y=SPACE_VAR_MAP_OUT_OF_RANGE;
			}

			if(m_pSpatialButterflyMap[index].x>=srcW
							||m_pSpatialButterflyMap[index].y>=srcH)
			{
				m_pSpatialButterflyMap[index].x=SPACE_VAR_MAP_OUT_OF_RANGE;
				m_pSpatialButterflyMap[index].y=SPACE_VAR_MAP_OUT_OF_RANGE;
			}
		}
	}
}


bool LogPolarMap_ZPlusA::Magnify(AbstractImage &dst,const AbstractImage &src) const
{
	unsigned int x,y;

	switch(dst.DataType())
	{
	case IMG_UCHAR:	{
						unsigned char *pSrcData, *pDstPix;
						pSrcData=(unsigned char*)src.Data();
						pDstPix=(unsigned char*)dst.Data();
						Coordinate *pMapPix=m_pSpatialMap;
						
						for (y=0;y<m_hdr.dstH;++y){
							for(x=0;x<m_hdr.dstW;++x){
							
								if(pMapPix->x!=SPACE_VAR_MAP_OUT_OF_RANGE)
								{
									BilinearInterpolatePixel(	pDstPix, //+(dst.Channels()*((y*m_hdr.dstW)+x)),
																pSrcData,
																src.Channels(),src.Width(),
																src.Height(),
																pMapPix->x,
																pMapPix->y);
								}
								else
									memset(pDstPix,//+(y*m_hdr.dstW)+x,
											0,dst.Channels()*sizeof(unsigned char));

								pDstPix+=dst.Channels();
								pMapPix++;
							}
						}
					} break;
	case IMG_LONG:	{
						long *pSrcData, *pDstPix;
						pSrcData=(long*)src.Data();
						pDstPix=(long*)dst.Data();
						Coordinate *pMapPix=m_pSpatialMap;
						
						for (y=0;y<m_hdr.dstH;++y){
							for(x=0;x<m_hdr.dstW;++x){
							
								if(pMapPix->x!=SPACE_VAR_MAP_OUT_OF_RANGE)
								{
									BilinearInterpolatePixel(	pDstPix, //+(dst.Channels()*((y*m_hdr.dstW)+x)),
																pSrcData,
																src.Channels(),src.Width(),
																src.Height(),
																pMapPix->x,
																pMapPix->y);
								}
								else
									memset(pDstPix,//+(y*m_hdr.dstW)+x,
											0,dst.Channels()*sizeof(long));

								pDstPix+=dst.Channels();
								pMapPix++;
							}
						}
					} break;
	case IMG_FLOAT: {
						float *pSrcData, *pDstPix;
						pSrcData=(float*)src.Data();
						pDstPix=(float*)dst.Data();
						Coordinate *pMapPix=m_pSpatialMap;
						
						for (y=0;y<m_hdr.dstH;++y){
							for(x=0;x<m_hdr.dstW;++x){
							
								if(pMapPix->x!=SPACE_VAR_MAP_OUT_OF_RANGE)
								{
									BilinearInterpolatePixel(	pDstPix, //+(dst.Channels()*((y*m_hdr.dstW)+x)),
																pSrcData,
																src.Channels(),src.Width(),
																src.Height(),
																pMapPix->x,
																pMapPix->y);
								}
								else
									memset(pDstPix,//+(y*m_hdr.dstW)+x,
											0,dst.Channels()*sizeof(float));

								pDstPix+=dst.Channels();
								pMapPix++;
							}
						}
					} break;
	default:		{
						fprintf(pErrorFile,"LogPolarMap_ZPlusA::Magnify(), unrecognised datatype\n");
						return false;
					}
	}

	return true;
}


bool LogPolarMap_ZPlusA::MagnifyButterfly(AbstractImage &dst,const AbstractImage &src) const
{
	unsigned int x,y;

	switch(dst.DataType())
	{
	case IMG_UCHAR:	{
						unsigned char *pSrcData, *pDstPix;
						pSrcData=(unsigned char*)src.Data();
						pDstPix=(unsigned char*)dst.Data();
						Coordinate *pMapPix=m_pSpatialButterflyMap;
						
						for (y=0;y<m_hdr.dstH;++y){
							for(x=0;x<m_hdr.dstW;++x){
							
								if(pMapPix->x!=SPACE_VAR_MAP_OUT_OF_RANGE)
								{
									BilinearInterpolatePixel(	pDstPix, //+(dst.Channels()*((y*m_hdr.dstW)+x)),
																pSrcData,
																src.Channels(),src.Width(),
																src.Height(),
																pMapPix->x,
																pMapPix->y);
								}
								else
									memset(pDstPix,//+(y*m_hdr.dstW)+x,
											0,dst.Channels()*sizeof(unsigned char));

								pDstPix+=dst.Channels();
								pMapPix++;
							}
						}
					} break;
	case IMG_LONG:	{
						long *pSrcData, *pDstPix;
						pSrcData=(long*)src.Data();
						pDstPix=(long*)dst.Data();
						Coordinate *pMapPix=m_pSpatialButterflyMap;
						
						for (y=0;y<m_hdr.dstH;++y){
							for(x=0;x<m_hdr.dstW;++x){
							
								if(pMapPix->x!=SPACE_VAR_MAP_OUT_OF_RANGE)
								{
									BilinearInterpolatePixel(	pDstPix, //+(dst.Channels()*((y*m_hdr.dstW)+x)),
																pSrcData,
																src.Channels(),src.Width(),
																src.Height(),
																pMapPix->x,
																pMapPix->y);
								}
								else
									memset(pDstPix,//+(y*m_hdr.dstW)+x,
											0,dst.Channels()*sizeof(long));

								pDstPix+=dst.Channels();
								pMapPix++;
							}
						}
					} break;
	case IMG_FLOAT: {
						float *pSrcData, *pDstPix;
						pSrcData=(float*)src.Data();
						pDstPix=(float*)dst.Data();
						Coordinate *pMapPix=m_pSpatialButterflyMap;
						
						for (y=0;y<m_hdr.dstH;++y){
							for(x=0;x<m_hdr.dstW;++x){
							
								if(pMapPix->x!=SPACE_VAR_MAP_OUT_OF_RANGE)
								{
									BilinearInterpolatePixel(	pDstPix, //+(dst.Channels()*((y*m_hdr.dstW)+x)),
																pSrcData,
																src.Channels(),src.Width(),
																src.Height(),
																pMapPix->x,
																pMapPix->y);
								}
								else
									memset(pDstPix,//+(y*m_hdr.dstW)+x,
											0,dst.Channels()*sizeof(float));

								pDstPix+=dst.Channels();
								pMapPix++;
							}
						}
					} break;
	default:		{
						fprintf(pErrorFile,"LogPolarMap_ZPlusA::MagnifyButterfly(), unrecognised datatype\n");
						return false;
					}
	}

	return true;
}


bool LogPolarMap_ZPlusA::Process(	AbstractImage &dst,const AbstractImage &src)
{
	if(!dst.IsValid()||!src.IsValid()){
		fprintf(pErrorFile,"LogPolarMap_ZPlusA::Process(), arg 'src' or arg 'dst' invalid\n");
		return false;
	}

	if(dst.Channels()!=src.Channels()){
		fprintf(pErrorFile,"LogPolarMap_ZPlusA::Process(), args 'src' and arg 'dst' have different no. channels\n");
		return false;
	}

	if(dst.DataType()!=src.DataType()){
		fprintf(pErrorFile,"LogPolarMap_ZPlusA::Process(), args 'src' and arg 'dst' different datatypes\n");
		return false;
	}

		// if the img arg sizes don't match those stored
		// recalculate parameters and maps
	if( src.Width()!=m_hdr.srcW||src.Height()!=m_hdr.srcH 
		||dst.Width()!=m_hdr.dstW||dst.Height()!=m_hdr.dstH )
	{
		m_hdr.srcW=src.Width(); m_hdr.srcH=src.Height();
		m_hdr.dstW=dst.Width(); m_hdr.dstH=dst.Height();

		if(!AllocMaps())
		{
			fprintf(pErrorFile,"LogPolarMap_ZPlusA::Process(), memory allocation failure");
			Destroy();		
			return false;
		}
		CalcMaps();
	}

	if(!Magnify(dst,src)){
		fprintf(pErrorFile,"LogPolarMap_ZPlusA::Process(), error magnifying image\n");
		return false;
	}
	
	return true;
}

bool LogPolarMap_ZPlusA::ProcessButterfly(	AbstractImage &dst,const AbstractImage &src)
{
	if(!dst.IsValid()||!src.IsValid()){
		fprintf(pErrorFile,"LogPolarMap_ZPlusA::ProcessButterfly(), arg 'src' or arg 'dst' invalid\n");
		return false;
	}

	if(dst.Channels()!=src.Channels()){
		fprintf(pErrorFile,"LogPolarMap_ZPlusA::ProcessButterfly(), args 'src' and arg 'dst' have different no. channels\n");
		return false;
	}

	if(dst.DataType()!=src.DataType()){
		fprintf(pErrorFile,"LogPolarMap_ZPlusA::ProcessButterfly(), args 'src' and arg 'dst' different datatypes\n");
		return false;
	}

		// if the img arg sizes don't match those stored
		// recalculate parameters and maps
	if( src.Width()!=m_hdr.srcW||src.Height()!=m_hdr.srcH 
		||dst.Width()!=m_hdr.dstW||dst.Height()!=m_hdr.dstH )
	{
		m_hdr.srcW=src.Width(); m_hdr.srcH=src.Height();
		m_hdr.dstW=dst.Width(); m_hdr.dstH=dst.Height();

		if(!AllocMaps())
		{
			fprintf(pErrorFile,"LogPolarMap_ZPlusA::ProcessButterfly(), memory allocation failure");
			Destroy();		
			return false;
		}
		CalcMaps();
	}

	if(!MagnifyButterfly(dst,src)){
		fprintf(pErrorFile,"LogPolarMap_ZPlusA::ProcessButterfly(), error magnifying image\n");
		return false;
	}
	
	return true;
}










////////////////////////////////////////////////////////////////////////////
// CONIC MAGNIFICATION_____________________________________________________________
////////////////////////////////////////////////////////////////////////////
//
//
//	aja 2005


ConicMap2D::ConicMap2D()
{
	m_hdr.apexAng=0.11;
	m_hdr.srcW=m_hdr.srcH=m_hdr.dstW=m_hdr.dstH=0;
	m_hdr.srcMaxYEcc=m_hdr.dstMaxEcc=0;

	m_pSpatialMap=NULL;
	m_pScaleMap=NULL;
	m_pMagnifMap=NULL;
}

ConicMap2D::ConicMap2D(float apexAngle_rad)
{
	m_hdr.apexAng=apexAngle_rad;
	m_hdr.srcW=m_hdr.srcH=m_hdr.dstW=m_hdr.dstH=0;
	m_hdr.srcMaxYEcc=m_hdr.dstMaxEcc=0;

	m_pSpatialMap=NULL;
	m_pScaleMap=NULL;
	m_pMagnifMap=NULL;
}

ConicMap2D::ConicMap2D(	float apexAngle_rad,
						unsigned int dstWidth, unsigned int dstHeight,
						float dstMaxEccentricity_rad,
						unsigned int srcWidth, unsigned int srcHeight,
						float srcMaxYEccentricity_rad
						)
{
	if(!Create(apexAngle_rad,dstWidth,dstHeight,dstMaxEccentricity_rad,
		srcWidth,srcHeight,srcMaxYEccentricity_rad)){
		fprintf(pErrorFile,"ConicMap2D::ConicMap2D(), Error creating conic map\n");
	}
}


ConicMap2D::ConicMap2D(const ConicMap2D &src)
{
	if(!Copy(src)){
		fprintf(pErrorFile,"ConicMap2D::ConicMap2D(), error copying\n");
	}
}

ConicMap2D &ConicMap2D::operator=(const ConicMap2D &src)
{
	if (this!=&src) 
	{
		if(!Copy(src)){
			fprintf(pErrorFile,"ConicMap2D::operator=(), error copying map\n");	
		}
	}
	return *this;
}

ConicMap2D::~ConicMap2D()
{
	if(!Destroy()){
		fprintf(pErrorFile,"ConicMap2D::~ConicMap2D, error destroying object\n");	
	}	
}

bool ConicMap2D::Create(float apexAngle_rad,
						unsigned int dstWidth, unsigned int dstHeight,
						float dstMaxEccentricity_rad,
						unsigned int srcWidth, unsigned int srcHeight,
						float srcMaxYEccentricity_rad
						)
{
	if(srcMaxYEccentricity_rad>UCL_VRL_HALF_PI-0.000001)
	{
		fprintf(pErrorFile, "ConicMap2D::ConicMap2D() input im ecc arg (%f)>=pi/2\n",
				srcMaxYEccentricity_rad );
		Destroy();
		return false;
	}
	
	ConicMap2DHdr tempHdr;

	tempHdr.apexAng=apexAngle_rad;
	tempHdr.srcW=srcWidth;
	tempHdr.srcW=srcWidth;
	tempHdr.srcH=srcHeight;
	tempHdr.dstW=dstWidth;
	tempHdr.dstH=dstHeight;
	tempHdr.srcMaxYEcc=srcMaxYEccentricity_rad;
	tempHdr.dstMaxEcc=dstMaxEccentricity_rad;

	if(memcmp(&tempHdr,&m_hdr,sizeof(ConicMap2DHdr))==0)
	{
		CalcMaps();
		return true;
	}
	else
	{
		Destroy();
		memcpy(&m_hdr,&tempHdr,sizeof(ConicMap2DHdr));
	}
	
	if(!AllocMaps())
	{
		fprintf(pErrorFile,"ConicMap2D::Create() memory allocation failure\n");
		Destroy();
		
		return false;
	}

	CalcMaps();

	return true;
}

bool ConicMap2D::Copy(const ConicMap2D &src)
{
	if(this==&src)
	{
		fprintf(pErrorFile,"ConicMap2D::Copy(), arg 'src' == this\n");
		return false;
	}

	unsigned int sz=src.m_hdr.dstW*src.m_hdr.dstH;

	if(memcmp(&m_hdr,&src.m_hdr,sizeof(ConicMap2DHdr))==0)
	{
		if(m_pSpatialMap&&m_pScaleMap&&m_pMagnifMap)
		{
			memcpy(m_pSpatialMap,src.m_pSpatialMap,sz*sizeof(Coordinate));
			memcpy(m_pScaleMap,src.m_pScaleMap,sz*sizeof(float));
			memcpy(m_pMagnifMap,src.m_pMagnifMap,sz*sizeof(float));
		return true;
		}	
	}

	memcpy(&m_hdr,&src.m_hdr,sizeof(ConicMap2DHdr));

	if(!AllocMaps())
	{
		fprintf(pErrorFile,"ConicMap2D::Copy() memory allocation failure");
		Destroy();
		return false;
	}
	
	memcpy(m_pSpatialMap,src.m_pSpatialMap,sz*sizeof(Coordinate));
	memcpy(m_pScaleMap,src.m_pScaleMap,sz*sizeof(float));
	memcpy(m_pMagnifMap,src.m_pMagnifMap,sz*sizeof(float));

	return true;
}

bool ConicMap2D::Destroy()
{
	if(m_pSpatialMap){
		delete [] m_pSpatialMap;
		m_pSpatialMap=NULL;
	}

	if(m_pScaleMap){
		delete [] m_pScaleMap;
		m_pScaleMap=NULL;
	}

	if(m_pMagnifMap){
		delete [] m_pMagnifMap;
		m_pMagnifMap=NULL;
	}

	memset(&m_hdr,0,sizeof(ConicMap2DHdr));
	m_hdr.apexAng=0.11;

	return true;
}

bool ConicMap2D::AllocMaps()
{
	if( m_pSpatialMap )
		delete [] m_pSpatialMap;

	if( m_pScaleMap )
		delete [] m_pScaleMap;

	if( m_pMagnifMap )
		delete [] m_pMagnifMap;
	
	try{
		m_pSpatialMap = new Coordinate[m_hdr.dstW*m_hdr.dstH];
		m_pScaleMap = new float[m_hdr.dstW*m_hdr.dstH];
		m_pMagnifMap = new float[m_hdr.dstW*m_hdr.dstH];
	}
	catch( bad_alloc xa )
	{
		fprintf(pErrorFile,"ConicMap2D::AllocMaps(), Error allocating memory");
		return false;
	}
	return true;
}

bool ConicMap2D::ToFArrays(float *x,float *y,unsigned int sz) const
{
	if(!x||!y){
		fprintf(pErrorFile,"ConicMap2D::ToFArrays(), arg x and/or y ==NULL");
		return false;
	}

	if(sz!=m_hdr.dstW*m_hdr.dstH){
		fprintf(pErrorFile,
	"ConicMap2D::ToFArrays(), arg 'sz' (%d) doesn't match map size (%d)",
				sz,m_hdr.dstW*m_hdr.dstH);
		return false;
	}

	for(int i=0;i<sz;++i)
	{
		x[i]=m_pSpatialMap[i].x;
		y[i]=m_pSpatialMap[i].y;
	}

	return true;
}

	//NOTE: WE TAKE CONE AXIS LENGTH=1 FOR CALCULATIONS (DOESN'T MATTER)

	//returns dist from apex given eccentricity
float ConicMap2D::ConicEqu_Dist(float ecc) const
{
	return sin(ecc)/sin(ecc+m_hdr.apexAng);
}
	//returns eccentricity given dist from apex
float ConicMap2D::ConicEqu_Ecc(float s) const
{
	double side_a = sqrt( (s*s) + 1 - (2.0*s*cos(m_hdr.apexAng) ) );
	
	return acos( - ( ( (s*s)-1-(side_a*side_a) ) / (2.0*side_a) ) );
}
	
void ConicMap2D::CalcMaps()
{
	unsigned int index;
	float srcMidX, srcMidY, dstMidX, dstMidY, hyp, dstX, dstY, inx, iny;
	float srcHalfDiag, dstHalfDiag, s, max_s, scale, ecc, picPlanePolarAng;


	dstMidX = m_hdr.dstW/2.0;  dstMidY = m_hdr.dstH/2.0; srcMidX=m_hdr.srcW/2.0; srcMidY=m_hdr.srcH/2.0;
	srcHalfDiag =  sqrt((srcMidX*srcMidX)+(srcMidY*srcMidY));
	dstHalfDiag = sqrt((dstMidX*dstMidX)+(dstMidY*dstMidY));
		
		// s refers to dist from a point on cone to apex
	max_s = ConicEqu_Dist(m_hdr.dstMaxEcc);

	float srcDistLensToRetina=(m_hdr.srcH/2.0)/tan(m_hdr.srcMaxYEcc);

		// scale factor relating distance on dst image to distance on cone surface
		// scale = max_s/dstHalfDiag;
	if(dstMidY<=dstMidX)
		scale = max_s/dstMidY;
	else
		scale = max_s/dstMidX;

	for (int x=0;x<m_hdr.dstW;x++){
		for (int y=0;y<m_hdr.dstH;y++){
				// index for point (x,y) in the array
			index = (y*m_hdr.dstW)+x;
				//get xy coordinates identifying position relative to centre of image
			dstX = (float) x-dstMidX;
			dstY = (float) y-dstMidY;

				//find angle of point relative to the xaxis of the picture plane
			picPlanePolarAng =  formatnegpi_pi_f( angle_rad_f( dstX, dstY ) );

				//find distance of point from centre (representing linearly scaled
				//distance along cone surface)
				//and calculate eccentricity on original image via conic equation
			s = scale * sqrt((dstX*dstX)+(dstY*dstY));
			ecc = ConicEqu_Ecc( s );
		
			if( ecc < UCL_VRL_HALF_PI )
			{
					// find the distance from the origin of the original image 
					// corresponding to eccentricity
				hyp = srcDistLensToRetina*tan(ecc);
			
				inx = hyp*cos(picPlanePolarAng);
				iny = hyp*sin(picPlanePolarAng);
				
					// ensure that the point is within the original image
				if((int)floor(inx+srcMidX+0.5) < m_hdr.srcW && (int) floor( iny+srcMidY + 0.5 ) < m_hdr.srcH )
				{
					m_pSpatialMap[index].x=inx+srcMidX;
					m_pSpatialMap[index].y=iny+srcMidY;
					m_pScaleMap[index]=(s/max_s)/(hyp/srcHalfDiag);
					m_pMagnifMap[index]=sin(m_hdr.apexAng)/pow(sin(ecc+m_hdr.apexAng),2);
				}
				else
				{
					m_pSpatialMap[index].x=m_pSpatialMap[index].y=SPACE_VAR_MAP_OUT_OF_RANGE;
					m_pScaleMap[index]=0;
					m_pMagnifMap[index]=0;

				}
			}
			else // if ecc > HALF_PI
			{
				m_pSpatialMap[index].x=m_pSpatialMap[index].y=SPACE_VAR_MAP_OUT_OF_RANGE;
				m_pScaleMap[index]=0;
				m_pMagnifMap[index]=0;
			}
		}
	}
	
	//deal with singularity at the fovea
	index=(dstMidY*m_hdr.dstW)+dstMidX;
	
	m_pSpatialMap[index].x=srcMidX; 
	m_pSpatialMap[index].y=srcMidY;
	m_pScaleMap[index]=0;
	m_pMagnifMap[index]=0;
}


bool ConicMap2D::Magnify(AbstractImage &dst,const AbstractImage &src) const
{
	unsigned int x,y;

	switch(dst.DataType())
	{
	case IMG_UCHAR:	{
						unsigned char *pSrcData, *pDstPix;
						pSrcData=(unsigned char*)src.Data();
						pDstPix=(unsigned char*)dst.Data();
						Coordinate *pMapPix=m_pSpatialMap;

						for (y=0;y<m_hdr.dstH;++y){
							for(x=0;x<m_hdr.dstW;++x){
								if(pMapPix->x!=SPACE_VAR_MAP_OUT_OF_RANGE)
								{
									BilinearInterpolatePixel(	pDstPix, //(src.Channels()*((y*m_hdr.dstW)+x)),
																pSrcData,
																src.Channels(),src.Width(),
																src.Height(),
																pMapPix->x,pMapPix->y);
								}
								else
									memset(pDstPix,//+(y*m_hdr.dstW)+x,
											0,dst.Channels()*sizeof(unsigned char));

								pMapPix++;
								pDstPix+=src.Channels();
							}
						}
					} break;
	case IMG_LONG:	{
						long *pSrcData, *pDstPix;
						pSrcData=(long*)src.Data();
						pDstPix=(long*)dst.Data();
						Coordinate *pMapPix=m_pSpatialMap;

						for (y=0;y<m_hdr.dstH;++y){
							for(x=0;x<m_hdr.dstW;++x){
								if(pMapPix->x!=SPACE_VAR_MAP_OUT_OF_RANGE)
								{
									BilinearInterpolatePixel(	pDstPix, //(src.Channels()*((y*m_hdr.dstW)+x)),
																pSrcData,
																src.Channels(),src.Width(),
																src.Height(),
																pMapPix->x,pMapPix->y);
								}
								else
									memset(pDstPix,0,dst.Channels()*sizeof(long));

								pMapPix++;
								pDstPix+=src.Channels();
							}
						}

					} break;
	case IMG_FLOAT: {
						float *pSrcData, *pDstPix;
						pSrcData=(float*)src.Data();
						pDstPix=(float*)dst.Data();
						Coordinate *pMapPix=m_pSpatialMap;

						for (y=0;y<m_hdr.dstH;++y){
							for(x=0;x<m_hdr.dstW;++x){
								if(pMapPix->x!=SPACE_VAR_MAP_OUT_OF_RANGE)
								{
									BilinearInterpolatePixel(	pDstPix, //(src.Channels()*((y*m_hdr.dstW)+x)),
																pSrcData,
																src.Channels(),src.Width(),
																src.Height(),
																pMapPix->x,pMapPix->y);
								}
								else
									memset(pDstPix,0,dst.Channels()*sizeof(float));

								pMapPix++;
								pDstPix+=src.Channels();
							}
						}
					} break;
	default:		{
						fprintf(pErrorFile,"ConicMap2D::Magnify(), unrecognised datatype\n");
						return false;
					}
	}

	return true;
}

	
bool ConicMap2D::Process(	AbstractImage &dst,float dstMaxEccentricity_rad,
							const AbstractImage &src,float srcMaxYEccentricity_rad)
{
	if(!dst.IsValid()||!src.IsValid()){
		fprintf(pErrorFile,"ConicMap2D::Process(), arg 'src' or arg 'dst' invalid\n");
		return false;
	}

	if(dst.Channels()!=src.Channels()){
		fprintf(pErrorFile,"ConicMap2D::Process(), args 'src' and arg 'dst' have different no. channels\n");
		return false;
	}

	if(dst.DataType()!=src.DataType()){
		fprintf(pErrorFile,"ConicMap2D::Process(), args 'src' and arg 'dst' different datatypes\n");
		return false;
	}

	if( srcMaxYEccentricity_rad > UCL_VRL_HALF_PI - 0.000001 )
	{
		fprintf(pErrorFile,"ConicMap2D::Process() input im ecc arg (%f) >= pi/2\n"); fflush(stdout);
		return false;
	}
		// if the img arg sizes don't match those stored
		// recalculate parameters and maps
	if( src.Width()!=m_hdr.srcW||src.Height()!=m_hdr.srcH 
		||dst.Width()!=m_hdr.dstW||dst.Height()!=m_hdr.dstH
		||srcMaxYEccentricity_rad!=m_hdr.srcMaxYEcc
		||dstMaxEccentricity_rad!=m_hdr.dstMaxEcc )
	{
		m_hdr.srcW=src.Width(); m_hdr.srcH=src.Height();
		m_hdr.dstW=dst.Width(); m_hdr.dstH=dst.Height();
		m_hdr.srcMaxYEcc=srcMaxYEccentricity_rad;
		m_hdr.dstMaxEcc=dstMaxEccentricity_rad;

		if(!AllocMaps())
		{
			fprintf(pErrorFile,"ConicMap2D::Process(), memory allocation failure");
			Destroy();		
			return false;
		}
		CalcMaps();
	}

	if(!Magnify(dst,src)){
		fprintf(pErrorFile,"ConicMap2D::Process(), error magnifying image\n");
		return false;
	}
	
	return true;
}



////////////////////////////////////////////////////////////////////////////
// VECTOR FIELD_____________________________________________________________
////////////////////////////////////////////////////////////////////////////
//
//
// aja 2005
//  Declarations for VectField2D class, a data structure containing a 2D vector
//  field (represented in polar/cartesian coordinates). 
//  IO as PCM image files (similar format to pgm file) supported

VectField2D::VectField2D()
{
	memset(&hdr,0,sizeof(VectField2DHdr));
	isPolar=false;
	field=NULL;
}

VectField2D::VectField2D(int w, int h) {
	field=NULL;
	Init(w,h);
}

VectField2D::VectField2D(const VectField2D &c)
{
	field=NULL;
	//fputs("VectField2D::Copy Constructor",pErrorFile);
	Copy(c);
}

VectField2D &VectField2D::operator=(const VectField2D &c)
{
	if (this!=&c) 
	{
		Copy(c);
	}
  return *this;
}

VectField2D::~VectField2D() {
  
	Destroy();
}

void VectField2D::Destroy() {
  
	if(field)
		delete[] field;

	field=NULL;
	memset(&hdr,0,sizeof(VectField2DHdr));
}


bool VectField2D::Init(unsigned int w,unsigned int h)
{
	hdr.w=w; hdr.h=h; hdr.sz=w*h; hdr.max=0.0;
	isPolar=false;

	if(field)
	{
		delete [] field;
	}

	try{
		field=new Vect2D[hdr.sz];
		memset(field,0,hdr.sz*sizeof(Vect2D));
	}

	catch( bad_alloc xa )
	{
		fprintf(pErrorFile,"VectField2D::VectField2D() error allocating memory"); 
		field=NULL;
		memset(&hdr,0,sizeof(VectField2DHdr));
		return false;
	}
	return true;
}

bool VectField2D::SameType(const VectField2D &c)
{
	if(memcmp(&hdr,&c.hdr,sizeof(VectField2D))!=0)
		return false;

	if(((field&&c.field)||(!field&&!c.field)))
		return false;

	return true;
}


bool VectField2D::Copy(const VectField2D &c)
{
	if(this==&c) 
		return false;
	
	if(!c.field)
	{
		puts("VectField2D::Copy() field arg=NULL"); fflush(stdout);
		return false;
	}
	else
	{
		if(!SameType(c))
		{
			Destroy();
			if(!Init(c.hdr.w,c.hdr.h))
				return false;
		}
		memcpy(field,c.field,c.hdr.w*c.hdr.h*sizeof(Vect2D));
		isPolar=c.isPolar;
	}
	return true;
}

void VectField2D::CalcMax() {
	unsigned long p;
	float i,r,m;
	Vect2D *_i;
	hdr.max=m=0.0;

	if(!isPolar)
	{
		for(p=0;p<hdr.sz;p++) {
			r=field[p].x1; i=field[p].x2;
			m=r*r+i*i;
			if (m>hdr.max) hdr.max=m;
		}
		hdr.max=(float)sqrt(hdr.max);
	}
	else
	{
		for(_i=field;_i<field+hdr.sz;++_i)
		{
			if(_i->x2>hdr.max)
				hdr.max=_i->x2;
		}
	}
}

bool VectField2D::ReadBin(const char *fnm){
	
	FILE *_f;

	_f=fopen(fnm,"rb");

	if ( !_f )	
	{
		fprintf(pErrorFile,"VectField2D::ReadBin() error opening %s\n", fnm );
		fflush(pErrorFile);
		return false;
	}

	bool returnval=ReadBin(_f);

	fclose(_f);

	return returnval;     
}

bool VectField2D::ReadBin(FILE *_f){
	
	VectField2DHdr temphdr;

	if(fread(&temphdr,sizeof(VectField2DHdr),1,_f)!=1)
	{
		fprintf(pErrorFile,"VectField2D::ReadBin() Error reading header\n");
		return false;
	}

	if(memcmp(&hdr,&temphdr,sizeof(VectField2DHdr))!=0||!field)
	{
		Destroy();
		if(!Init(temphdr.w,temphdr.h))
			return false;
	}

	if(fread(field,sizeof(Vect2D),hdr.sz,_f)!=hdr.sz)
	{
		fprintf(pErrorFile,"VectField2D::ReadBin() Error reading field\n");
		return false;
	}
	
	return true;
}

bool VectField2D::WriteBin(const char *fnm) {
	FILE *_f;
  
	
	_f=fopen(fnm,"wb");

	if(_f)
	{
		fprintf(pErrorFile,"VectField2D::Write(), Error opening file: %s\n", fnm);
		return false;
	}
  
	fclose(_f);

	return true;
}

bool VectField2D::WriteBin(FILE *_f)
{  
	fwrite(&hdr,sizeof(hdr),1,_f);
	
	bool returnval=true;

	if(field)
		fwrite(field,sizeof(Vect2D),hdr.sz,_f);
	else
		return false;
	
	return returnval;
}

bool VectField2D::ReadPCM(const char *fnm){
	
	FILE *_f;

	_f=fopen(fnm,"r");

	if ( !_f )	
	{
		fprintf(pErrorFile,"Pcm::VectField2D() error opening %s\n", fnm );
		fflush(pErrorFile);
		return false;
	}

	bool returnval=ReadPCM(_f);

	fclose(_f);

	return returnval;     
}

bool VectField2D::ReadPCM(FILE *_f){
	
	unsigned long p;

	if(!(fscanf(_f,"PC\n%d %d\n%f",&hdr.w,&hdr.h,&hdr.max)>2))
	{
		fprintf(pErrorFile,"VectField2D::ReadPCM() Error reading header\n");
		return false;
	}
  
	while(getc(_f)!='\n');

	p=(long)hdr.w*hdr.h;
	
	if (p!=hdr.sz&&field) {
		delete[] field; 
		field=NULL;
	}

	if (!field)
	{
		try{
				hdr.sz=p;
				field=new Vect2D[hdr.sz];
		}
		catch(bad_alloc xa)
		{
			fprintf(pErrorFile,"VectField2D::ReadPCM(), Error allocating memory");
			return false;
		}
	}

	fread(field,sizeof(Vect2D),hdr.w*hdr.h,_f);
	isPolar=false;
	
	return true;
}

bool VectField2D::WritePCM(const char *fnm) {
	FILE *_f;
  
	
	_f=fopen(fnm,"w+");

	if(_f)
	{
		fprintf(pErrorFile,"VectField2D::Write(), Error opening file: %s\n", fnm);
		return false;
	}
  
	fclose(_f);

	return true;
}

bool VectField2D::WritePCM(FILE *_f)
{  
	if(!isPolar)
	{
		if(field)
		{
			CalcMax();
			fwrite(field,hdr.w*sizeof(Vect2D),hdr.h,_f);
		}
		else
			return false;
	}
	else
	{
		VectField2D temp(hdr.w,hdr.h);
		ToCartesian(temp);
		
		if(temp.field)
		{
			temp.CalcMax();
			fprintf(_f,"PC\n%d %d\n%f\n",hdr.w,hdr.h,hdr.max);
			fwrite(temp.field,hdr.w*sizeof(Vect2D),hdr.h,_f);
		}
		else
			return false;
	}
	return true;
}

bool VectField2D::ToPolar(VectField2D &polar)
{
	if(!polar.field)
	{
		fprintf(pErrorFile,"VectField2D::ToPolar() arg field==NULL\n");
		return false;
	}

	if(polar.hdr.w!=hdr.w||polar.hdr.h!=hdr.h)
	{
		fprintf(pErrorFile,"VectField2D::ToPolar(), arg w||h doesn't match this field\n");
		return false;
	}

	float x,y;

	Vect2D *_c,*_p;
	
	if(!isPolar)
	{
		for(_c=field,
			_p=polar.field;
			_c<field+hdr.sz;
			++_c,++_p)
		{
			x=_c->x1;
			y=_c->x2;
			
			_p->x1=angle_rad_f(x,y);
			_p->x2=sqrt((x*x)+(y*y));
		}
	}
	else
	{
		memcpy(polar.field,field,hdr.sz*sizeof(Vect2D));
	}
	
	polar.isPolar=true;
	polar.CalcMax();

	return true;
}

bool VectField2D::ThisToPolar()
{
	if(!field)
	{
		fprintf(pErrorFile,"VectField2D::ToPolar() arg field==NULL\n");
		return false;
	}

	float x,y;

	Vect2D *_p;
	
	if(!isPolar)
	{
		for(_p=field;
			_p<field+hdr.sz;
			++_p)
		{
			x=_p->x1;
			y=_p->x2;
			
			_p->x1=angle_rad_f(x,y);
			_p->x2=sqrt((x*x)+(y*y));
		}
		isPolar=true;
		CalcMax();
	}
	return true;
}

bool VectField2D::ToPolar(float *polarcoords,unsigned int w,unsigned int h)
{
	if(!polarcoords)
	{
		fprintf(pErrorFile,"VectField2D::ToPolar() polarcoords arg==NULL\n");
		return false;
	}

	if(w!=hdr.w||h!=hdr.h)
	{
		fprintf(pErrorFile,"VectField2D::ToPolar(), w||h arg doesn't match this field\n");
		return false;
	}

	if(!isPolar)
	{
		float x,y;

		Vect2D *_c;
		float *_p;
	
		for(_c=field,
			_p=polarcoords;
			_c<field+hdr.sz;
			++_c,_p+=2)
		{
			x=_c->x1;
			y=_c->x2;
			
			*_p=angle_rad_f(x,y);
			*(_p+1)=sqrt((x*x)+(y*y));
		}
	}
	else
	{
		memcpy(polarcoords,field,hdr.sz*sizeof(Vect2D));
	}
	return true;
}

bool VectField2D::ToPolar(float *angs,float *dists,unsigned int w, unsigned int h)
{
	if(!angs||!dists)
	{
		fprintf(pErrorFile,"VectField2D::ToPolar() ang||dist arg==NULL\n");
		return false;
	}

	if(w!=hdr.w||h!=hdr.h)
	{
		fprintf(pErrorFile,"VectField2D::ToPolar(), w||h arg doesn't match this field\n");
		return false;
	}

	if(!isPolar)
	{
		float x,y;
		for(unsigned int i=0;i<hdr.sz;++i)
		{
			x=field[i].x1;
			y=field[i].x2;
			
			angs[i]=angle_rad_f(x,y);
			dists[i]=sqrt((x*x)+(y*y));
		}
	}
	else
	{
		for(unsigned int i=0;i<hdr.sz;++i)
		{
			angs[i]=field[i].x1;
			dists[i]=field[i].x2;
		}
	}
	return true;
}

bool VectField2D::ToCartesian(VectField2D &cartes)
{
	if(!cartes.field)
	{
		fprintf(pErrorFile,"VectField2D::ToCartesian() arg field==NULL\n");
		return false;
	}

	if(cartes.hdr.w!=hdr.w||cartes.hdr.h!=hdr.h)
	{
		fprintf(pErrorFile,"VectField2D::ToCartesian(), arg w||h doesn't match this field\n");
		return false;
	}

	Vect2D *_p,*_c;
	
	if(isPolar)
	{
		for(_p=field,
			_c=cartes.field;
			_p<field+hdr.sz;
			++_p,++_c)
		{
			_c->x1=_p->x2*cos(_p->x1);
			_c->x2=_p->x2*sin(_p->x1);
		}
	}
	else
	{
		memcpy(cartes.field,field,hdr.sz*sizeof(Vect2D));
	}
	
	cartes.isPolar=false;
	cartes.CalcMax();

	return true;
}

bool VectField2D::ThisToCartesian()
{
	if(!field)
	{
		fprintf(pErrorFile,"VectField2D::ThisToCartesian() arg field==NULL\n");
		return false;
	}

	float ang,dist;

	Vect2D *_c;
	
	if(isPolar)
	{
		for(_c=field;
			_c<field+hdr.sz;
			++_c)
		{
			ang=_c->x1;
			dist=_c->x2;
			_c->x1=dist*cos(ang);
			_c->x2=dist*sin(ang);
		}
		isPolar=false;
		CalcMax();
	}
	return true;
}

bool VectField2D::ToCartesian(float *cartescoords,unsigned int w,unsigned int h)
{
	if(!cartescoords)
	{
		fprintf(pErrorFile,"VectField2D::ToCartesian() cartescoords arg==NULL\n");
		return false;
	}

	if(w!=hdr.w||h!=hdr.h)
	{
		fprintf(pErrorFile,"VectField2D::ToCartesian(), w||h arg doesn't match this field\n");
		return false;
	}

	float ang=0,dist=0;

	if(isPolar)
	{
		Vect2D *_p;
		float *_c;
	
		for(_p=field,
			_c=cartescoords;
			_p<field+hdr.sz;
			++_p,_c+=2)
		{
			ang=_p->x1;
			dist=_p->x2;
			
			*_c=dist*cos(ang);
			*(_c+1)=dist*sin(ang);
		}
	}
	else
	{
		memcpy(cartescoords,field,hdr.sz*sizeof(Vect2D));
	}
	return true;
}

bool VectField2D::ToCartesian(float *x,float *y,unsigned int w, unsigned int h)
{
	if(!x||!y)
	{
		fprintf(pErrorFile,"VectField2D::ToCartesian() x||y arg==NULL\n");
		return false;
	}

	if(w!=hdr.w||h!=hdr.h)
	{
		fprintf(pErrorFile,"VectField2D::ToCartesian(), w||h arg doesn't match this field\n");
		return false;
	}

	if(isPolar)
	{
		for(unsigned int i=0;i<hdr.sz;++i)
		{
			x[i]=field[i].x2*cos(field[i].x1);
			y[i]=field[i].x2*sin(field[i].x1);
		}
	}
	else
	{
		for(unsigned int i=0;i<hdr.sz;++i)
		{
			x[i]=field[i].x1;
			y[i]=field[i].x2;
		}
	}
	return true;
}

bool VectField2D::PolarError(VectField2D &polarerror,const VectField2D &target)
{
	if(!target.field||target.hdr.w!=hdr.w||target.hdr.h!=hdr.h)
	{
		fprintf(pErrorFile,
"VectField2D::PolarError(), target uninitialised or incorrect dimensions:(%d*%d), expecting (%d*%d)\n",
		target.hdr.w,target.hdr.h,hdr.w,hdr.h);
		return false;
	}

	if(!field)
	{
		fprintf(pErrorFile,"VectField2D::PolarError(), this field uninitialised\n");
		return false;
	}
	
	if(!polarerror.field||polarerror.hdr.w!=hdr.w||polarerror.hdr.h!=hdr.h)
	{
		polarerror.Init(hdr.w,hdr.h);
		polarerror.isPolar=true;
	}

	VectField2D targetpolar,actualpolar;
	Vect2D *targ=NULL,*actual=NULL;

	if(isPolar)
	{
		actual=field;
	}
	else
	{
		actualpolar.Copy(*this);
		actualpolar.ThisToPolar();
		actual=actualpolar.field;
	}

	if(target.isPolar)
		targ=target.field;
	else
	{
		targetpolar.Copy(target);
		targetpolar.ThisToPolar();
		targ=targetpolar.field;
	}	

	for(int i=0;i<hdr.sz;++i)
	{
		polarerror.field[i].x1=fabs(formatnegpi_pi_f(targ[i].x1-actual[i].x1));
		polarerror.field[i].x2=fabs(targ[i].x2-actual[i].x2);
	}
	return true;
}

bool VectField2D::CartesianError(VectField2D &carteserror,const VectField2D &target)
{
	if(!target.field||target.hdr.w!=hdr.w||target.hdr.h!=hdr.h)
	{
		fprintf(pErrorFile,
"VectField2D::CartesianError(), target uninitialised or incorrect dimensions:(%d*%d), expecting (%d*%d)\n",
		target.hdr.w,target.hdr.h,hdr.w,hdr.h);
		return false;
	}

	if(!field)
	{
		fprintf(pErrorFile,"VectField2D::CartesianError(), this field uninitialised\n");
		return false;
	}
	
	if(!carteserror.field||carteserror.hdr.w!=hdr.w||carteserror.hdr.h!=hdr.h)
	{
		carteserror.Init(hdr.w,hdr.h);
		carteserror.isPolar=false;
	}

	VectField2D targetcartes,actualcartes;
	Vect2D *targ=NULL,*actual=NULL;

	if(!isPolar)
	{
		actual=field;
	}
	else
	{
		actualcartes.Copy(*this);
		actualcartes.ThisToCartesian();
		actual=actualcartes.field;
	}

	if(!target.isPolar)
		targ=target.field;
	else
	{
		targetcartes.Copy(target);
		targetcartes.ThisToCartesian();
		targ=targetcartes.field;
	}	

	for(int i=0;i<hdr.sz;++i)
	{
		carteserror.field[i].x1=fabs(targ[i].x1-actual[i].x1);
		carteserror.field[i].x2=fabs(targ[i].x2-actual[i].x2);
	}
	return true;
}

bool VectField2D::PolarDifference(VectField2D &polardiff,const VectField2D &target)
{
	if(!target.field||target.hdr.w!=hdr.w||target.hdr.h!=hdr.h)
	{
		fprintf(pErrorFile,
"VectField2D::PolarDifference(), target uninitialised or incorrect dimensions:(%d*%d), expecting (%d*%d)\n",
		target.hdr.w,target.hdr.h,hdr.w,hdr.h);
		return false;
	}

	if(!field)
	{
		fprintf(pErrorFile,"VectField2D::PolarDifference(), this field uninitialised\n");
		return false;
	}
	
	if(!polardiff.field||polardiff.hdr.w!=hdr.w||polardiff.hdr.h!=hdr.h)
	{
		polardiff.Init(hdr.w,hdr.h);
		polardiff.isPolar=true;
	}

	VectField2D targetpolar,actualpolar;
	Vect2D *targ=NULL,*actual=NULL;

	if(isPolar)
	{
		actual=field;
	}
	else
	{
		actualpolar.Copy(*this);
		actualpolar.ThisToPolar();
		actual=actualpolar.field;
	}

	if(target.isPolar)
		targ=target.field;
	else
	{
		targetpolar.Copy(target);
		targetpolar.ThisToPolar();
		targ=targetpolar.field;
	}	

	for(int i=0;i<hdr.sz;++i)
	{
		polardiff.field[i].x1=formatnegpi_pi_f(targ[i].x1-actual[i].x1);
		polardiff.field[i].x2=targ[i].x2-actual[i].x2;
	}
	return true;
}

bool VectField2D::CartesianDifference(VectField2D &cartesdiff,const VectField2D &target)
{
	if(!target.field||target.hdr.w!=hdr.w||target.hdr.h!=hdr.h)
	{
		fprintf(pErrorFile,
"VectField2D::CartesianDifference(), target uninitialised or incorrect dimensions:(%d*%d), expecting (%d*%d)\n",
		target.hdr.w,target.hdr.h,hdr.w,hdr.h);
		return false;
	}

	if(!field)
	{
		fprintf(pErrorFile,"VectField2D::CartesianDifference(), this field uninitialised\n");
		return false;
	}
	
	if(!cartesdiff.field||cartesdiff.hdr.w!=hdr.w||cartesdiff.hdr.h!=hdr.h)
	{
		cartesdiff.Init(hdr.w,hdr.h);
		cartesdiff.isPolar=false;
	}

	VectField2D targetcartes,actualcartes;
	Vect2D *targ=NULL,*actual=NULL;

	if(!isPolar)
	{
		actual=field;
	}
	else
	{
		actualcartes.Copy(*this);
		actualcartes.ThisToCartesian();
		actual=actualcartes.field;
	}

	if(!target.isPolar)
		targ=target.field;
	else
	{
		targetcartes.Copy(target);
		targetcartes.ThisToCartesian();
		targ=targetcartes.field;
	}	

	for(int i=0;i<hdr.sz;++i)
	{
		cartesdiff.field[i].x1=targ[i].x1-actual[i].x1;
		cartesdiff.field[i].x2=targ[i].x2-actual[i].x2;
	}
	return true;
}


bool VectField2D::FlipVert()
{
	Vect2D *newim,*old_i,*new_i;

	try{
		newim=new Vect2D[hdr.w*hdr.h];
	}
	catch(bad_alloc xa)
	{
		fprintf(pErrorFile,"Pgm5::FlipVert(), Error allocating memory\n");
		return false;
	}


	for(	new_i=newim,
			old_i=field+hdr.sz-1-hdr.w;
			new_i<newim+hdr.sz;
			new_i+=hdr.w,
			old_i-=hdr.w )
	{
		memcpy(new_i,old_i,hdr.w*sizeof(Vect2D));
	}
	
	delete [] field;
	field=newim;

	return true;
}



////////////////////////////////////////////////////////////////////////////
// ImgPixelStats_____________________________________________________________
////////////////////////////////////////////////////////////////////////////
//
//

ImgPixelStats::ImgPixelStats()
{
	memset(&hdr,0,sizeof(ImgPixelStatsHdr));

	imStats=NULL;
	imRunningTotals=NULL;
}

ImgPixelStats::ImgPixelStats(unsigned int w,unsigned int h,unsigned int cols){
	imStats=NULL;
	imRunningTotals=NULL;
	Init(w,h,cols);
}

ImgPixelStats::ImgPixelStats(const ImgPixelStats &c)
{
	imStats=NULL;
	imRunningTotals=NULL;
	//fputs("ImgPixelStats::Copy Constructor",pErrorFile);
	Copy(c);
}

ImgPixelStats &ImgPixelStats::operator=(const ImgPixelStats &c)
{
	if (this!=&c) 
	{
		Copy(c);
	}
  return *this;
}

ImgPixelStats::~ImgPixelStats() {
  
	Destroy();
}

void ImgPixelStats::Destroy(){
	
	if(imStats)
	{
		delete[] imStats;
		imStats=NULL;
	}

	if(imRunningTotals)
	{
		delete[] imRunningTotals;
		imRunningTotals=NULL;
	}
	memset(&hdr,0,sizeof(ImgPixelStatsHdr));
}


bool ImgPixelStats::Init(unsigned int w,unsigned int h,unsigned int channels)
{
	if(hdr.sz!=w*h*channels)
	{
		if(imStats)
		{
			delete [] imStats;
			imStats=NULL;
		}

		if(imRunningTotals)
		{
			delete [] imRunningTotals;
			imRunningTotals=NULL;
		}
	}
	hdr.w=w; hdr.h=h; hdr.channels=channels; hdr.sz=w*h*channels;
	
	if(!imStats)
	{
		try{
			imStats=new Stats[hdr.sz];
			memset(imStats,0,hdr.sz*sizeof(Stats));
			imRunningTotals=new RunningTotals[hdr.sz];
			memset(imRunningTotals,0,hdr.sz*sizeof(RunningTotals));
		}
		catch( bad_alloc xa )
		{
			fprintf(pErrorFile,"ImgPixelStats::ImgPixelStats() error allocating memory"); 
			Destroy();
			return false;
		}
	}
	return true;
}

bool ImgPixelStats::SameType(const ImgPixelStats &c)
{
	if(memcmp(&hdr,&c.hdr,sizeof(ImgPixelStatsHdr))!=0)
		return false;

	if(((imStats&&c.imStats)||(!imStats&&!c.imStats)))
		return false;

	if(!((imRunningTotals&&c.imRunningTotals)||(!imRunningTotals&&!c.imRunningTotals)))
		return false;

	return true;
}

bool ImgPixelStats::Copy(const ImgPixelStats &c)
{
	if(this==&c) 
		return false;
	
	if(!c.imStats)
	{
		puts("ImgPixelStats::Copy() c.imStats=NULL"); fflush(stdout);
		return false;
	}
	else if(!c.imRunningTotals)
	{
		puts("ImgPixelStats::Copy() c.RunningTotal=NULL"); fflush(stdout);
		return false;
	}
	else
	{
		if(!SameType(c))
		{
			Destroy();
			if(!Init(c.hdr.w,c.hdr.h,c.hdr.channels))
				return false;
		}

		memcpy(imStats,c.imStats,c.hdr.sz*sizeof(Stats));
		memcpy(imRunningTotals,c.imRunningTotals,c.hdr.sz*sizeof(RunningTotals));
	}
	return true;
}

bool ImgPixelStats::ReadBin(const char *fnm){
	
	FILE *_f;

	_f=fopen(fnm,"rb");

	if ( !_f )	
	{
		fprintf(pErrorFile,"ImgPixelStats::ImgPixelStats() error opening %s\n", fnm );
		fflush(pErrorFile);
		return false;
	}

	bool returnval=ReadBin(_f);

	fclose(_f);

	return returnval;     
}

bool ImgPixelStats::ReadBin(FILE *_f){
	
	ImgPixelStatsHdr temphdr;

	if(fread(&temphdr,sizeof(ImgPixelStatsHdr),1,_f)!=1)
	{
		fprintf(pErrorFile,"ImgPixelStats::ReadBin() Error reading header\n");
		return false;
	}

	if(memcmp(&hdr,&temphdr,sizeof(ImgPixelStatsHdr))!=0||!imStats||!imRunningTotals)
	{ 
		Destroy();
		if(!Init(temphdr.w,temphdr.h,temphdr.channels))
			return false;
	}

	if(fread(imStats,sizeof(Stats),hdr.sz,_f)!=hdr.sz)
	{
		fprintf(pErrorFile,"ImgPixelStats::ReadBin() Error reading imStats\n");
		return false;
	}
	if(fread(imRunningTotals,sizeof(RunningTotals),hdr.sz,_f)!=hdr.sz){
		fprintf(pErrorFile,"ImgPixelStats::ReadBin() Error reading imRunningTotals\n");
		return false;
	}
	
	return true;
}

bool ImgPixelStats::WriteBin(const char *fnm) {
	FILE *_f;
  
	
	_f=fopen(fnm,"wb");

	if(!_f)
	{
		fprintf(pErrorFile,"ImgPixelStats::WriteBin(), Error opening file: %s\n", fnm);
		return false;
	}
  
	if(!WriteBin(_f)){
		fprintf(pErrorFile,"ImgPixelStats::WriteBin(), Error writing file: %s\n",fnm);
		fclose(_f);
		return false;
	}
	fclose(_f);

	return true;
}

bool ImgPixelStats::WriteBin(FILE *_f)
{  
	fwrite(&hdr,sizeof(hdr),1,_f);
	
	bool returnval=true;

	if(imStats)
		fwrite(imStats,sizeof(Stats),hdr.sz,_f);
	else
		returnval=false;
	
	if(imRunningTotals)
		fwrite(imRunningTotals,sizeof(RunningTotals),hdr.sz,_f);
	else
		returnval=false;

	return returnval;
}

bool ImgPixelStats::Process(float *img,unsigned int w,unsigned int h,unsigned int channels)
{
	if(!img||w!=hdr.w||h!=hdr.h||channels!=hdr.channels)
	{
		fprintf(pErrorFile,
"ImgPixelStats::Process(), arg unallocated or incorrect dimensions(%d*%d*d), expecting(%d*%d*d)\n",
		w,h,channels,hdr.w,hdr.h,hdr.channels);
		return false;
	}
	
	hdr.n++;

	for(unsigned int i=0;i<hdr.sz;++i)
	{
		imRunningTotals[i].sum+=img[i];
		imRunningTotals[i].sumofsquares+=img[i]*img[i];
	}
	return true;
}

bool ImgPixelStats::ToFArrays(float *mean,float *stdev,unsigned int sz) const
{
	if(hdr.sz!=sz){
		fprintf(pErrorFile,"ImgPixelStats::ToFArrays(), arg sz (%d) incompatible, expecting %d\n",
							sz,hdr.sz);
		return false;
	}
	for(unsigned int i=0;i<hdr.sz;++i)
	{
		mean[i]=imStats[i].mean;
		stdev[i]=imStats[i].stdev;
	}
	return true;
}

bool ImgPixelStats::CalcStats()
{
	if(!imRunningTotals)
	{
		fprintf(pErrorFile,"ImgPixelStats::GetStats(), No running totals to calculate stats upon\n");
		return false;
	}

	if(hdr.n<2)
	{
		for(unsigned int i=0;i<hdr.sz;++i)
		{
			imStats[i].mean=imRunningTotals[i].sum/hdr.n;
			imStats[i].stdev=0;
		}
		return true;
	}
	
	float mn,sd;

	for(unsigned int i=0;i<hdr.sz;++i)
	{
		mn=imRunningTotals[i].sum/hdr.n;
		sd=(imRunningTotals[i].sumofsquares/hdr.n)-(mn*mn);
	
			// Get unbiased estimate
		sd*=((float)hdr.n/(hdr.n-1));
		sd=(float)sqrt(sd);

		imStats[i].mean=mn;
		imStats[i].stdev=sd;
	}
	return true;
}





















