
// AJAnderson 5/2005

////////////////////////////////////////////////////////////////////////////
// IMPORTANT NOTE___________________________________________________
// Code uses Run Time Type Identification, which is not set on as default in
// Visual c++ v6 compiler.  If not set get error messages like:
//							"/GR-; unpredictable behavior may result"
// To set,	Visual c++6, menu: Project->Settings->C++->Category(C++ Language)
//			Visual C++ 2008, menu: Project->Properties/Configuration Properties/C/C++/ ...
//			Language/  and 'Enable run time type info'
//
// Loading bitmaps currently relies on OPENGL glaux/(glut?), These may need to be manually installed
// e.g. glut manually installed in "C:\glut";
// In Visual C++ 2008, this can be added to the search path via:
// Project->Properties/Configuration Properties/C/C++/General  and 'Additional directories'
// Project->Properties/Configuration Properties/Linker/General and 'Additional libraries'
// 
// To run IPL version the IPL library will need to be manually installed and added to the search path:
// In Visual C++ 2008, IPL can be added by:
// Project->Properties/Configuration Properties/C/C++/General  and 'Additional directories'
// add: "C:\Program Files\Intel\include";
// Project->Properties/Configuration Properties/Linker/General and 'Additional libraries'
// add: "C:\Program Files\Intel\lib\msvc";
//
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// TO-DO/BUGS___________________________________________________
// 1. ipl lib and our custom lib use diff image conventions wrt rgb and bgr colour format
// also vertical axis reversed.  needs standardising.
// 2. bug with current _Img::Resize() function (also image shrinking not supported, could use box filter)
// CIplImg::Resize() needs writing.
// 3. AbstractImage::MultiplyS() takes a pointer as arg.  Don't know why I did that. Would be simpler
// to take float as default arg for all image data types.
// 4. Should be a switch installed in McGM2005 between new and old versions of the McGM.
// 5. Need to add a boundary to the speed output image (greyscale val of 1 pix motion)
// 6. Open GL libraries currently used in function _Img::ReadBMP(), Better to remove dependency on openGL
// ....might be able to copy cimg code
// 7. AbstractImage contains unnecessary  functions for McGm etc, reduce this

//__________________________________________________________________________
//						FILE SUMMARY
//
//	class AbstractFilter
//	class AbstractImage
//	class _Filter : AbstractFilter
//	class CIplFilter : AbstractFilter (requires Intel Pentium Libraries)
//	class _Img : AbstractImage
//	class CIplImg : AbstractImage (requires Intel Pentium Libraries)
//  class CSteerBasis, has AbstractFilters and AbstractImages
//	class TaylorReconstruct, takes CSteerBasis and AbstractImage as function args
//							 (alternatively we could set these as member variables)
//	class McGM2005, has CSteerBasis, AbstractImages and AbstractFilters (temporal filtering)
//	class ColourWheelLUT
//  class ConicMap2D
//	class LogPolarMap
//	class VectField2D
//	class ImagePixelStats


#ifndef _UCLVisionResearchLabLib_H_INCLUDED_2005_
#define _UCLVisionResearchLabLib_H_INCLUDED_2005_

#include <vector> 
#include <list>

	//__ INTEL PENTIUM LIBRARY ________________________________
	// COMMENT OUT THE #define LINE BELOW TO DISABLE IPL COMPATIBILITY
//#define _UCLVisionResearchLabLib_H_IPL_INCLUDED_

#ifdef _UCLVisionResearchLabLib_H_IPL_INCLUDED_
	//#pragma comment(lib, "C:/Program Files (x86)/Intel/lib/msvc/ipl.lib")
	#pragma comment(lib, "ipl.lib")
	//#pragma comment(lib, "ilib_32.lib")
	#include <IPL.h>
#endif

using namespace std;

////////////////////////////////////////////////////////////////////////////
// GLOBAL DATATYPE LABELS___________________________________________________
////////////////////////////////////////////////////////////////////////////

	//Data Type ID's, 
	//These are the image data-types that are supported by the vision library
	//NOTE: 0 reserved as NULL dataType to set uninitialised images)
const short int IMG_NULL_DATATYPE=0;
const short int IMG_UCHAR=1;
const short int IMG_LONG=2;
const short int IMG_FLOAT=3;

const int IMG_UCHAR_MIN=0;
const int IMG_UCHAR_MAX=255;

const int IMG_LONG_MIN=-2147483647;	//1-(2^31)
const int IMG_LONG_MAX=2147483647;	//(2^31)-1

	// Note maximum/min float values not implemented

	// The below variables specify the maximum and minimum numerator and denominator
	// used in floating point image division (functions Divide(), TestDivideF(), TestInvertF())
const float ABSTRACTIMAGE_MIN_DENOM=0.0001f;	
const float ABSTRACTIMAGE_MIN_NUMER=0.00001f;

const double UCL_VRL_PI=3.14159265358979323846;
const double UCL_VRL_TWO_PI=6.28318530717959;
const double UCL_VRL_HALF_PI=1.5707963267949;
const double UCL_VRL_THREE_OVER_TWO_PI=4.712388980384690;


////////////////////////////////////////////////////////////////////////////
// ERROR LOG OUTPUT_______________________________________________________
////////////////////////////////////////////////////////////////////////////
// All error messages are sent to the same (C-style) out file
// Use these functions to redirect error (default is stderr) 
extern void SetUCLVRLErrorFile(FILE *_f);
extern FILE *GetUCLVRLErrorFile();

////////////////////////////////////////////////////////////////////////////
// ABSTRACT FILTER CLASS____________________________________________________
////////////////////////////////////////////////////////////////////////////
	//This pure abstract class contains the virtual function definitions 
	//that need to be supported to satisfy the filtering requirements 
	//of the other classes in the vision library
	//Essentially this class should create and store filter data						
 
class AbstractFilter
{
public:
	virtual bool Create(	unsigned int xsize,unsigned int ysize,
							short int dataType,const void *filter)=0;
	virtual bool CreateSep(	unsigned int xsize,unsigned int ysize,
							short int dataType,const void *xFilter,const void *yFilter)=0;
		//temporal filter
	virtual bool CreateT(	unsigned int tsize,short int dataType,const void *tFilter)=0;

	virtual bool CreateDog(	unsigned int xsize,unsigned int ysize,
									unsigned int xorder,unsigned yorder,
									double sigma, double angle)=0;
	virtual bool CreateDogSep(unsigned int xsize,	unsigned int ysize,
									unsigned int xorder, unsigned int yorder,
									double sigma)=0;
	virtual bool CreateDolgT(	unsigned int tsize,unsigned int xorder,double alpha, 
									double tau,float multiplier=1.0f)=0;	
	
	virtual AbstractFilter* New() const=0;
	virtual AbstractFilter* NewHeader() const=0;
	virtual AbstractFilter* NewCopy() const=0;
	
	virtual bool Copy(const AbstractFilter &src)=0;
	virtual AbstractFilter& operator=(const AbstractFilter &f)=0;
	
	virtual inline unsigned int Width() const=0;
	virtual inline unsigned int Height() const=0;
	virtual inline unsigned int tSize() const=0;
	virtual inline short int DataType() const=0;
	virtual inline short int tDataType() const=0;
	virtual inline bool IsSep() const=0;
	virtual inline const char* ClassID() const=0;

	virtual inline const void *Filter() const=0;
	virtual inline const void *XFilter() const=0;
	virtual inline const void *YFilter() const=0;
	virtual inline const void *TFilter() const=0;

	virtual inline bool xyIsValid() const=0;
	virtual inline bool tIsValid() const=0;
	virtual inline bool SameClass(const AbstractFilter &f) const=0;
};

////////////////////////////////////////////////////////////////////////////
// ABSTRACT IMAGE CLASS_____________________________________________________
////////////////////////////////////////////////////////////////////////////
	//This pure abstract class contains the virtual function definitions 
	//that need to be supported to satisfy the image manipulation requirements 
	// of the other classes in the vision library

class AbstractImage{

public:
		//Creation destruction
	virtual bool Create(unsigned int w,unsigned int h,
				unsigned int channels,short int dataType,
				const void *data=NULL)=0;
	virtual ~AbstractImage(){};

	virtual AbstractImage &operator=(const AbstractImage &img)=0;

		//Get
	virtual inline unsigned int Width() const=0;
	virtual inline unsigned int WidthStep() const=0;
	virtual inline unsigned int WidthStepBytes() const=0;
	virtual inline unsigned int Height() const=0;
	virtual inline unsigned int Channels() const=0;
	virtual inline unsigned int Depth() const=0;
	virtual inline short int DataType() const=0;
	virtual inline const char*  ClassID() const=0;

		//Expected to return NULL if image invalid
	virtual inline void* Header() const=0;
	virtual inline void* Data() const=0;
			
		//copies to 'pixel', assumes pixel pre-allocated
	virtual inline void Pixel(void *pixel,unsigned int index) const=0;
	virtual inline void Pixel(void *pixel,unsigned int x,unsigned int y) const=0;

	virtual inline void SetPixel(void *pixel,unsigned int index) const=0;
	virtual inline void SetPixel(void *pixel,unsigned int x,unsigned int y) const=0;

	virtual bool Zero()=0;

	virtual AbstractImage* New() const=0;
	virtual AbstractImage* NewCopy() const=0;
	virtual AbstractImage* NewHeader() const=0;
	virtual AbstractImage* NewBlankCopy() const=0;
	virtual AbstractImage* NewBlankCopy(unsigned int w,unsigned int h) const=0;

		//Copy data
	virtual bool Copy(const AbstractImage &src)=0;
	virtual bool BlankCopy(const AbstractImage &src)=0;

		//Convert src data into to this image format 
		//(this should implement rescaling, depth change, colour change)
	virtual bool Convert(const AbstractImage &src)=0;
	virtual bool ConvertDataType(const AbstractImage &src,bool scaleToRange=true)=0;

		//Colour manipulations
	virtual bool RgbToBgr()=0;
	virtual bool ToGreyscale(const AbstractImage &src)=0;
	virtual bool To3ChannelGreyscale(const AbstractImage &src)=0;

