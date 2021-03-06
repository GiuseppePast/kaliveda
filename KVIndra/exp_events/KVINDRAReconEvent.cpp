/***************************************************************************
                          kvindrareconevent.cpp  -  description
                             -------------------
    begin                : Thu Oct 10 2002
    copyright            : (C) 2002 by J.D. Frankland
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

#include "Riostream.h"
#include "TROOT.h"
#include "KVINDRAReconEvent.h"
#include "KVList.h"
#include "KVGroup.h"
#include "KVTelescope.h"
#include "KVDetector.h"
#include "KVCsI.h"
#include "KVSilicon.h"
#include "KVDetectorEvent.h"
#include "KVINDRAReconNuc.h"
#include "KVINDRA.h"
#include "TStreamerInfo.h"
#include "KVIDCsI.h"
#include "KVIDGCsI.h"
#include "KVDataSet.h"
#include "KVChIo.h"

using namespace std;

ClassImp(KVINDRAReconEvent);

///////////////////////////////////////////////////////////////////////////////
//Event reconstructed from energy losses in INDRA multidetector.
//By default this contains a TClonesArray of KVINDRAReconNuc objects.
//
//Random vs. mean angles
//----------------------
//By default reconstructed particles have randomised angles within the
//aperture of the detector where they were stopped. To use the mean angles
//of the detectors, call UseMeanAngles().
//
//"OK" & not "OK" particles
//---------------------------
//The "OK" status of particles is defined by setting acceptable identification and calibration
//codes using AcceptIDCodes and AcceptECodes. The comparison of each particle's codes
//with the "acceptable" codes then determines whether KVParticle::IsOK() is set or not.
//
//Trigger Information
//--------------------
//Use EventTrigger() to access the KVINDRATriggerInfo object which stores the values
//of Selecteur registers STAT_EVE, R_DEC and CONFIG for this event.
///////////////////////////////////////////////////////////////////////////////

void KVINDRAReconEvent::init()
{
   //default initialisations
   fCodeMask = 0;
   fHitGroups = 0;
}


KVINDRAReconEvent::KVINDRAReconEvent(Int_t mult, const char* classname)
   : KVReconstructedEvent(mult, classname)
{
   init();
}

KVINDRAReconEvent::~KVINDRAReconEvent()
{
   if (fCodeMask) {
      delete fCodeMask;
      fCodeMask = 0;
   }
   SafeDelete(fHitGroups);
};

/////////////////////////////////////////////////////////////////////////////

KVINDRAReconNuc* KVINDRAReconEvent::GetParticle(Int_t npart) const
{
   //
   //Access to event member with index npart (1<=npart<=fMult)
   //

   return (KVINDRAReconNuc*)(KVReconstructedEvent::GetParticle(npart));
}

/////////////////////////////////////////////////////////////////////////////

KVINDRAReconNuc* KVINDRAReconEvent::AddParticle()
{
   //Wrapper for KVEvent::GetNextParticle casting result to KVINDRAReconNuc*

   KVINDRAReconNuc* tmp = (KVINDRAReconNuc*)(KVReconstructedEvent::AddParticle());
   return tmp;
}

/////////////////////////////////////////////////////////////////////////////
void KVINDRAReconEvent::Streamer(TBuffer& R__b)
{
   //Stream an object of class KVINDRAReconEvent.
   //We loop over the newly-read particles in order to set their
   //IsOK() status by comparison with the event's code mask.

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(KVINDRAReconEvent::Class(), this);
      KVINDRAReconNuc* par;
      while ((par = GetNextParticle())) {
         par->SetIsOK(CheckCodes(par->GetCodes()));
      }
   } else {
      R__b.WriteClassBuffer(KVINDRAReconEvent::Class(), this);
   }
}

//______________________________________________________________________________
void KVINDRAReconEvent::Print(Option_t* option) const
{
   //Print out list of particles in the event.
   //If option="ok" only particles with IsOK=kTRUE are included.

   cout << GetTitle() << endl;  //system
   cout << GetName() << endl;   //run
   cout << "Event number: " << GetNumber() << endl << endl;
   cout << "MULTIPLICITY = " << ((KVINDRAReconEvent*) this)->
        GetMult(option) << endl << endl;

   KVINDRAReconNuc* frag = 0;
   while ((frag = ((KVINDRAReconEvent*) this)->GetNextParticle(option))) {
      frag->Print();
   }
}

KVINDRAReconNuc* KVINDRAReconEvent::GetNextParticle(Option_t* opt)
{
   //Use this method to iterate over the list of particles in the event.
   //
   //If opt="ok" only the particles whose ID codes and E codes correspond to those set using AcceptIDCodes and
   //AcceptECodes will be returned, in the order in which they appear in the event.
   //
   //After the last particle in the event GetNextParticle() returns a null pointer and
   //resets itself ready for a new iteration over the particle list.
   //Therefore, to loop over all particles in an event, use a structure like:
   //
   //      KVINDRAReconNuc* p;
   //      while( (p = GetNextParticle() ){
   //              ... your code here ...
   //      }

   return (KVINDRAReconNuc*) KVEvent::GetNextParticle(opt);
}

//______________________________________________________________________________________________

Bool_t KVINDRAReconEvent::IsOK()
{
   //Returns kTRUE if event is acceptable for analysis.
   //This means that the multiplicity of particles with IsOK()=kTRUE (having good ID codes etc.)
   //is at least equal to the multiplicity trigger of the (experimental) event.

   if (gIndra) {
      if (gIndra->GetTrigger())
         return (GetMult("ok") >= gIndra->GetTrigger());
      else
         return (GetMult("ok") > 0);
   } else {
      return (KVEvent::IsOK());
   }
   return kTRUE;
}

//______________________________________________________________________________________________

void KVINDRAReconEvent::AcceptIDCodes(UShort_t code)
{
   //Define the identification codes that you want to include in your analysis.
   //Example: to keep only (VEDA) codes 2 and 3, use
   //        event.AcceptIDCodes( kIDCode2 | kIDCode3 );
   //Particles which have the correct ID codes will have IsOK() = kTRUE
   //i.e.  if( particle.IsOK() ) { // ... analysis ... }
   //although it is easier to loop over all 'correct' particles using:
   //   while ( particle = GetNextParticle("ok") ){ // ... analysis ... }
   //
   //To remove any previously defined acceptable identification codes, use AcceptIDCodes(0)
   //
   //N.B. : this method is preferable to using directly the KVINDRACodeMask pointer
   //as the 'IsOK' status of all particles of the current event are automatically updated
   //when the code mask is changed using this method.
   GetCodeMask()->SetIDMask(code);
   KVINDRAReconNuc* par = 0;
   while ((par = GetNextParticle())) {
      if (CheckCodes(par->GetCodes()))
         par->SetIsOK();
      else
         par->SetIsOK(kFALSE);
   }
}

//______________________________________________________________________________________________

void KVINDRAReconEvent::AcceptECodes(UChar_t code)
{
   //Define the calibration codes that you want to include in your analysis.
   //Example: to keep only ecodes 1 and 2, use
   //        event.AcceptECodes( kECode2 | kECode3 );
   //Particles which have the correct E codes will have IsOK() = kTRUE
   //i.e.  if( particle.IsOK() ) { // ... analysis ... }
   //although it is easier to loop over all 'correct' particles using:
   //   while ( particle = GetNextParticle("ok") ){ // ... analysis ... }
   //
   //N.B. : this method is preferable to using directly the KVINDRACodeMask pointer
   //as the 'IsOK' status of all particles of the current event are automatically updated
   //when the code mask is changed using this method.
   GetCodeMask()->SetEMask(code);
   KVINDRAReconNuc* par = 0;
   while ((par = GetNextParticle())) {
      if (CheckCodes(par->GetCodes()))
         par->SetIsOK();
      else
         par->SetIsOK(kFALSE);
   }
}

//______________________________________________________________________________________________

void KVINDRAReconEvent::ChangeFragmentMasses(UChar_t mass_formula)
{
   //Changes the mass formula used to calculate A from Z for all nuclei in event
   //For the values of mass_formula, see KVNucleus::GetAFromZ
   //
   //The fragment energy is modified in proportion to its mass, this is due to the
   //contribution from the CsI light-energy calibration:
   //
   //    E -> E + E_CsI*( newA/oldA - 1 )
   //
   //From an original lunch by Remi Bougault.
   //
   //Only particles with 'acceptable' ID & E codes stopping in (or passing through)
   //a CsI detector are affected; particles whose mass was measured
   //(i.e. having KVReconstructedNucleus::IsAMeasured()==kTRUE)
   //are not affected by the change of mass formula.

   ResetGetNextParticle();
   KVINDRAReconNuc* frag;
   while ((frag = (KVINDRAReconNuc*)GetNextParticle("ok"))) {

      if (!frag->IsAMeasured()) {

         Float_t oldA = (Float_t)frag->GetA();
         frag->SetMassFormula(mass_formula);

         if (frag->GetCsI()) {
            Float_t oldECsI = frag->GetEnergyCsI();
            Float_t oldE = frag->GetEnergy();
            Float_t newECsI = oldECsI * ((Float_t)frag->GetA() / oldA);
            frag->GetCsI()->SetEnergy(newECsI);
            frag->SetEnergy(oldE - oldECsI + newECsI);
         }
      }
   }

}

//____________________________________________________________________________

void KVINDRAReconEvent::IdentifyEvent()
{
   // Performs event identification (see KVReconstructedEvent::IdentifyEvent), and then
   // particles stopping in first member of a telescope (GetStatus() == KVReconstructedNucleus::kStatusStopFirstStage) are
   // labelled with VEDA ID code kIDCode5 (Zmin)
   //
   //   When CsI identification gives a gamma, we unset the 'analysed' state of all detectors
   // in front of the CsI and reanalyse the group in order to reconstruct and identify charged particles
   // stopping in them.
   //
   // Unidentified particles receive the general ID code for non-identified particles (kIDCode14)

   KVReconstructedEvent::IdentifyEvent();
   KVINDRAReconNuc* d = 0;
   int mult = GetMult();
   KVUniqueNameList gammaGroups;//list of groups with gammas identified in CsI
   ResetGetNextParticle();

   while ((d = GetNextParticle())) {
      if (d->IsIdentified() && d->GetStatus() == KVReconstructedNucleus::kStatusStopFirstStage) {
         d->SetIDCode(kIDCode5);   // Zmin
      } else if (d->IsIdentified() && d->GetCodes().TestIDCode(kIDCode0)) {
         // gamma identified in CsI
         // reset analysed state of all detectors in front of CsI
         if (d->GetCsI()) {
            if (d->GetCsI()->GetAlignedDetectors()) {
               TIter next(d->GetCsI()->GetAlignedDetectors());
               KVDetector* det = (KVDetector*)next(); //first detector = CsI
               while ((det = (KVDetector*)next())) det->SetAnalysed(kFALSE);
               gammaGroups.Add(d->GetGroup());
            } else {
               Error("IdentifyEvent", "particule id gamma, no aligned detectors???");
               d->Print();
            }
         } else {
            Error("IdentifyEvent", "particule identified as gamma, has no CsI!!");
            d->Print();
         }
      }
   }

   // perform secondary reconstruction in groups with detected gammas
   int ngamG = gammaGroups.GetEntries();
   if (ngamG) {
      for (int i = 0; i < ngamG; i++) {
         gIndra->AnalyseGroupAndReconstructEvent(this, (KVGroup*)gammaGroups.At(i));
      }
   }
   if (GetMult() > mult) {
      /*Info("IdentifyEvent", "Event#%d: Secondary reconstruction (gammas) -> %d new particles",
         GetNumber(), GetMult()-mult);*/

      // identify new particles generated in secondary reconstruction
      KVReconstructedEvent::IdentifyEvent();
      ResetGetNextParticle();

      while ((d = GetNextParticle())) {
         if (d->IsIdentified() && d->GetStatus() == KVReconstructedNucleus::kStatusStopFirstStage) {
            d->SetIDCode(kIDCode5);   // Zmin
         } else if (!d->IsIdentified()) {
            d->SetIDCode(kIDCode14);
         }
      }
      /*
      for(int i=mult+1; i<=GetMult(); i++){
         d = GetParticle(i);
         if(d->IsIdentified())
            printf("\t%2d: Ring %2d Module %2d Z=%2d  A=%3d  code=%d\n",i,d->GetRingNumber(),
                  d->GetModuleNumber(),d->GetZ(),d->GetA(),d->GetCodes().GetVedaIDCode());
         else
            printf("\t%2d: Ring %2d Module %2d UNIDENTIFIED status=%d\n", i,d->GetRingNumber(),
                  d->GetModuleNumber(), d->GetStatus());
      }
      */
   }
}

