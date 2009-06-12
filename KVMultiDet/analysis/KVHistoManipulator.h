/*
$Id: KVHistoManipulator.h,v 1.8 2009/04/07 14:54:15 ebonnet Exp $
$Revision: 1.8 $
$Date: 2009/04/07 14:54:15 $
*/

//Created by KVClassFactory on Thu Oct 18 11:48:18 2007
//Author: bonnet

#ifndef __KVHISTOMANIPULATOR_H
#define __KVHISTOMANIPULATOR_H

#include "Riostream.h"

#include "TH1.h"
#include "TH2.h"
#include "TCutG.h"
#include "KVList.h"
#include "TString.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TMath.h"

class KVHistoManipulator
{
   public:
	
	void init(void){
		
	}

	KVHistoManipulator();
	virtual ~KVHistoManipulator(void);
	
   Int_t CutStatBin(TH1 *hh,Int_t stat_min=-1,Int_t stat_max=-1);
	
	Int_t Apply_TCutG(TH2 *hh,TCutG *cut,TString mode="in");
	
	TH1* 	ScaleHisto(TH1* hh,TF1* fx,TF1* fy=NULL,Int_t nx=-1,Int_t ny=-1,Double_t xmin=-1.,Double_t xmax=-1.,Double_t ymin=-1.,Double_t ymax=-1.);
	TGraph* ScaleGraph(TGraph *hh,TF1*fx,TF1*fy);
	
	TH1* 	CentreeReduite(TH1* hh,Int_t nx=-1,Int_t ny=-1,Double_t xmin=-1.,Double_t xmax=-1.,Double_t ymin=-1.,Double_t ymax=-1.);
	TH2* 	CentreeReduiteX(TH2* hh,Int_t nx=-1,Double_t xmin=-1.,Double_t xmax=-1.);
	TH2* 	CentreeReduiteY(TH2* hh,Int_t ny=-1,Double_t ymin=-1.,Double_t ymax=-1.);

	TH2* 	RenormaliseHisto(TH2 *hh,Int_t bmin=-1,Int_t bmax=-1,TString axis="X",Double_t valref=1);
	TH2* 	RenormaliseHisto(TH2 *hh,Double_t valmin,Double_t valmax,TString axis="X",Double_t valref=1);
	
	TH1*	CumulatedHisto(TH1* hh,TString direction="C",Int_t bmin=-1,Int_t bmax=-1);
	TH1* 	GetDerivative(TH1* hh,Int_t order);
	
	TGraphErrors* GetMomentEvolution(TH2 *hh,TString momentx,TString momenty,TString axis="Y");
	Double_t GetCorrelationFactor(TH2* hh);
	TGraph* LinkGraphs(TGraph* grx,TGraph* gry);
	
	KVList* Give_ProjectionList(TH2 *hh,TString axis="X");

	void DefinePattern(TH1* ob,TString titleX="22 0.08 0.8",TString titleY="22 0.07 1.2",TString labelX="22 0.05 0.005",TString labelY="22 0.05 0.006");
	void DefinePattern(TGraph* ob,TString titleX="22 0.08 0.8",TString titleY="22 0.07 1.2",TString labelX="22 0.05 0.005",TString labelY="22 0.05 0.006");
	void DefinePattern(TF1* ob,TString titleX="22 0.08 0.8",TString titleY="22 0.07 1.2",TString labelX="22 0.05 0.005",TString labelY="22 0.05 0.006");
	void DefinePattern(TAxis* ax,TString title="22 0.08 0.8",TString label="22 0.05 0.005");

	void DefineLineStyle(TAttLine* ob,TString line);
	void DefineMarkerStyle(TAttMarker* ob,TString marker);
	void DefineStyle(TObject* ob,TString line,TString marker);
	
	void DefineTitle(TH1* ob,TString xtit,TString ytit);
	void DefineTitle(TGraph* ob,TString xtit,TString ytit);
	void DefineTitle(TF1* ob,TString xtit,TString ytit);
	
	
	ClassDef(KVHistoManipulator,1)//Propose differentes operations sur les histo
};

//................  global variable
R__EXTERN KVHistoManipulator *gHistoManipulator;

#endif