		//Tests
	virtual inline bool IsValid() const=0;
	virtual inline bool SameClass(const AbstractImage &img) const=0;
	virtual inline bool SameDepth(const AbstractImage &img) const=0;
	virtual inline bool SameChannels(const AbstractImage &img) const=0;
	virtual inline bool SameDataType(const AbstractImage &img) const=0;
	virtual inline bool SameDimensions(const AbstractImage &img) const=0;
		
		//Check validity, dimensions, channels and datatype
		// but not whether SameClass()
	virtual inline bool SameType(const AbstractImage &img) const=0;

		//Arithmetic & processing
		// Functions followed by S add/subtract etc by a scalar
		// currently the address of the scalar is given as arg and
		// assumed to be the same type as the image data
		// TODO, WOULD BE MORE SENSIBLE TO HAVE FLOAT SCALING ARG REGARDLESS OF IMG TYPE
	virtual bool Add(const AbstractImage &img)=0;
	virtual bool AddS(const void *pV)=0;
	virtual bool AddS(const AbstractImage &img,const void *pV)=0;
	virtual bool Add(const AbstractImage &img,const AbstractImage &img2)=0;

	virtual inline void operator+=(const AbstractImage &img){
			Add(img);
	};

	virtual bool Subtract(const AbstractImage &img)=0;
	virtual bool SubtractS(const void *pV)=0;
	virtual bool SubtractS(const AbstractImage &img,const void *pV)=0;
	virtual bool Subtract(const AbstractImage &img,const AbstractImage &img2)=0;

	virtual inline void operator-=(const AbstractImage &img){
			Subtract(img);
	};

	virtual bool Multiply(const AbstractImage &img)=0;
	virtual bool MultiplyS(const void *pV)=0;
	virtual bool MultiplyS(const AbstractImage &img,const void *pV)=0;
	virtual bool Multiply(const AbstractImage &img,const AbstractImage &img2)=0;

	virtual inline void operator*=(const AbstractImage &img){
			Multiply(img);
	};
		
		//calculates the product of img and v and adds to 'this' image
		//(suitable for floating point images)
	virtual bool MultAcc(const AbstractImage &img,float v)=0;
	
	virtual bool Divide(const AbstractImage &img,float numThresh=ABSTRACTIMAGE_MIN_NUMER,
						float denomThresh=ABSTRACTIMAGE_MIN_DENOM)=0;
	//virtual bool TestDivideF(const AbstractImage &ImageDenom,float fNumThresh,
	//							float fDenThresh,AbstractImage &ImOutput)=0;
	virtual bool TestDivideF(	const AbstractImage &num,const AbstractImage &denom,
								float fNumThresh,float fDenThresh)=0;
	virtual bool TestInvertF(const AbstractImage &src,float fDenThresh)=0;

	virtual inline void operator/=(const AbstractImage &img){
			Divide(img);
	};

	virtual bool ThreshToZeroF(const AbstractImage &src,const AbstractImage &test,float fThresh)=0;
	virtual bool ThreshToZeroF(const AbstractImage &test,float fThresh)=0;

	virtual bool ScaleF(const AbstractImage &img,float min,float range)=0;
	virtual bool ScaleF(float min,float range)=0;

		//Math (suitable for floating point images)
	virtual bool Sq()=0;
	virtual bool Sq(const AbstractImage &img)=0;
	virtual bool Sqrt()=0;
	virtual bool Sqrt(const AbstractImage &img)=0;
	virtual bool Abs()=0;
	virtual bool Abs(const AbstractImage &img)=0;
	virtual bool ATan2(const AbstractImage &XImg,const AbstractImage &YImg)=0;

		//Convolution
	virtual bool Convolve(const AbstractImage &src,const AbstractFilter &f)=0;
	virtual bool Blur(unsigned int xSize,unsigned int ySize)=0;
	virtual bool BlurF(unsigned int size)=0;
	virtual bool TConvolve(const AbstractImage **srcs,unsigned int sz,
							const AbstractFilter &f)=0;
		//IO
	virtual bool ReadBMP(const char *fnm)=0;
	virtual bool WriteBMP(const char *fnm)=0;
};


//#include <vector>

class ImgSeq
{
public:
	ImgSeq();
	ImgSeq(const ImgSeq &src);
		//rescales to w and h using gluScaleImage
	//ImgSeq( const ImgSeq &imseq,	unsigned int w, unsigned int h );
		//copies a section of the image arg
	//ImgSeq( const ImgSeq &imseq,	unsigned int minx, unsigned int miny,
	//								unsigned int w, unsigned int h );
	virtual ~ImgSeq();
	
		//note that vector can also be accessed via Seq() below
	ImgSeq& operator=(const ImgSeq &src);

		//Note that 
	const AbstractImage& operator [](unsigned int index) const;

	bool Copy(const ImgSeq &src);
	bool Append(const ImgSeq &src);
	bool Destroy();

	unsigned int Size() const;
	
	bool AllSameType() const;
	bool Convert(const ImgSeq &src,const AbstractImage &templateImg);
	
	const vector <AbstractImage*> &Seq() const;
	void push_back(const AbstractImage &img);

	unsigned int ReadBMP(	const char *pFnmPrefix,
							unsigned int minIndex,unsigned int maxIndex,
							unsigned int increment,unsigned int noDigits,
							const AbstractImage &templateImg);
	unsigned int WriteBMP(const char *pFnmPrefix,unsigned int fnmstartindex);

private:
		//TODO: ineffficient to call Img::printbmp for each image
		//(and creating same bmp header) if the images have the same parameters
	
	vector <AbstractImage*> m_Seq;
};


////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//__________________________GLOBAL FUNCTIONS________________________________ 
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
// GLOBAL GEOMETRIC IMAGE MANIPLUATIONS
extern void InnerProd(const AbstractImage &src1x,const AbstractImage &src1y,const AbstractImage &src1t,
					  const AbstractImage &src2x,const AbstractImage &src2y,const AbstractImage &src2t, 
					  AbstractImage &dst);
extern void OuterProd(const AbstractImage &src1x,const AbstractImage &src1y,const AbstractImage &src1t,
					  const AbstractImage &src2x,const AbstractImage &src2y,const AbstractImage &src2t, 
					  AbstractImage &dstx, AbstractImage &dsty, AbstractImage &dstt);
extern void OuterProdThresh(const AbstractImage &src1x,const AbstractImage &src1y,const AbstractImage &src1t,
							const AbstractImage &src2x,const AbstractImage &src2y,const AbstractImage &src2t, 
							AbstractImage &dstx, AbstractImage &dsty, AbstractImage &dstt,
							float fThresh);


////////////////////////////////////////////////////////////////////////////
// BILINEAR INTERPOLATION
extern bool BilinearInterpolatePixel(	unsigned char *pDstPixel,const unsigned char *pSrcImg,
								unsigned int channels,
								unsigned int srcW, unsigned int srcH,
								float x, float y );
extern bool BilinearInterpolatePixel(	long *pDstPixel,const long *pSrcImg,
								unsigned int channels,
								unsigned int srcW, unsigned int srcH,
								float x, float y );
extern bool BilinearInterpolatePixel(	float *pDstPixel,const float *pSrcImg,
								unsigned int channels,
								unsigned int srcW, unsigned int srcH,
								float x, float y );

////////////////////////////////////////////////////////////////////////////
// GLOBAL CONVOLUTION FUNCTIONS
////////////////////////////////////////////////////////////////////////////
	// C-Style Convolution functions, utilised by the standard vision library image (_Img above)
extern bool Conv2Di(	long *imgOut,const long *imgIn,
						unsigned int imgW,int imgH,unsigned int channels,
						const long *kernel,
						unsigned int kXSize,unsigned int kYSize);
extern bool Conv2Df(	float *imgOut,const float *imgIn,
						unsigned int imgW,int imgH,unsigned int channels,
						const float *kernel,
						unsigned int kXSize,unsigned int kYSize);
extern bool ConvSepi(	long *imgOut,long *imgMid,const long *imgIn,
						unsigned int imgW,int imgH,unsigned int channels,
						const long *xKernel,const long *yKernel,
						unsigned int kXSize,unsigned int kYSize);
extern bool ConvSepf(	float *imgOut,float *imgMid,const float *imgIn,
						unsigned int imgW,int imgH,unsigned int channels,
						const float *xKernel,const float *yKernel,
						unsigned int kXSize,unsigned int kYSize);

extern bool Bluruc(	unsigned char *imgOut,const unsigned char *imgIn,
					unsigned int imgW,int imgH,unsigned int channels,
					unsigned int kXSize,unsigned int kYSize);

extern bool Bluri(	long *imgOut,const long *imgIn,
					unsigned int imgW,int imgH,unsigned int channels,
					unsigned int kXSize,unsigned int kYSize);

extern bool Blurf(	float *imgOut,const float *imgIn,
					unsigned int imgW,int imgH,unsigned int channels,
					unsigned int kXSize,unsigned int kYSize);

////////////////////////////////////////////////////////////////////////////
// GLOBAL FILTER GENERATION FUNCTIONS
////////////////////////////////////////////////////////////////////////////
	//C Style Math functions for creating filters
	//dirSwap swaps the order that values are read to the kernel (i.e. reflects the filter)
	//of consequence for different data-ordering conventions, ipl filter creaton functions reverse array args, 
	//these ucl libraries don't
extern bool CreateDogFilter(	float *dog,
								unsigned int xsize, unsigned int ysize,
								unsigned int xorder, unsigned int yorder,
								double sigma, double angle,bool dirSwap);
extern bool CreateDogFilterSep(	float *dogx,float *dogy,
								unsigned int xsize, unsigned int ysize,
								unsigned int xorder, unsigned int yorder,
								double sigma,bool dirSwap);
extern bool CreateDolgFilterT(	float *tKernel,
							unsigned int tsize, unsigned int order, double alpha, 
							double tau,float multiplier);
