/*
$Id: KVClassFactory.cpp,v 1.19 2009/01/21 08:04:20 franklan Exp $
$Revision: 1.19 $
$Date: 2009/01/21 08:04:20 $
*/

//Created by KVClassFactory on Fri Jun 12 11:34:53 2009
//Author: Abdelouahao Chbihi

#include "KVIDSiCsIVamos.h"
#include "KVINDRAReconNuc.h"
#include "KVIDGridManager.h"

ClassImp(KVIDSiCsIVamos)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVIDSiCsIVamos</h2>
<h4>Identification map SIE_xx-CSIyy of Vamos</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVIDSiCsIVamos::KVIDSiCsIVamos()
{
   // Default constructor
   fSi = 0;
   fCsI = 0;
}

KVIDSiCsIVamos::~KVIDSiCsIVamos()
{
   // Destructor
}

void KVIDSiCsIVamos::Initialize()
{
   // Initialize telescope for current run.
   // Pointers to grids for run are set, and if there is at least 1 (GG) grid,
   // we set IsReadyForID = kTRUE

	fSi = (KVSiliconVamos*)GetDetector(1);
	fCsI = (KVCsIVamos*)GetDetector(2);
	fgrid=0;
	TIter next(fIDGrids); KVIDGrid*grid=0;
	while( (grid=(KVIDGrid*)next()) ){
        if( !strcmp(grid->GetVarY(),"SI") ) fgrid = (KVIDZGrid*)grid;
	}
   if( fgrid ){
      SetBit(kReadyForID);
      fgrid->Initialize();
   }
   else
		ResetBit(kReadyForID);
}

const Char_t *KVIDSiCsIVamos::GetName() const
{
	// don t give any name, adapted for detectors of Vamos FP
	TString nom;
	nom = GetDetector(1)->GetName();
	nom+="_";
	nom+= GetDetector(2)->GetName();
	const_cast<KVIDSiCsIVamos*>(this)->SetName(nom.Data());
	return TNamed::GetName();
}

Double_t KVIDSiCsIVamos::GetIDMapX(Option_t * opt)
{
	return (Double_t) fCsI->GetACQPar()->GetData();
}

Double_t KVIDSiCsIVamos::GetIDMapY(Option_t * opt)
{
	return (Double_t) fSi->GetEnergy();
}

Bool_t KVIDSiCsIVamos::Identify(KVReconstructedNucleus * nuc)
{
   //Particle identification and code setting using identification grids.

      KVINDRAReconNuc *irnuc = (KVINDRAReconNuc *) nuc;

      //perform identification in Si - CsI map, in Vamos FP

      Double_t si = GetIDMapY();
      Double_t csi = GetIDMapX();

      KVIDGrid* theIdentifyingGrid = 0;

      fgrid->Identify(csi, si, irnuc);
      theIdentifyingGrid =(KVIDGrid*)fgrid;

      //set subcode in particle
      SetIDSubCode(irnuc->GetCodes().GetSubCodes(), theIdentifyingGrid->GetQualityCode());

		if(theIdentifyingGrid->GetQualityCode() == KVIDZGrid::kICODE8){
			// only if the final quality code is kICODE8 do we consider that it is
			// worthwhile looking elsewhere. In all other cases, the particle has been
			// "identified", even if we still don't know its Z and/or A (in this case
			// we consider that we have established that they are unknowable).
			return kFALSE;
		}

		if(theIdentifyingGrid->GetQualityCode() == KVIDZGrid::kICODE7){
			// if the final quality code is kICODE7 (above last line in grid) then the estimated
			// Z is only a minimum value (Zmin)
			irnuc->SetIDCode( kIDCode5 );
			return kTRUE;
		}


		// set general ID code SiLi-CsI
      irnuc->SetIDCode( kIDCode3 );
      return kTRUE;
}

