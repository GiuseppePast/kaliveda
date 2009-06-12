/***************************************************************************
$Id: KVCsI.h,v 1.26 2009/04/09 09:25:43 ebonnet Exp $
                          kvcsi.h  -  description
                             -------------------
    begin                : Fri Oct 4 2002
    copyright            : (C) 2002 by A. Mignon & J.D. Frankland
    email                : frankland@ganil.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KVCSI_H
#define KVCSI_H

//status values for calculation of total light output
#define NOT_CALCULATED 0
#define CALCULATED_WITH_GAIN_CORRECTION 1
#define NO_GAIN_CORRECTION 4
#define NEGATIVE_PEDESTAL_CORRECTED_VALUE 15
#define CALCULATION_NOT_CONVERGED 12

#include "KVDetector.h"
#include "KVACQParam.h"
#include "KVINDRA.h"

class KVChIo;
class KVTelescope;
class KVLightEnergyCsI;

class KVCsI:public KVDetector {

   Double_t fLumiereTotale;     //total light output calculated from R and L components
   UInt_t fLumTotStatus;        //status of light calculation
   Double_t fA1, fA2, fA3;

   KVLightEnergyCsI* fCalZ1; //! light-energy calibration for Z=1
   KVLightEnergyCsI* fCal; //! light-energy calibration for Z>1

	Char_t  fPinLaser;//number of pin laser used to control stability of crystal
	Double_t fGainCorrection;//gain correction for total light output. by default equal to 1.0.

	KVACQParam* fACQ_R;//'Rapide' acquisition parameter
	KVACQParam* fACQ_L;//'Lente' acquisition parameter

   Double_t Calculate(UShort_t mode, Double_t rapide =
                      -1.0, Double_t lente = -1.0);

   enum { kLumiere, kTau };

 protected:
	void init();

 public:
    KVCsI();
    KVCsI(Float_t thick);
    virtual ~ KVCsI();

   KVChIo *GetChIo() const;

   Float_t GetR() {
      return fACQ_R->GetData();
   }
   Float_t GetL() {
      return fACQ_L->GetData();
   }
   UShort_t GetMT() {
      return GetACQParam("T")->GetCoderData();
   }

   Double_t GetLumiereTotale(Double_t rapide = -1.0, Double_t lente =
                             -1.0);
   Double_t GetCorrectedLumiereTotale(Double_t rapide = -1.0, Double_t lente =
                             -1.0);
   Double_t GetTauZero(Double_t rapide = -1.0, Double_t lente = -1.0);
   void SetLumiereTotale(Double_t lum) {
      fLumiereTotale = lum;
   };
   UInt_t GetStatusLumiere();
   Bool_t LightIsGood();
   virtual void Clear(Option_t * opt = "");
   virtual void Print(Option_t * option = "") const;

   void SetACQParams();
   void SetCalibrators();

   Double_t GetCorrectedEnergy(UInt_t Z, UInt_t A, Double_t lum = -1., Bool_t transmission=kTRUE);
   Double_t GetLightFromEnergy(UInt_t Z, UInt_t A, Double_t E = -1.);

	void SetPinLaser(Int_t n){ if(n>0&&n<255) fPinLaser = (Char_t)n; };
	Int_t GetPinLaser()
	{
		// Returns number of Pin Laser used to control stability of CsI detector.
		return (Int_t)fPinLaser;
	};
	void SetTotalLightGainCorrection(Double_t c)
	{
	    fGainCorrection = c;
	};
	Double_t GetTotalLightGainCorrection() const
	{
	    return fGainCorrection;
	};

   ClassDef(KVCsI, 4)           // The CsI(Tl) detectors of the INDRA array
};

#endif