extern double Dolg(int pos, int order, double alpha, double tau); //Derivative of log gaussian
extern double Gaussian(double pos,double sig);				//Used to Generate DOG polynomials
extern double HermiteFunction(double x, double sig, int ord);
extern double Herpow(double a1, double b1); //Compute power in robust manner
extern int Binomial(int n,int r);	//Binomial Number generation
extern int Fac(int x); 	//Factorial
extern int parity(int n);


////////////////////////////////////////////////////////////////////////////
// VISION LIBRARY FILTER IMPLEMENTS ABSTRACT FILTER CLASS___________________
////////////////////////////////////////////////////////////////////////////
	// Independent filter class (i.e. not dependent on 3rd party software libraries)
	// Code specifies creation, destruction and copying of (1) derivative of gaussian
	// filters (2d and separable) for spatial filtering and (2) derivative of log
	// gaussian for temporal filtering
	// NOTE: only IMG_INT and IMG_FLOAT data types supported

class _Filter : public AbstractFilter {

	struct _FilterHdr {
		unsigned int w,h,depth;
		bool isSep;
		short int dataType;
		unsigned int imSize;
		
		short int tDataType;
		unsigned int tDepth;
		int tSize;
	};

public:

	_Filter();
	_Filter(const _Filter &f);
	virtual ~_Filter();
	
	virtual bool Create(	unsigned int xsize,unsigned int ysize,
							short int dataType,const void *filter);
	virtual bool CreateSep(	unsigned int xsize,unsigned int ysize,
							short int dataType,const void *xFilter,const void *yFilter);
	virtual bool CreateT(	unsigned int tsize,short int dataType,const void *tFilter);
	
			//gaussian filters
	virtual bool CreateDog(	unsigned int xsize,unsigned int ysize,
									unsigned int xorder,unsigned yorder,
									double sigma, double angle);
	virtual bool CreateDogSep(unsigned int xsize,	unsigned int ysize,
									unsigned int xorder, unsigned int yorder,
									double sigma);
	virtual bool CreateDolgT(	unsigned int tsize,unsigned int xorder,double alpha, 
									double tau,float multiplier=1.0f);	
	
	virtual bool Copy(const AbstractFilter &src);
	virtual AbstractFilter* New() const;
	virtual AbstractFilter* NewCopy() const;
	virtual AbstractFilter* NewHeader() const;
	
	virtual AbstractFilter& operator=(const AbstractFilter &f);
	
	bool Destroy();
	bool xyDestroy();
	bool tDestroy();

		//can simultaneously contain spatial and temporal filter
		//spatial filter is either seperable or not seperable, but not both
	virtual inline bool xyIsValid() const;
	virtual inline bool tIsValid() const;

	virtual inline unsigned int Width() const;
	virtual inline unsigned int Height() const;
	virtual inline unsigned int tSize() const;
	virtual inline bool IsSep() const;
	virtual inline short int DataType() const;
	virtual inline short int tDataType() const;
	virtual inline const char* ClassID() const;

	virtual inline bool SameClass(const AbstractFilter &f) const;

	virtual inline const void *Filter() const;
	virtual inline const void *XFilter() const;
	virtual inline const void *YFilter() const;
	virtual inline const void *TFilter() const;
	
private:

	bool CheckDataType(short int dataType);
	bool CalcDepth(unsigned int &depth,short int dataType);
	
	_FilterHdr m_Hdr;
			//2D Kernel
	void *m_Kernel;	
			//Sep Kernels
	void *m_XKernel,*m_YKernel;	
			//For temporal Convolution
	void *m_TKernel;										
	
	//	the following are currently not utilised
	//	would be used for storing temporary data in filtering operations
	//	to avoid repeat memory allocation
	void *m_pInput;
	void *m_pOutput;
			// storage for mid stage output of separable filtering
	void *m_pSepOutput;
};

////////////////////////////////////////////////////////////////////////////
// VISION LIBRARY IMAGE IMPLEMENTS ABSTRACT IMAGE CLASS_____________________
////////////////////////////////////////////////////////////////////////////
	// Independent image class (i.e. not dependent on 3rd party software libraries)
	// Code specifies I/O, creation, destruction, copying and format conversion of images.
	// Code implements convolution using filters from the AbstractFilter class
	
class _Img : public AbstractImage 
{

	struct _ImgHdr {
		unsigned int w,h,channels,depth;
		short int dataType;
	};

	public:
		_Img();
		_Img(	unsigned int w,unsigned int h,
				unsigned int channels,short int dataType=IMG_UCHAR);
		_Img(const _Img &img);
		virtual ~_Img();

			//Creation destruction
		virtual bool Create(unsigned int w,unsigned int h,
				unsigned int channels,short int dataType,
				const void *data=NULL);
		bool Destroy();

		virtual AbstractImage &operator=(const AbstractImage &img);

			//Get
		virtual inline unsigned int Width() const;
		virtual inline unsigned int WidthStep() const;
		virtual inline unsigned int WidthStepBytes() const;
		virtual inline unsigned int Height() const;
		virtual inline unsigned int Channels() const;
		virtual inline unsigned int Depth() const;
		virtual inline short int DataType() const;
		virtual inline const char* ClassID() const;

		virtual inline void* Header() const;
			//returns NULL if image invalid
		virtual inline void* Data() const;

			// 'pixel' assumed to have been pre-allocated
			// No array out of bounds check on indexing
		virtual inline void Pixel(void *pixel,unsigned int index) const;
		virtual inline void Pixel(void *pixel,unsigned int x,unsigned int y) const;

		virtual inline void SetPixel(void *pixel,unsigned int index) const;
		virtual inline void SetPixel(void *pixel,unsigned int x,unsigned int y) const;

			// Returns pointer to the actual image data
		inline void* Pixel(unsigned int index) const;
		inline void* Pixel(unsigned int x,unsigned int y) const;
			
		// checks whether x,y out of bounds, if so returns NULL
		inline void* Pixel2(unsigned int x,unsigned int y) const;

		virtual bool Zero();

		virtual AbstractImage* New() const;
		virtual AbstractImage* NewCopy() const;
		virtual AbstractImage* NewHeader() const;
		virtual AbstractImage* NewBlankCopy() const;
		virtual AbstractImage* NewBlankCopy(unsigned int w,unsigned int h) const;
		
			//Copy data
		virtual bool Copy(const AbstractImage &src);
		bool Copy(	const AbstractImage &src,
					unsigned int x,unsigned int y,
					unsigned int xRange,unsigned int yRange);
		virtual bool BlankCopy(const AbstractImage &src);

		bool Paste(	const AbstractImage &src,	
					unsigned int x,unsigned int y);
		bool PasteTile(unsigned int rows,unsigned int cols,const AbstractImage **ppSrcs);

			//if data type to be converted, automatically scales to range (no-effect with float)
			//if the resultant image (this) is smaller than the arg image, its recommended to 
			//blur the source image beforehand
		virtual bool Convert(const AbstractImage &src);
			//note, data range is unspecified for float data so scale to range has no effect
			//		i.e. values are copied straight to or from float data 
			//		(and saturate if out of range)
		virtual bool ConvertDataType(const AbstractImage &src,bool scaleToRange=true);
		bool ConvertDataType(const unsigned char *pData,bool scaleToRange);
		bool ConvertDataType(const long *pData,bool scaleToRange);
		bool ConvertDataType(const float *pData);

			//if the resultant image (this) is smaller than the arg image, its recommended to 
			//blur the source image before calling this funct
		bool Resize(const AbstractImage &src);
		
		virtual bool RgbToBgr();
		virtual bool ToGreyscale(const AbstractImage &src);
		virtual bool To3ChannelGreyscale(const AbstractImage &src);

		virtual bool ScaleF(const AbstractImage &img,float min,float range);
		virtual bool ScaleF(float min,float range);

			//Tests
		virtual inline bool IsValid() const;
		virtual inline bool SameDepth(const AbstractImage &img) const;
		virtual inline bool SameChannels(const AbstractImage &img) const;
		virtual inline bool SameDimensions(const AbstractImage &img) const;
		virtual inline bool SameDataType(const AbstractImage &img) const;
		
		virtual inline bool SameClass(const AbstractImage &img) const;
		virtual inline bool SameType(const AbstractImage &img) const;
		
		inline bool SameHdr(const _Img &img) const;

			//Arithmetic
		virtual bool Add(const AbstractImage &img);
		virtual bool AddS(const void *pV);
		virtual bool AddS(const AbstractImage &img,const void *pV);
		virtual bool Add(const AbstractImage &img,const AbstractImage &img2);

		virtual bool Subtract(const AbstractImage &img);
		virtual bool SubtractS(const void *pV);
		virtual bool SubtractS(const AbstractImage &img,const void *pV);
		virtual bool Subtract(const AbstractImage &img,const AbstractImage &img2);

		virtual bool Multiply(const AbstractImage &img);
		virtual bool MultiplyS(const void *pV);
		virtual bool MultiplyS(const AbstractImage &img,const void *pV);
		virtual bool Multiply(const AbstractImage &img,const AbstractImage &img2);
		
		virtual bool MultAcc(const AbstractImage &img,float v);

		virtual bool Divide(const AbstractImage &img);
		virtual bool Divide(const AbstractImage &img,float numThresh,float denomThresh);
		
		virtual bool TestDivideF(	const AbstractImage &num,const AbstractImage &denom,
									float fNumThresh,float fDenThresh);
		virtual bool TestInvertF(const AbstractImage &src,float fDenThresh);

		virtual bool ThreshToZeroF(const AbstractImage &src,const AbstractImage &test,float fThresh);
		virtual bool ThreshToZeroF(const AbstractImage &test,float fThresh);

		virtual bool Sq();
		virtual bool Sq(const AbstractImage &img);
		virtual bool Sqrt();
		virtual bool Sqrt(const AbstractImage &img);
		virtual bool Abs();
		virtual bool Abs(const AbstractImage &img);
		virtual bool ATan2(const AbstractImage &XImg,const AbstractImage &YImg);

