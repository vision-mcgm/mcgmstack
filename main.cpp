
// Visual C++ 2008, Need to add: WIN32 to: 
// Project/Properties/Configuration Properties/C/C++/Preprocessor/ -> Preprocessor definitions


	#include <windows.h>
	#include <wingdi.h>
//#endif /* WIN32 */

#define _CRT_SECURE_NO_WARNINGS


#include "UCLVisionResearchLabLib.h"
#include "CStopWatch.h"

#include <string>
#include <iostream>
#include <stdio.h>


#include <gl\gl.h>			
#include <gl\glu.h>		

#include <fDebug.h>		


#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
//#pragma comment(lib, "glaux.lib")


int main()
{
	fdInit();
	fdWriteFile();

	CStopWatch sw;
	ImgSeq inputSeq;
	//ImgSeq inputSeq,iplInputSeq;
	_Img templateImg;
	//CIplImg iplTemplateImg;

	sw.Start();
	//inputSeq.ReadBMP("../OpticFlowTestSeq/VocalTract/vTract",1,100,1,5,templateImg);
	//inputSeq.ReadBMP("../OpticFlowTestSeq/EttlingetTorTraffic/bad_",0,100,1,4,templateImg);
	//inputSeq.ReadBMP("../OpticFlowTestSeq/Expansion_BlurSlant/sVblur",0,40,1,4,templateImg);
	//inputSeq.ReadBMP("../OpticFlowTestSeq/Contraction/Flow",0,100,1,5,templateImg);
	inputSeq.ReadBMP("D:/fintanData/AndyMCGMStandalone_fsnRework/AJAStandaloneFSNREwork/Debug/gratingsBmp/",0,100,1,4,templateImg);
	cout<<"Read in "<<sw.Read()<<endl;

	if(!inputSeq.AllSameType()){
		fprintf(stderr,"main(), Input seq empty or not all same type\n");
		return false;
	}
	
	if(inputSeq.Size()>0){
		//iplTemplateImg.Create(inputSeq[0].Width(),inputSeq[0].Height(),1,IMG_FLOAT);
		templateImg.Create(inputSeq[0].Width(),inputSeq[0].Height(),1,IMG_FLOAT);
	}
	else{
		fprintf(stderr,"main(), Input seq empty\n");
		return false;
	}

	ImgSeq greyStimSeq;
	//iplInputSeq.Convert(inputSeq,iplTemplateImg);
	greyStimSeq.Convert(inputSeq,templateImg);
	cout<<"Converted input"<<sw.Read()<<endl;

	sw.Start();
	
	McGM2005 mcgm;
	//CIplFilter iplTemplateFilter;
	_Filter templateFilter;
	

//#define _IPL_

#ifdef _IPL_
	mcgm.ReadCriticalParameters("Params_McGM.txt",iplTemplateFilter);
#else
	mcgm.ReadCriticalParameters("Params_McGM.txt",templateFilter);
#endif

	mcgm.SetDoMask(true);
	mcgm.SetMaskThresh(0.04f);
	//mcgm.SetMaskThresh(10.0f);
	mcgm.SetDoQuotientBlur(true);

	_Img ang,speed,colang;
	ColWheelLUT cw;

	cw.Init(255,true);

	//_Img tFilterOut;
	//tFilterOut.Create(768,256,1,IMG_FLOAT);

	float divtwofivefive=1/255.0f;
	// hack aja 2009 ConicMap2D cm=ConicMap2D();

#ifdef _IPL_
	for(int i=0;i<iplInputSeq.Size();++i)
	{
		iplInputSeq.Seq()[i]->MultiplyS(&divtwofivefive);
		// hack aja 2009 cm.Process(*iplInputSeq.Seq()[i],0.5,iplInputSeq[i],0.5);
		if(mcgm.Process(iplInputSeq[i]))
		{	
			//tFilterOut.PasteTile(1,3,(const AbstractImage**)mcgm.TResponses());
			//tFilterOut.ScaleF(0,255);
			//tFilterOut.WriteBMP("tFilter.bmp");

			mcgm.MaskedOpFlow(ang,speed,0,1.5,255);
			cw.RadToRGB(colang,ang,*mcgm.Mask(),mcgm.GetMaskThresh(),11);
			colang.WriteBMP("OpFlowAngRGB.bmp");
			//for some reason this scaling causes colour wheel representations to skew-wiff
			//ang.ScaleF(0,255);
			//ang.WriteBMP("OpFlowAng.bmp");

			/*
			for(int k=0;k<mcgm.OpFlow()[0]->Width()*mcgm.OpFlow()[0]->Height();++k){
				if(((float*)mcgm.OpFlow()[0]->Data())[k]>1.5)
					((float*)mcgm.OpFlow()[0]->Data())[k]=1.5;
			}
			*/

			//mcgm.OpFlow()[0]->ScaleF(0,255);
			float twofivefive=255;
			mcgm.OpFlow()[0]->MultiplyS(&twofivefive);
			mcgm.OpFlow()[1]->ScaleF(0,255);
			mcgm.OpFlow()[0]->WriteBMP("OpFlowSpeed.bmp");
			mcgm.OpFlow()[1]->WriteBMP("OpFlowAngle.bmp");
			
			speed.WriteBMP("OpFlowSpeed1.bmp");
		}
		
		cout <<"Processed "<<i<<endl;
	}
#else
	for(int i=0;inputSeq.Size();++i)
	{
		//inputSeq.Seq()[i]->MultiplyS(&divtwofivefive);
		//if(mcgm.Process(inputSeq[i]))

		greyStimSeq.Seq()[i]->MultiplyS(&divtwofivefive);
		if(mcgm.Process(greyStimSeq[i]))
		{	
			//tFilterOut.PasteTile(1,3,(const AbstractImage**)mcgm.TResponses());
			//tFilterOut.ScaleF(0,255);
			//tFilterOut.WriteBMP("tFilter.bmp");

			greyStimSeq.Seq()[i]->WriteBMP("InputTest.bmp");
			mcgm.MaskedOpFlow(ang,speed,0,1.5,255);
			cw.RadToRGB(colang,ang,*mcgm.Mask(),mcgm.GetMaskThresh(),11);
			//cw.RadToRGB(colang,ang,11);/
			colang.WriteBMP("D:/fintanData/AndyMCGMStandalone_fsnRework/OpFlowAngRGB.bmp");
			speed.WriteBMP("D:/fintanData/AndyMCGMStandalone_fsnRework/OpFlowSpeed1.bmp");
			
		
			
			//mcgm.OpFlow()[0]->ScaleF(0,255);
			//mcgm.OpFlow()[1]->ScaleF(0,255);

			//mcgm.OpFlow()[0]->WriteBMP("OpFlowSpeed.bmp");
			//mcgm.OpFlow()[1]->WriteBMP("OpFlowAngle.bmp");
			

			float twofivefive=255;
			mcgm.OpFlow()[0]->MultiplyS(&twofivefive);
			mcgm.OpFlow()[1]->ScaleF(0,255);
			mcgm.OpFlow()[0]->WriteBMP("OpFlowSpeed.bmp");
			mcgm.OpFlow()[1]->WriteBMP("OpFlowAngle.bmp");
		}
		
		cout <<"Processed "<<i<<endl;
	}
#endif
	//cout <<sw.Read();
	
//	return 0;
}