void KVINDRAReconEvent::SecondaryIdentCalib()
{
   // Perform identifications and calibrations of particles not included
   // in first round (methods IdentifyEvent() and CalibrateEvent()).
   //
   // Here we treat particles with GetStatus()==KVReconstructedNucleus::kStatusOKafterSub
   // after subtracting the energy losses of all previously calibrated particles in group from the
   // measured energy losses in the detectors they crossed.

   if (!fHitGroups)
      fHitGroups = new KVUniqueNameList;
   else
      fHitGroups->Clear();
   // build list of hit groups
   KVINDRAReconNuc* d;
   while ((d = GetNextParticle())) fHitGroups->Add(d->GetGroup());

   //loop over hit groups
   TIter next_grp(fHitGroups);
   KVGroup* grp;
   while ((grp = (KVGroup*)next_grp())) {
      SecondaryAnalyseGroup(grp);
   }

   // set "unidentified" code for any remaining unidentified particle
   ResetGetNextParticle();
   while ((d = GetNextParticle())) {
      if (!d->IsIdentified()) {
         d->SetIDCode(kIDCode14);
      }
   }
}

void KVINDRAReconEvent::SecondaryAnalyseGroup(KVGroup* grp)
{
   // Perform identifications and calibrations of particles not included
   // in first round (methods IdentifyEvent() and CalibrateEvent()).
   //
   // Here we treat particles with GetStatus()==KVReconstructedNucleus::kStatusOKafterSub
   // after subtracting the energy losses of all previously calibrated particles in group from the
   // measured energy losses in the detectors they crossed.

   // loop over al identified & calibrated particles in group and subtract calculated
   // energy losses from all detectors
   KVINDRAReconNuc* nuc;
   TList sixparts;
   TIter parts(grp->GetParticles());
   while ((nuc = (KVINDRAReconNuc*)parts())) {
      if (nuc->IsIdentified() && nuc->IsCalibrated()) {
         nuc->SubtractEnergyFromAllDetectors();
         // reconstruct particles from pile-up in silicon detectors revealed by coherency CsIR/L - SiCsI
         if (nuc->IsSiPileup() && nuc->GetSi()->GetEnergy() > 0.1) {
            KVINDRAReconNuc* SIX = AddParticle();
            SIX->Reconstruct(nuc->GetSi());
            sixparts.Add(SIX);
         }
         // reconstruct particles from pile-up in si75 detectors revealed by coherency
         if (nuc->IsSi75Pileup()) {
            KVINDRAReconNuc* SIX = AddParticle();
            SIX->Reconstruct(nuc->GetSi75());
            sixparts.Add(SIX);
         }
         // reconstruct particles from pile-up in sili detectors revealed by coherency
         if (nuc->IsSiLiPileup()) {
            KVINDRAReconNuc* SIX = AddParticle();
            SIX->Reconstruct(nuc->GetSiLi());
            sixparts.Add(SIX);
         }

         // reconstruct particles from pile-up in ChIo detectors revealed by coherency CsIR/L - ChIoCsI
         if (nuc->IsChIoPileup() && nuc->GetChIo()->GetEnergy() > 1.0) {
            KVINDRAReconNuc* SIX = AddParticle();
            SIX->Reconstruct(nuc->GetChIo());
            sixparts.Add(SIX);
         }
      }
   }
   // reanalyse group
   KVReconstructedNucleus::AnalyseParticlesInGroup(grp);

   Int_t nident = 0; //number of particles identified in each step
   if (sixparts.GetEntries()) { // identify any particles added by coherency CsIR/L - SiCsI
      KVINDRAReconNuc* SIX;
      TIter nextsix(&sixparts);
      while ((SIX = (KVINDRAReconNuc*)nextsix())) {
         if (SIX->GetStatus() == KVReconstructedNucleus::kStatusOK) {
            SIX->Identify();
            if (SIX->IsIdentified()) {
               nident++;
               if (SIX->GetCodes().TestIDCode(kIDCode5)) SIX->SetIDCode(kIDCode7);
               else SIX->SetIDCode(kIDCode6);
               SIX->Calibrate();
               if (SIX->IsCalibrated()) SIX->SubtractEnergyFromAllDetectors();
            } else {
               // failure of ChIo-Si identification: particle stopped in ChIo ?
               // estimation of Z (minimum) from energy loss (if detector is calibrated)
               UInt_t zmin = ((KVDetector*)SIX->GetDetectorList()->Last())->FindZmin(-1., SIX->GetMassFormula());
               if (zmin) {
                  SIX->SetZ(zmin);
                  SIX->SetIsIdentified();
                  SIX->SetIDCode(kIDCode7);
                  // "Identifying" telescope is taken from list of ID telescopes
                  // to which stopping detector belongs
                  SIX->SetIdentifyingTelescope((KVIDTelescope*)SIX->GetStoppingDetector()->GetIDTelescopes()->Last());
                  SIX->Calibrate();
               }
            }
         }
      }
   }
   if (nident) { // newly-identified particles may change status of others in group
      // reanalyse group
      KVReconstructedNucleus::AnalyseParticlesInGroup(grp);
      nident = 0;
   }

   TIter parts2(grp->GetParticles()); // list may have changed if we have added particles
   // identify & calibrate any remaining particles with status=KVReconstructedNucleus::kStatusOK
   while ((nuc = (KVINDRAReconNuc*)parts2())) {
      if (!nuc->IsIdentified() && nuc->GetStatus() == KVReconstructedNucleus::kStatusOK) {
         nuc->ResetNSegDet();
         nuc->Identify();
         if (nuc->IsIdentified()) {
            nident++;
            nuc->Calibrate();
            if (nuc->IsCalibrated()) nuc->SubtractEnergyFromAllDetectors();
         }
      }
   }
   if (nident) { // newly-identified particles may change status of others in group
      // reanalyse group
      KVReconstructedNucleus::AnalyseParticlesInGroup(grp);
      nident = 0;
   }

   // any kStatusOKafterShare particles ?
   TList shareChIo;
   parts2.Reset();
   while ((nuc = (KVINDRAReconNuc*)parts2())) {
      if (!nuc->IsIdentified() && nuc->GetStatus() == KVReconstructedNucleus::kStatusOKafterShare) {
         shareChIo.Add(nuc);
      }
   }
   Int_t nshares = shareChIo.GetEntries();
   if (nshares) {
      KVChIo* theChIo = ((KVINDRAReconNuc*)shareChIo.At(0))->GetChIo();
      if (theChIo && nshares > 1) {
         // divide chio energy equally
         Double_t Eshare = theChIo->GetEnergyLoss() / nshares;
         theChIo->SetEnergyLoss(Eshare);
         // modify PG and GG of ChIo according to new energy loss
         Double_t volts = theChIo->GetVoltsFromEnergy(Eshare);
         Double_t GG = theChIo->GetCanalGGFromVolts(volts);
         Double_t PG = theChIo->GetCanalPGFromVolts(volts);
         theChIo->GetACQParam("PG")->SetData(TMath::Min(4095, (Int_t)PG));
         theChIo->GetACQParam("GG")->SetData(TMath::Min(4095, (Int_t)GG));
      }
      // now try to identify
      TIter nextSh(&shareChIo);
      while ((nuc = (KVINDRAReconNuc*)nextSh())) {
         nuc->SetNSegDet(10);
         nuc->Identify();
         if (nuc->IsIdentified()) {
            nuc->SetIDCode(kIDCode8);
            nuc->Calibrate();
         }
      }
   }

   // any remaining stopped in first stage particles ?
   parts2.Reset();
   while ((nuc = (KVINDRAReconNuc*)parts2())) {
      if (!nuc->IsIdentified() && nuc->GetStatus() == KVReconstructedNucleus::kStatusStopFirstStage) {
         // estimation of Z (minimum) from energy loss (if detector is calibrated)
         UInt_t zmin = ((KVDetector*)nuc->GetDetectorList()->Last())->FindZmin(-1., nuc->GetMassFormula());
         if (zmin) {
            nuc->SetZ(zmin);
            nuc->SetIsIdentified();
            nuc->SetIDCode(kIDCode5);
            // "Identifying" telescope is taken from list of ID telescopes
            // to which stopping detector belongs
            nuc->SetIdentifyingTelescope((KVIDTelescope*)nuc->GetStoppingDetector()->GetIDTelescopes()->Last());
            nuc->Calibrate();
         }
      }
   }
}