		virtual bool Convolve(const AbstractImage &src,const AbstractFilter &f);
		virtual bool Blur(unsigned int xSize,unsigned int ySize);
		virtual bool BlurF(unsigned int size);
		virtual bool TConvolve(const AbstractImage **srcs,unsigned int sz,
													const AbstractFilter &f);
		//virtual bool MultiplyAccumulate(const AbstractImage &img1,
		//								const AbstractImage &img2);
		//virtual bool Convolve(const Filter &f);

		virtual bool ReadBMP(const char *fnm);
		virtual bool WriteBMP(const char *fnm);

private:
		inline unsigned int DataSz() const;

			//allocates memory for data and initialises to zero
		bool Alloc();
		bool AllocInit();
		bool CheckDataType(short int dataType) const;
		bool CalcDepth(unsigned int &depth,short int dataType) const;

		bool ErrorCheckConvolve(const AbstractImage &src,const AbstractFilter &f);
		bool ErrorCheckTConvolve(const AbstractImage **srcs,unsigned int sz,
														const AbstractFilter &f);
		void *m_pData;
		_ImgHdr m_Hdr;
};

#ifdef _UCLVisionResearchLabLib_H_IPL_INCLUDED_
////////////////////////////////////////////////////////////////////////////
// IPL FILTER
////////////////////////////////////////////////////////////////////////////
	// Interface to the IPL filter code
	// Code specifies creation, destruction and copying of (1) derivative of gaussian
	// filters (2d and separable) for spatial filtering and (2) derivative of log
	// gaussian for temporal filtering
	// NOTE only IMG_INT and IMG_FLOAT data types supported as with vision library filter


class CIplFilter : public AbstractFilter
{
	struct CIplFilterHdr {
		unsigned int w,h,depth;
		bool isSep;
		short int dataType;
		unsigned int imSize;
		
		short int tDataType;
		unsigned int tDepth;
		int tSize;
	};

public:
	CIplFilter();
	CIplFilter(const CIplFilter &f);
	CIplFilter &operator=(const CIplFilter &f);
	virtual AbstractFilter& operator=(const AbstractFilter &f);

	virtual ~CIplFilter();
	
	virtual bool Create(	unsigned int xsize,unsigned int ysize,
							short int dataType,const void *filter);
	virtual bool CreateSep(	unsigned int xsize,unsigned int ysize,
							short int dataType,const void *xFilter,const void *yFilter);
	virtual bool CreateT(	unsigned int tsize,short int dataType,const void *tFilter);
	
			//gaussian filters
	virtual bool CreateDog(	unsigned int xsize,unsigned int ysize,
									unsigned int xorder,unsigned yorder,
									double sigma, double angle);
	virtual bool CreateDogSep(unsigned int xsize,	unsigned int ysize,
									unsigned int xorder, unsigned int yorder,
									double sigma);
	virtual bool CreateDolgT(	unsigned int tsize,unsigned int xorder,double alpha, 
									double tau,float multiplier=1.0f);	
	
	bool CreateInt(	unsigned int xsize,unsigned int ysize,
								const long *filter,int shift);
	bool CreateSepInt(	unsigned int xsize,unsigned int ysize,
								const long *xFilter,const long *yFilter,
								int xShift,int yShift);

	virtual bool Copy(const AbstractFilter &src);
	virtual AbstractFilter* New() const;
	virtual AbstractFilter* NewCopy() const;
	virtual AbstractFilter* NewHeader() const;
	
	bool Destroy();
	bool xyDestroy();
	bool tDestroy();

		//can simultaneously contain spatial and temporal filter
		//spatial filter is either seperable or not seperable, but not both
	virtual inline bool xyIsValid() const;
	virtual inline bool tIsValid() const;

	virtual inline unsigned int Width() const;
	virtual inline unsigned int Height() const;
	virtual inline unsigned int tSize() const;
	virtual inline bool IsSep() const;
	virtual inline short int DataType() const;
	virtual inline short int tDataType() const;
	virtual inline const char* ClassID() const;

	virtual inline bool SameClass(const AbstractFilter &f) const;

	virtual inline const void *Filter() const;
	virtual inline const void *XFilter() const;
	virtual inline const void *YFilter() const;
	virtual inline const void *TFilter() const;

	inline const void *iplFilter() const;
	inline const void *iplXFilter() const;
	inline const void *iplYFilter() const;

	bool CreateNumDiffFilterX(int xorder);
	bool CreateNumDiffFilterY(int yorder);
	//bool QuantiseFilter(int bitdepth);						//Turns Float filter to Integer
	
	//bool AddNoise(IplImage *src,double stddev);				//Add Gaussian Noise
	//bool Median(IplImage *src,IplImage *dst,int size);		//Median Filter
	//bool Average(IplImage *src,IplImage *dst, int size);	//Box Filter
	//bool Blur(IplImage *src,IplImage *dst);					//3x3Gaussian
	//bool Convolve(IplImage *src,IplImage *dst);				//Public Convolve Routine
	//bool TConvolve(IplImage **src,IplImage *dst);			//Convolve a temporal sequence
	//void Print(const char *Filename=NULL);					//Print Filter Values to File
private:

	bool CheckDataType(short int dataType) const;
	bool CalcDepth(unsigned int &depth,short int dataType) const;

	CIplFilterHdr m_Hdr;	
	void *m_Kernel;		//2D Kernel
	void *m_XKernel,*m_YKernel;	//Sep Kernels
	
	void *m_TKernel;	//For temporal Convolution
};


////////////////////////////////////////////////////////////////////////////
// IPL IMAGE IMPLEMENTS ABSTRACTIMAGE CLASS
////////////////////////////////////////////////////////////////////////////
	// Interface for the IPL filter class to allow compatibility with 
	// vision libraries
	// Code specifies I/O, creation, destruction, copying and format conversion of images.
	// Code implements convolution using filters from the AbstractFilter class

//#include "C:\Program Files\Intel\plsuite\include\IPL.h"
//#include <IPL.h>


struct CIpl_RGBA32F {
	float b;
	float g;
	float r;
	float a;
};

struct CIpl_RGB {
	BYTE b;
	BYTE g;
	BYTE r;
};


class CIplImg : public AbstractImage
{
	public:
//Construction/Destruction Initialisation
	
		CIplImg();
		CIplImg(	unsigned int w,unsigned int h,
					unsigned int channels,short int dataType=IMG_UCHAR);
		CIplImg(const CIplImg &img);
		CIplImg &operator=(const CIplImg &img);
		virtual ~CIplImg();

			//Creation destruction
		virtual bool Create(unsigned int w,unsigned int h,
				unsigned int channels,short int dataType,
				const void *data=NULL);
		virtual bool Destroy();

		virtual AbstractImage &operator=(const AbstractImage &img);

			//Get
		virtual inline unsigned int Width() const;
		virtual inline unsigned int WidthStep() const;
		virtual inline unsigned int WidthStepBytes() const;
		virtual inline unsigned int Height() const;
		virtual inline unsigned int Channels() const;
		virtual inline unsigned int Depth() const;
		virtual inline short int DataType() const;
		virtual inline const char* ClassID() const;

		virtual inline void* Header() const;
			//returns NULL if image invalid
		virtual inline void* Data() const;

			// No array out of bounds check on indexing
		virtual inline void Pixel(void *pixel,unsigned int index) const;
		virtual inline void Pixel(void *pixel,unsigned int x,unsigned int y) const;

		virtual inline void SetPixel(void *pixel,unsigned int index) const;
		virtual inline void SetPixel(void *pixel,unsigned int x,unsigned int y) const;
			
		// checks whether x,y out of bounds, if so returns NULL
		inline void* Pixel2(unsigned int x,unsigned int y) const;

			//Copy data
		virtual bool Copy(const AbstractImage &src);
		virtual bool BlankCopy(const AbstractImage &src);
		virtual AbstractImage* New() const;
		virtual AbstractImage* NewCopy() const;
		virtual AbstractImage* NewHeader() const;
		virtual AbstractImage* NewBlankCopy() const;
		virtual AbstractImage* NewBlankCopy(unsigned int w,unsigned int h) const;
		
			//if the resultant image (this) is smaller than the arg image, its recommended to 
			//blur the source image before calling this funct
		virtual bool Convert(const AbstractImage &src);
		virtual bool ConvertDataType(const AbstractImage &src,bool scaleToRange=true);

			//if the resultant image (this) is smaller than the arg image, its recommended to 
			//blur the source image before calling this funct
		bool Resize(const AbstractImage &src);
		
		virtual bool RgbToBgr();
		virtual bool ToGreyscale(const AbstractImage &src);
		virtual bool To3ChannelGreyscale(const AbstractImage &src);

		virtual bool ScaleF(const AbstractImage &img,float min,float range);
		virtual bool ScaleF(float min,float range);

			//Tests
		virtual inline bool IsValid() const;
		virtual inline bool SameDepth(const AbstractImage &img) const;
		virtual inline bool SameChannels(const AbstractImage &img) const;
		virtual inline bool SameDimensions(const AbstractImage &img) const;
		virtual inline bool SameDataType(const AbstractImage &img) const;
		
		virtual inline bool SameClass(const AbstractImage &img) const;
		virtual inline bool SameType(const AbstractImage &img) const;
		
		inline bool SameHdr(const _Img &img) const;

			//Arithmetic
		virtual bool Add(const AbstractImage &img);
		virtual bool AddS(const void *pV);
		virtual bool AddS(const AbstractImage &img,const void *pV);
		virtual bool Add(const AbstractImage &img,const AbstractImage &img2);

		virtual bool Subtract(const AbstractImage &img);
		virtual bool SubtractS(const void *pV);
		virtual bool SubtractS(const AbstractImage &img,const void *pV);
		virtual bool Subtract(const AbstractImage &img,const AbstractImage &img2);

		virtual bool Multiply(const AbstractImage &img);
		virtual bool MultiplyS(const void *pV);
		virtual bool MultiplyS(const AbstractImage &img,const void *pV);
		virtual bool Multiply(const AbstractImage &img,const AbstractImage &img2);

		virtual bool MultAcc(const AbstractImage &img,float v);

		virtual bool Divide(const AbstractImage &img);
		virtual bool Divide(const AbstractImage &img,float numThresh,float denomThresh);

		virtual bool TestDivideF(	const AbstractImage &num,const AbstractImage &denom,
									float fNumThresh,float fDenThresh);
		virtual bool TestInvertF(const AbstractImage &src,float fDenThresh);

		virtual bool ThreshToZeroF(const AbstractImage &src,const AbstractImage &test,float fThresh);
		virtual bool ThreshToZeroF(const AbstractImage &test,float fThresh);

		virtual bool Sq();
		virtual bool Sq(const AbstractImage &img);
		virtual bool Sqrt();
		virtual bool Sqrt(const AbstractImage &img);
		virtual bool Abs();
		virtual bool Abs(const AbstractImage &img);
		virtual bool ATan2(const AbstractImage &XImg,const AbstractImage &YImg);

		virtual bool Convolve(const AbstractImage &src,const AbstractFilter &f);
		virtual bool Blur(unsigned int xSize,unsigned int ySize);
		virtual bool BlurF(unsigned int size);
		virtual bool TConvolve(const AbstractImage **srcs,unsigned int sz,
													const AbstractFilter &f);
		bool TConvolveIPL(const CIplImg **srcs,unsigned int sz,const CIplFilter &f);
		virtual bool ReadBMP(const char *fnm);
		virtual bool WriteBMP(const char *fnm);

///////////////////////////////////////////////////////////////////////////////////
// OLD FUNCTIONS
// Below are the functions written for the original CIPLImage class (contrasting to
// this class CIplImg


	virtual bool Create(unsigned int Width, unsigned int Height, const void *data=NULL, int Depth=IPL_DEPTH_8U,int Planes=1);
	bool Create8U(unsigned int Width, unsigned int Height, const void *data=NULL);
	bool Create16U(unsigned int Width, unsigned int Height, const void *data=NULL);
	bool Create16S(unsigned int Width, unsigned int Height, const void *data=NULL);
	bool Create32S(unsigned int Width, unsigned int Height, const void *data=NULL);
	bool Create32F(unsigned int Width, unsigned int Height, const void *data=NULL);
	bool CreateRGB(unsigned int Width, unsigned int Height, const void *data=NULL);
	bool CreateRGBFP(unsigned int Width, unsigned int Height, const void *data=NULL);
	
//Image Data IO
	inline IplImage *GetpImg(void) {return m_Img;}
	inline void PutpImg(IplImage *p) {return;}
	inline const void *GetpData(void) {return (void *)(m_Img->imageData);}
	inline void PutpData(void *p) {return;}

	/*
	void OnDraw(CDC *pDC=NULL, const CRect *rectDestSize=NULL);
	void OnDrawExternal(void);

	
	bool MakeFullRangeViewableImage();
	bool MakeViewableAngleImage(bool bRadians = true);
	*/

	//bool Read(const char *fnm);
	//bool ReadBMP(FILE *_f);

	//bool Write(const char *fnm);
	//bool WriteBMP(FILE *_f);

	//bool Load(const char *Filename=NULL);//Load by Dialog if Filename==NULL
	//bool Save(const char *Filename=NULL);//Save by Dialog if Filename==NULL
	//bool LoadIPL(HANDLE hFile);
	//bool SaveIPL(HANDLE hFile);
	//bool LoadBMP(HANDLE hfile);
	//bool SaveBMP(HANDLE hFile);
	//bool CopyToClipboard(void);
	//bool LoadRaw(const char *Filename,unsigned long Width,unsigned long Height,unsigned int Planes=1);
	//bool SaveRaw(const char *Filename);
	CIpl_RGBA32F GetPixel(int x,int y);
	const CIpl_RGBA32F GetPixel(int x,int y) const;
	
	//void GetPixel32F(int x,int y,float *result);
	//void SetPixel(int x, int y,CIpl_RGBA32F val);
	//void SetPixel(int x,int y,CIpl_RGB val);
	//void SetPixel(int x, int y,float *val);
	//void SetPixel(int x, int y,int *val);

	
	//Copying
	CIplImg * BlankCopy(void) const;
	CIplImg * Clone(void) const;
	//bool Copy(CIplImg &src);//Copies from src into this
	CIplImg * Decimate(int factor);//Resize image
	CIplImg * Zoom(int factor);//Resize image
	bool Resize(CIplImg *dst,int inter=IPL_INTER_LINEAR);//Resize image
	bool ShiftVertical(int nPixels);//Move image data up/down
	bool ShiftHorizontal(int nPixels);//Move image data left/right

//Conversion
	CIplImg * ToGreyScale();
	bool ToGreyScale(CIplImg *dst);
	CIplImg * ConvertDepthTo(int depth,bool bScaleToFitRange=true);
	bool ConvertDepthTo(CIplImg *dst, bool bScaleToFitRange=true);
	CIpl_RGBA32F RadToColour(float rad);
	CIpl_RGB RadToColourLUT (float rad);
	void RadToColourLUTInit(int LUTSize, int nAngleMult=1);
//Synthetic
	bool Zero(void);
	bool Fill(int value);
	bool FillRamp(float xinc,float yinc,float offset);
	bool FillSine(float xfreq, float yfreq,float xphase,float yphase,float rot_degs);
	bool FillCheck(unsigned int size,unsigned int offset);
	bool FillJaehne(void);
//Processing
	//bool SetROI(const CRect &roi);
	//bool ResetROI(void);
	bool Blur(int size);
	bool GBlur3(int iter);
	bool GBlur5(int iter);
//Misc
	inline const int ImageSize() const {return m_Img->imageSize;} // returns useful size of image array in bytes
	
	bool IsSameTypeAs(const CIplImg *src) const;
	//static void GetIplVersion(CString *Info);
	int GetVersion(void);
	
	__declspec(property(get=GetpImg, put=PutpImg)) IplImage * pImg;
	__declspec(property(get=GetpData, put=PutpData)) void * pData;

//Arithmetic
	//bool Square(void);
	//bool Root(void);
	//bool Abs(void);

	/*
	CIpl_RGB32F Mean(CRect *region=NULL);
	void GetStats(CRect *region=NULL, CIpl_RGB32F *usrMean=NULL, CIpl_RGB32F *usrStdDev=NULL, CIpl_RGB32F *usrMin=NULL, CIpl_RGB32F *usrMax=NULL);
	*/
	//bool ATan2(CIplImg &XImg,CIplImg &YImg);
	//bool Mac1(CIplImg &AImg,float A);
	//bool Mac2(CIplImg &AImg,CIplImg &BImg,float A,float B);
	//bool Mac3(CIplImg &AImg,CIplImg &BImg,CIplImg &CImg,float A,float B, float C);
	//void operator+=(CIplImg &input);
	//void operator+=(float val);
	//void operator+=(int val);
	//void operator*=(CIplImg &image);
	//void operator*=(float k);
	//void operator*=(int k);
	//void operator-=(CIplImg &input);
	//void operator-=(float k);
	//void operator-=(int k);
	//void operator/=(CIplImg &input);
	//void TestDivide(CIplImg *denominator, float fNumThresh=MIN_NUMER,float fDenThresh=MIN_DENOM, CIplImg *output=NULL);
	//void TestInvert(float fDenThresh, CIplImg *ImOutput);
	//void RoundToZero(float fThresh, CIplImg *ImTest, CIplImg *ImOutput);
	//void operator/=(float k);
	
		//aja removed, allocates new memory
	//CIplImg * operator=(CIplImg &input);
	
private:
	IplImage *m_Img;			//The IPL Image Header
	IplImage *m_ImgDisp;		//Secondary Image for display only
	unsigned char *m_Bmi;
	bool m_bHasROI;

	inline unsigned int DataSz() const;
	bool CheckDataType(short int dataType) const;
	bool ImgDataTypeToIPLDataType(int &iplDataType,short int imgDataType) const;
	bool IPLDataTypeToImgDataType(short int &imgDataType,int iplDataType) const;
	bool CalcDepth(unsigned int &depth,short int dataType) const;


	bool ErrorCheckConvolve(const AbstractImage &src,const AbstractFilter &f);
	bool ErrorCheckTConvolve(const AbstractImage **srcs,unsigned int sz,
														const AbstractFilter &f);
	
	bool ConvolveSep2Di(const CIplImg &src,const CIplFilter &f);
	bool ConvolveSep2Df(const CIplImg &src,const CIplFilter &f);
	bool Convolve2Di(const CIplImg &src,const CIplFilter &f);
	bool Convolve2Df(const CIplImg &src,const CIplFilter &f);

	bool MakeViewableImage(float min=0,float max=255.0f);
	bool ReadBMP(FILE *_f);
	bool WriteBMP(FILE *_f);
};

#endif // IPL  #ifdef _UCLVisionResearchLabLib_H_IPL_INCLUDED_



///////////////////////////////////////////////////////////////////////////////////
// STEER BASIS: BASIS SET OF STEERED FILTERS________________________________________
///////////////////////////////////////////////////////////////////////////////////
// Reconstructs image from basis set (above).
// Supports image enlargement and warping


//MAXORDER sets the maximum derivative order which the class can handle

const unsigned int CSTEERBASIS_MAXORDER=15;//20;
const double CSTEERBASIS_MINSTEERWEIGHT=0.0000001;

class CSteerBasis  
{
public:
	
	CSteerBasis(	const AbstractFilter &prototypeFilter,
					unsigned int order,unsigned int support,float sigma);
		//aja
	CSteerBasis(const CSteerBasis &sb);
	CSteerBasis &operator=(const CSteerBasis &sb);

	bool Copy(const CSteerBasis &sb);
	bool Destroy();
	virtual ~CSteerBasis();
		
	bool MakeBasis(const AbstractImage &src);
	bool MakeBasisFast(const AbstractImage &src);
	bool MakeOriented(AbstractImage *src,float angle,CSteerBasis *dst=NULL);
	bool SteerBasis(float rad_angle);	//Generates Oriented Set to Internal
										//should make dst this image
	bool SteerBasis(float rad_angle,CSteerBasis *dst);
	bool SteerBasis90(CSteerBasis *dst);

	bool QuantiseFilters(int bitdepth);
	
		//aja adapated
	AbstractFilter *GetFilter(unsigned int XOrder,unsigned int YOrder);
	const AbstractFilter *GetFilter(unsigned int XOrder,unsigned int YOrder) const;
	
	AbstractImage *GetResponse(unsigned int XOrder,unsigned int YOrder);
	const AbstractImage *GetResponse(unsigned int XOrder,unsigned int YOrder) const;

	AbstractImage *GetOrientedResponse(unsigned int XOrder,unsigned int YOrder);
	const AbstractImage *GetOrientedResponse(unsigned int XOrder,unsigned int YOrder) const;
		
	bool SetResponse(unsigned int XOrder,unsigned int YOrder, AbstractImage *response);

		//aja
	unsigned int GetOrder() const { return m_nOrder; };
	unsigned int GetSupport() const { return m_nSupport; };
	unsigned int GetStDev() const { return m_fSigma; };
	unsigned int GetResponseWidth() const { return m_nResponseW; };
	unsigned int GetResponseHeight() const { return m_nResponseH; };
	
		//aja commented out
	//bool OnDraw(CDC *pDC,int nZoom=1, bool DrawOriented=false);
	inline void SetContrastRange(float range){m_fDrawContrast=range;};

private:
						//support size of filter kernel
	bool InitFilters(const AbstractFilter &prototypeFilter,
					unsigned int order,unsigned int support,float sigma);
	bool SteerWeight(float rad_angle);					//Generate All Steering Weight
	//aja changed from CString to string
	string SteerWeightFunctionString(int Num,int CosPow,int SinPow);
	int binomial(int N,int i);
	int parity(int n);
	int fac(int x);

	float m_fDrawContrast;	//Used for contrast stretching results during drawing
	unsigned int m_nOrder;
	unsigned int m_nSupport;
	unsigned int m_nNumFilterOrder;
	float m_fSigma;

		//aja
		//response width and height assumed to be the same for all images
	unsigned int m_nResponseW;
	unsigned int m_nResponseH;

	AbstractFilter *m_FilterSep[CSTEERBASIS_MAXORDER][CSTEERBASIS_MAXORDER];	//Separable Filters go in here
	AbstractFilter *m_NumFilterSep[2];					//Numerical Filters go in here
	AbstractImage *m_Response[CSTEERBASIS_MAXORDER][CSTEERBASIS_MAXORDER];		//Separable Basis Set goes in here
	AbstractImage *m_RotResponse[CSTEERBASIS_MAXORDER][CSTEERBASIS_MAXORDER];	//Rotated Basis Set goes in here
	float m_fSteerWeights[CSTEERBASIS_MAXORDER][CSTEERBASIS_MAXORDER][CSTEERBASIS_MAXORDER+1];			//Holds All Steering Weights
};


///////////////////////////////////////////////////////////////////////////////////
// Image reconstruction based on Taylor expansion__________________________________
///////////////////////////////////////////////////////////////////////////////////
// Reconstructs image from basis set (above).
// Supports image enlargement and warping

const unsigned int  TAYLORRECONSTRUCT_MAXORDER=15;
const unsigned int  TAYLORRECONSTRUCT_SUPPORT=23;

class TaylorReconstruct  
{
public:
	TaylorReconstruct();
	TaylorReconstruct(	unsigned int order,
						float xoffset,float yoffset,float soffset);
	virtual ~TaylorReconstruct();
	void Destroy();

	//TO DO: Write Copy constructor (add taylor expansion header struct as for images)
	
	bool Set(	unsigned int order,
				float xoffset,float yoffset,float soffset);
		

		//NOTE: currently no check for compatibility between 
		//no. channels in output and basis

	bool SlowReconstruct(AbstractImage &output,const CSteerBasis &basis);
		//allow space variant mapping
	bool SlowReconstruct(AbstractImage &output,const CSteerBasis &basis,
							const float *spvmap_x,const float *spvmap_y);
		//allow warp
	bool SlowReconstructWarp(AbstractImage &output,
							const CSteerBasis &basis,
							const float *warp_x,const float *warp_y);
		//allow warp and space variant mapping
	bool SlowReconstruct(AbstractImage &output,
									const CSteerBasis &basis,
									const float *spvmap_x,const float *spvmap_y,
									const float *warp_x,const float *warp_y);

		//ok for enlargement (but reduction not necessarily ideal)
	bool FastReconstruct(AbstractImage &output,const CSteerBasis &basis);
		//mask space allows base set to be subsampled 
		//(tho given a basis set reconstruction time increases proportional to subsampling)
	bool FastReconstruct(AbstractImage &output,const CSteerBasis &basis,unsigned int MaskSpace);
	
		
private:
	
	bool ParameterCheck(const AbstractImage &output,const CSteerBasis &basis,
						unsigned int &temporder);
	bool GenWeights(int MaxXorder,float xoffset=0.0f,float ysoffset=0.0f, float soffset=0.0f);
	
	
	//aja TODO LUT of weights for repeated reconstructions
	//store warp maps, etc in object?
	
	float m_fXOffset;
	float m_fYOffset;
	float m_fSOffset;
	int m_nOrder;
	float m_fWeights[TAYLORRECONSTRUCT_MAXORDER][TAYLORRECONSTRUCT_MAXORDER];
};



///////////////////////////////////////////////////////////////////////////////////
// McGM Version 2005______________________________________________________________
///////////////////////////////////////////////////////////////////////////////////
// Calculates optic flow from a sequence of abstract images
// Outputs representation of optic flow in terms of two floating point abstract images
// corresponding to the angle and speed of motion at each pixel in the input
// 
// McGM2005 follows a similar structure to J.Dale's original IPL McGM class

//#include <list>

// Thresholding Defines
const short int McGM2005_MAXANGLES=24;

//Critical Parameters structure contains the McGM model parameters that
//cannot be changed `live`.   These params are passed to the McGM Init 
//function.  Current values can be obtained using McGM::GetCriticalParams()
struct McGM2005CriticalParameters{
	long m_nSize;				//Size of this structure		
	
		// aja removed
	//int m_nSubSampleInput;	//Subsample Input Image		(Def 2)
	bool m_bFastSpatialFiltering;//Numerical Filtering			(Def True)
	int m_nAngles;				//Numer of Orientation Columns	(Def 24)
	int m_nTSupportSize;		//Size of Temporal Support		(Def 23)
	int m_nXSupportSize;		//Size of Spatial Support		(Def 23)
	int m_nXOrders;				//Primary Derivatives			(Def 5)
	int m_nYOrders;				//Orthogonal Derivatives		(Def 2)
	int m_nTOrders;				//Number of Temporal Filters	(Def 3)
	int m_nIntegrationZone;		//Integration Zone for Weights	(Def 11)
	int m_nBorder;				//Border for Anglewheel			(Def 10)
	float m_fQuotThreshDen;		//Quotients Threshold			
	float m_fQuotThreshNum;		//Quotients Threshold
	float m_fTau;				//Temporal Filter Params		(Def 0.25)
	float m_fAlpha;				//Temporal Filter Params		(Def 10)
	float m_fSigma;				//Spatial Filter Size			(Def 1.5)
	int m_nGrabber;				//PXC200 grabber number
	int m_nChannel;				//PXC200 channel
};

class McGM2005  
{
public:
//Creation/Destruction
	McGM2005();
	
	//aja:: really think we ought to get rid of all of the pointers
	//TODO, should write a copy function to cater for the below
	McGM2005(const McGM2005 &mcgm);
	McGM2005 &operator=(const McGM2005 &mcgm);

	virtual ~McGM2005();
	virtual void Init(McGM2005CriticalParameters *pCPInit,
						const AbstractFilter &prototypeFilter);	//Model can be init with new params
	virtual void Destroy(void);
	bool InitOK(void){return m_bInitOK;}
	
	bool ReadCriticalParameters(const char *fnm,AbstractFilter &prototypeFilter);
	void GetCriticalParams(McGM2005CriticalParameters *cp);

//Execution
	
		//aja 4/4/05
		//Processes the img arg
		//Args: assumed that img is a greyscale image, dimensions w*h*sizeof(unsigned char)
		//Returns false	:: if the temporal buffer has not accumulated enough images to begin processing
		//				   (as determined by m_nTSupportSize in critical params).
	bool Process(const AbstractImage &input);
	
		//Get OpFlow Output
	inline AbstractImage** OpFlow() { return m_ImOpFlow; }
	bool MaskedOpFlow(	AbstractImage &angle,AbstractImage &speed,
						float minSpeed,float speedRange,float speedGreyRange);

	inline const AbstractImage *Mask() const {return m_ImMask;}
		
//Manipulating Results
	inline AbstractImage** TResponses() {return m_ImTResponse;}

	AbstractImage *GetTFilterResult(int n) const;
	AbstractImage *GetXFilterResult(int xorder, int yorder,int torder,int angle=0) const;

	inline int GetBorder() {return McGMParams.m_nBorder;}
	inline int GetTFilterSupport(){return McGMParams.m_nTSupportSize;}

//Interface to Parameters that can be tweaked `live`
	inline void SetDoQuotientBlur(bool bDoBlur){m_bBlurQuotients=bDoBlur;}
	inline bool GetDoQuotientBlur(){return m_bBlurQuotients;}
	inline void SetMaskThresh(float fThresh){m_fMaskThresh=fThresh;}
	inline float GetMaskThresh(){return m_fMaskThresh;}
	inline void SetDoMask(bool bDoMask){m_bDoMask=bDoMask;}
	inline bool GetDoMask(){return m_bDoMask;}
	inline void SetQuotThresh(float fNumThresh,float fDenThresh){McGMParams.m_fQuotThreshNum=fNumThresh;McGMParams.m_fQuotThreshDen=fDenThresh;}
	inline float GetQuotThresh(bool bNum){if (bNum) {return McGMParams.m_fQuotThreshNum;}else{return McGMParams.m_fQuotThreshDen;}}
	virtual void SetRecTBlur(float alpha){m_fRecAlpha=alpha;}
	virtual float GetRecTBlur(){return m_fRecAlpha;}

	
	
//Public Timings
	float m_fTimer[12];		//Timings

////////////////////////////////////////////////////////////////////////////////////
// Private
private:

//Sub-processes that make the model
	
		//aja made below two functions protected
	bool Process();
	bool ProcessOrthogonal();

	virtual bool TFilter(void);					//Do Temporal Filtering
	bool XFilter(void);							//Do Spatial Filtering/ Generate Basis Set
	bool Compute_Mask(void);					//Threshold Output with this Mask
	bool Compute_Taylor_Products(int th);		//McGM Taylor Products

		//aja new 
	bool Compute_Outer_Taylor_Quotients(int th);
		//end aja new
	bool Compute_Taylor_Quotients(int th);		//McGM Raw Vel Quotients
	
		//aja new function
	bool New_3D_Vectors(int th);
	bool Compute_Speed_Matrices(int th);		//McGM Matrices

		//aja new function
	bool Compute_Outer_Speed_Matrices(int th);

	bool Compute_Velocity(void);				//McGM Velocity Extraction
	
	bool AddBorders(int size);					//Zero Borders of Output
	bool ZeroAccumulators(void);				//End of Process cycle requires Zeroing Op
	void Make_Trig_Tables(int Nangles);			//Pre calculate Trig terms
	

	bool Make_Weight_Matrices(int nIntegration_Zone,bool bUnityWeights=false);	//Pre Calculate Taylor Weights
	//void TestDiv(AbstractImage *pImNum,AbstractImage *pImDen,float fNumThresh,float fDenThresh);  //Thresholded Division

//Private Data

	//Critical Parameters (changing these requires reinitialisation)
	McGM2005CriticalParameters McGMParams;

//Non-Critical Parameters (can be changed on the fly)
	float m_fRecAlpha;			//Recursive Alpha
	bool m_bDoMask;				//Do Temporal Masking of Results
	float m_fMaskThresh;		//Threshold output with Temporal Magnitude
	bool m_bBlurQuotients;		//After forming quotients, before taking matrix dets
	
	float *rad_angles,*sn,*cn,*n_cn,*n_sn;	//Trig Tables
	float m_fWeight_Matrix[20][20][20];
	
	bool m_bInitOK;				//Model Can't run unless Init is OK

//Private Images
	
		//aja
	list <AbstractImage*> m_FrmBuf;

	AbstractImage *m_ImTResponse[3];		//Temporally Filtered Images
	AbstractImage *m_ImTaylorProd[McGM2005_MAXANGLES][6];//Taylor Squares  X.X, X.T, X.Y, Y.Y, Y.T, T.T
	
		// aja, new version
	AbstractImage *m_ImTempTaylorProd[6];
	AbstractImage *m_ImTempNew;
	AbstractImage *m_ImTempNew2;
	AbstractImage *m_ImTempOuterProd[3][3];
	AbstractImage *m_ImTaylorOuterProd[McGM2005_MAXANGLES][6];
	AbstractImage *m_ImDimensionIndex;
		//end new version

	AbstractImage *m_ImTaylorQuot[McGM2005_MAXANGLES][6];//Taylor Quotients
	AbstractImage *m_ImMatrices[4][2][2];	//Matrices for Determinants
	AbstractImage *m_ImMSpeed[4][2];		//Speed Magnitudes
	AbstractImage *m_ImOpFlow[2];			//Optical Flow.  Speed + Direction
	
	AbstractImage *m_ImTempFP;				//Tempory FP Image
	AbstractImage *m_ImMask;				//Masks Output
	AbstractImage *m_ImDetIms[8];			//Tempory Images used in ComputeVelocity
	AbstractFilter *m_TFilters[3];			//T Filters
	CSteerBasis *m_Basis[McGM2005_MAXANGLES][3];	//Spatial Filter Basis Set (one for each T Filter)
};


////////////////////////////////////////////////////////////////////////////
// COLOUR WHEEL_____________________________________________________________
////////////////////////////////////////////////////////////////////////////
// Maps angular data to a colour image representation (e.g. AbstractImage)
//
struct ColwheelRGB
{
	unsigned char r,g,b;
};

class ColWheelLUT
{
public:
	
	ColWheelLUT();
		// invert cols swaps between aj (false) & j.zanker's (true) colour wheel mode
		// -1 or 1
	ColWheelLUT(unsigned int lookuptablesize, bool invertcols=false);
	ColWheelLUT(const ColWheelLUT &cw);
	~ColWheelLUT();
	ColWheelLUT &operator=(const ColWheelLUT &cw);

	bool Init(unsigned int lookuptablesize, bool invertcols=false);

		//returns false if look up table hasn't been initialised
	bool RadToRGB(AbstractImage &dstRgbUchar,const AbstractImage &srcF,unsigned int borderSz) const;
	bool RadToRGB(	AbstractImage &dstRgbUchar,const AbstractImage &srcF,
					const AbstractImage &testF,float fThresh,
					unsigned int borderSz) const;

		//TODO, stylin' should swap order of src and dst args in the below to preserve consistency

	bool RadToRGB(float rad,unsigned char &r,unsigned char &g,unsigned char &b) const;
	bool RadToRGB(const float *pSrcRadf,unsigned char *pDstRgbUchar,unsigned int size) const;
	bool RadToRGB(const double *pSrcRadd,unsigned char *pDstRgbUchar,unsigned int size) const;
	bool RadToRGB(	const float *pSrcRadf,unsigned char *pDstRgbUchar,
					const float *pTestf,unsigned int size,float fThresh) const;
	
	
		//inverse mapping, colour back to angle doesn't use a look up table at the moment
	bool RGBtoRad(AbstractImage &dstF,const AbstractImage &srcRgbUchar) const;
	bool RGBtoRad(const unsigned char *pSrcRgb,float *pDstRadf,unsigned int size) const;
	

		//adds colour wheel border to image
	bool AddBorder(	unsigned char *pDstRgbUChar,unsigned int w,unsigned int h,
					unsigned int borderSz) const;

private:

	ColwheelRGB RadToColour(float rad) const;
	ColwheelRGB RadToRGB(float rad) const;
	float ColourToRad(const ColwheelRGB &col) const;

	ColwheelRGB *RadToRGBLUT_p;
	unsigned int LUTsize;
};



////////////////////////////////////////////////////////////////////////////
// SPACE VARIANT MAPPINGS__________________________________________________________
////////////////////////////////////////////////////////////////////////////
//
// supports log polar transforms and AJohnston's Conic model



////////////////////////////////////////////////////////////////////////////
// LOGPOLAR MAGNIFICATION_____________________________________________________________
////////////////////////////////////////////////////////////////////////////
//
//
//	aja 2005

class LogPolarMap{

public:

	struct Coordinate {
		float x; float y;
	};

	struct LogPolarMapHdr {
		unsigned int srcW,srcH,dstW,dstH;
		float alpha,beta;
	};

	LogPolarMap();
	LogPolarMap(float alpha,float beta);
	LogPolarMap(	float alpha,float beta,
				unsigned int dstWidth,unsigned int dstHeight,
				unsigned int srcWidth,unsigned int srcHeight
				);
	LogPolarMap(const LogPolarMap &src);
	LogPolarMap &operator=(const LogPolarMap &src);
	virtual ~LogPolarMap();
	
		//create,copy and destroy
	bool Create(	float alpha,float beta,
					unsigned int dstWidth,unsigned int dstHeight,
					unsigned int srcWidth,unsigned int srcHeight
				);
				
	bool Copy(const LogPolarMap &src);
	bool Destroy();

		//magnify image, uses bilinear interpolation for rescaling
	bool Process(	AbstractImage &dst,
					const AbstractImage &src);
	bool Process2(	AbstractImage &dst,
					const AbstractImage &src);

		//access maps
	bool ToFArraysMap(float *x,float *y,unsigned int sz) const;
	bool ToFArraysMap2(float *x,float *y,unsigned int sz) const;


	private:

		bool AllocMaps();

			// to do:
			// assuming symmetry of image only really need to calculate 1/4 vals
			// and reflect
		void CalcMaps();
		bool Magnify(AbstractImage &dst,const AbstractImage &src) const;
		bool Magnify2(AbstractImage &dst,const AbstractImage &src) const;

		LogPolarMapHdr m_hdr;
		Coordinate *m_pSpatialMap;
		Coordinate *m_pSpatialMap2;

};

// Implementation of the log(z+a) model

class LogPolarMap_ZPlusA{

public:

	struct Coordinate {
		float x; float y;
	};

	struct LogPolarMap_ZPlusAHdr {
		unsigned int srcW,srcH,dstW,dstH;
		float a;
	};

	LogPolarMap_ZPlusA();
	LogPolarMap_ZPlusA(float a);
	LogPolarMap_ZPlusA(float a,
				unsigned int dstWidth,unsigned int dstHeight,
				unsigned int srcWidth,unsigned int srcHeight
				);
	LogPolarMap_ZPlusA(const LogPolarMap_ZPlusA &src);
	LogPolarMap_ZPlusA &operator=(const LogPolarMap_ZPlusA &src);
	virtual ~LogPolarMap_ZPlusA();
	
		//create,copy and destroy
	bool Create(	float a,
					unsigned int dstWidth,unsigned int dstHeight,
					unsigned int srcWidth,unsigned int srcHeight
				);
				
	bool Copy(const LogPolarMap_ZPlusA &src);
	bool Destroy();

		//magnify image, uses bilinear interpolation for rescaling
	bool Process(	AbstractImage &dst,
					const AbstractImage &src);
	bool ProcessButterfly(	AbstractImage &dst,
							const AbstractImage &src);

		//access maps
	bool ToFArraysMap(float *x,float *y,unsigned int sz) const;
	bool ToFArraysButterflyMap(float *x,float *y,unsigned int sz) const;

	inline float *ScaleMap() const {return m_pScaleMap;};

	private:

		bool AllocMaps();

			// to do:
			// assuming symmetry of image only really need to calculate 1/4 vals
			// and reflect
		void CalcMaps();
		bool Magnify(AbstractImage &dst,const AbstractImage &src) const;
		bool MagnifyButterfly(AbstractImage &dst,const AbstractImage &src) const;

		LogPolarMap_ZPlusAHdr m_hdr;
		Coordinate *m_pSpatialMap;
		Coordinate *m_pSpatialButterflyMap;
		float *m_pScaleMap;
};



////////////////////////////////////////////////////////////////////////////
// CONIC MAGNIFICATION_____________________________________________________________
////////////////////////////////////////////////////////////////////////////
//
//
//	aja 2005

class ConicMap2D
{
public:

	struct Coordinate {
		float x; float y;
	};

	struct ConicMap2DHdr {
		unsigned int srcW,srcH,dstW,dstH;

			// conic parameter
		float apexAng;

			//max eccentricity (measured vertically) 
			//TODO change to max eccentricity in x or y dir, whichever is largest
		float srcMaxYEcc;
			//max eccentricity in x or y direction, whichever is largest
		float dstMaxEcc;
	};

		//constructors and destructors
	ConicMap2D();
	ConicMap2D(	float apexAngle_rad);
	ConicMap2D(	float apexAngle_rad,
				unsigned int dstWidth, unsigned int dstHeight,
				float dstMaxEccentricity_rad,
				unsigned int srcWidth, unsigned int srcHeight,
				float srcMaxYEccentricity_rad
				);
	ConicMap2D(const ConicMap2D &src);
	ConicMap2D &operator=(const ConicMap2D &src);

	virtual ~ConicMap2D();
	
		//create,copy and destroy

	bool Create(	float apexAngle_rad,
					unsigned int dstWidth, unsigned int dstHeight,
					float dstMaxEccentricity_rad,
					unsigned int srcWidth, unsigned int srcHeight,
					float srcMaxYEccentricity_rad
					);
	bool Copy(const ConicMap2D &src);
	bool Destroy();

		//magnify image, uses bilinear interpolation for rescaling
	bool Process(	AbstractImage &dst,
					float dstMaxEccentricity_rad,
					const AbstractImage &src, 
					float srcMaxYEccentricity_rad);

		//access maps
	bool ToFArrays(float *x,float *y,unsigned int sz) const;
	inline float *ScaleMap() const {return m_pScaleMap;};
	inline float *MagnificationMap() const {return m_pMagnifMap;};;
	
		//conic equation
	float ConicEqu_Ecc(float s) const;
	float ConicEqu_Dist(float ecc) const;
	
private:

	bool AllocMaps();

		// to do:
		// assuming symmetry of image only really need to calculate 1/4 vals
		// and reflect
	void CalcMaps();
	bool Magnify(AbstractImage &dst,const AbstractImage &src) const;
	
	ConicMap2DHdr m_hdr;
	
	Coordinate *m_pSpatialMap;
	float *m_pScaleMap;
	float *m_pMagnifMap;
};


////////////////////////////////////////////////////////////////////////////
// HOLBEIN PERSPECTIVE MAP___________________________________________________
// Mapping of the ilk of that in Holbein's painting the ambassadors	
////////////////////////////////////////////////////////////////////////////
//
//
//	aja 2005

class HolbeinMap
{
public:

	struct Coordinate {
		float x; float y;
	};

	struct HolbeinMapHdr {
		unsigned int srcW,srcH,dstW,dstH;

			// conic parameter
		float apexAng;

			//max eccentricity (measured vertically) 
			//TODO change to max eccentricity in x or y dir, whichever is largest
		float srcMaxYEcc;
			//max eccentricity in x or y direction, whichever is largest
		float dstMaxEcc;
	};

		//constructors and destructors
	HolbeinMap();
	HolbeinMap(	float apexAngle_rad);
	HolbeinMap(	float apexAngle_rad,
				unsigned int dstWidth, unsigned int dstHeight,
				float dstMaxEccentricity_rad,
				unsigned int srcWidth, unsigned int srcHeight,
				float srcMaxYEccentricity_rad
				);
	HolbeinMap(const HolbeinMap &src);
	HolbeinMap &operator=(const HolbeinMap &src);

	virtual ~HolbeinMap();
	
		//create,copy and destroy

	bool Create(	float apexAngle_rad,
					unsigned int dstWidth, unsigned int dstHeight,
					float dstMaxEccentricity_rad,
					unsigned int srcWidth, unsigned int srcHeight,
					float srcMaxYEccentricity_rad
					);
	bool Copy(const HolbeinMap &src);
	bool Destroy();

		//magnify image, uses bilinear interpolation for rescaling
	bool Process(	AbstractImage &dst,
					float dstMaxEccentricity_rad,
					const AbstractImage &src, 
					float srcMaxYEccentricity_rad);

		//access maps
	bool ToFArrays(float *x,float *y,unsigned int sz) const;
	inline float *ScaleMap() const {return m_pScaleMap;};
	inline float *MagnificationMap() const {return m_pMagnifMap;};;
	
		//conic equation
	float ConicEqu_Ecc(float s) const;
	float ConicEqu_Dist(float ecc) const;
	
private:

	bool AllocMaps();

		// to do:
		// assuming symmetry of image only really need to calculate 1/4 vals
		// and reflect
	void CalcMaps();
	bool Magnify(AbstractImage &dst,const AbstractImage &src) const;
	
	HolbeinMapHdr m_hdr;
	
	Coordinate *m_pSpatialMap;
	float *m_pScaleMap;
	float *m_pMagnifMap;
};

////////////////////////////////////////////////////////////////////////////
// VECTOR FIELD_____________________________________________________________
////////////////////////////////////////////////////////////////////////////
//
//
// aja 2005
//  Declarations for VectField2D class, a data structure containing a 2D vector
//  field (represented in polar/cartesian coordinates). 
//  IO as PCM image files (similar format to pgm file) supported

class VectField2D {

		//struct used to store vector in cartesian and polar representation,
		//in polar rep, x1=angle and x2=dist
	struct Vect2D{ 
		float x1,x2;
	};

	struct VectField2DHdr{
		unsigned int w,h,sz;
		float max;
	};

	public:
		VectField2DHdr hdr;

		Vect2D *field; 
		VectField2D();
		VectField2D(int w,int h);
		VectField2D(const VectField2D &c);
		VectField2D	&operator=(const VectField2D &c);
		~VectField2D();

		virtual void Destroy();
		bool SameType(const VectField2D &c);
		bool Init(unsigned int w,unsigned int h);
		bool Copy(const VectField2D &c);

		bool ReadBin(const char *fnm);
		bool ReadBin(FILE *_f);
		bool WriteBin(const char *fnm);
		bool WriteBin(FILE *_f);

		bool ReadPCM(const char *fnm);
		bool ReadPCM(FILE *_f);
		bool WritePCM(const char *fnm);
		bool WritePCM(FILE *_f);

		//BEWARE SOME OF THIS CODE IS UNTESTED
		bool ToPolar(VectField2D &polar);
		bool ThisToPolar();
		bool ToPolar(float *polarcoords,unsigned int w,unsigned int h);
		bool ToPolar(float *angs,float *dists,unsigned int w, unsigned int h);

		bool ToCartesian(VectField2D &cartes);
		bool ThisToCartesian();
		bool ToCartesian(float *cartescoords,unsigned int w,unsigned int h);
		bool ToCartesian(float *x,float *y,unsigned int w,unsigned int h);

		bool PolarDifference(VectField2D &polardiff,const VectField2D &target);
		bool CartesianDifference(VectField2D &cartesdiff,const VectField2D &target);
			
			//the absolute value of the above
		bool PolarError(VectField2D &polarerror,const VectField2D &target);
		bool CartesianError(VectField2D &carteserror,const VectField2D &target);
		
		bool FlipVert();

   //private:
		bool isPolar;
		
		void CalcMax();
};



////////////////////////////////////////////////////////////////////////////
// ImgPixelStats_____________________________________________________________
////////////////////////////////////////////////////////////////////////////
//
// Descriptive statistics for each image element
// Calls to process accumulate sum and sum of squares for each pixel.
// Call to Calc stats calculates the mean and standard deviation for each pixel in the image

class ImgPixelStats {

	struct Stats { 
		float mean,stdev;
	};

	struct RunningTotals{
		float sum,sumofsquares;
	};

		//size=w*h, store recalculation every loop
		//n is the number of images processed
		//channels,is the number of colours in the image
	struct ImgPixelStatsHdr{
		unsigned int w,h,sz,n,channels;
	};

	public:
		ImgPixelStatsHdr hdr;

		Stats *imStats;
		
		ImgPixelStats();
		ImgPixelStats(unsigned int w,unsigned int h,unsigned int channels);
		ImgPixelStats(const ImgPixelStats &c);
		ImgPixelStats &operator=(const ImgPixelStats &c);
		~ImgPixelStats();

		void Destroy();
		bool SameType(const ImgPixelStats &c);
		bool Init(unsigned int w,unsigned int h,unsigned int channels);
		bool Copy(const ImgPixelStats &c);

		bool ReadBin(const char *fnm);
		bool ReadBin(FILE *_f);
			//writes header,stats and running totals (in that order) in binary format
		bool WriteBin(const char *fnm);
		bool WriteBin(FILE *_f);

		bool Process(	unsigned char *img,
						unsigned int w,unsigned int h,unsigned int channels);
		bool Process(	float *img,
						unsigned int w,unsigned int h,unsigned int channels);
		bool ToFArrays(float *mean,float *stdev,unsigned int sz) const;

		bool CalcStats();

   private:
		RunningTotals *imRunningTotals;
};



#endif //#ifndef _UCLVisionResearchLabLib_H_INCLUDED_2005_
