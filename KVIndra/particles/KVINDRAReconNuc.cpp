/***************************************************************************
$Id: KVINDRAReconNuc.cpp,v 1.61 2009/04/03 14:28:37 franklan Exp $
                          kvdetectedparticle.cpp  -  description
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
#include "TBuffer.h"
#include "TEnv.h"
#include "KVDataSet.h"
#include "KVINDRAReconNuc.h"
#include "KVList.h"
#include "KVGroup.h"
#include "KVLayer.h"
#include "KVINDRA.h"
#include "KVTelescope.h"
#include "KVIDTelescope.h"
#include "TCollection.h"
#include "KVChIo.h"
#include "KVACQParam.h"
#include "KVSilicon.h"
#include "KVCsI.h"
/******* for Identify **********/
#include "KVIDGCsI.h"
#include "KVIDChIoSi.h"
#include "KVIDCsI.h"

ClassImp(KVINDRAReconNuc);

////////////////////////////////////////////////////////////////////////////
//KVINDRAReconNuc
//
//Nuclei reconstructed from data measured in the INDRA array.
//Most useful methods are already defined in parent classes KVReconstructedNucleus,
//KVNucleus and KVParticle. Here we add a few useful INDRA-specific methods :
//
//   GetRingNumber(), GetModuleNumber()
//   GetTimeMarker()  -  returns time-marker of detector in which the particle stopped
//
//   StoppedInChIo(), StoppedInSi(), StoppedInCsI()  -  information on the detector in
//                                                      which the particle stopped
//       ** Note1: to access the detector in question, use GetStoppingDetector() (see KVReconstructedNucleus)
//       ** Note2: for a general test of the type of detector in which the particle stopped,
//                 you can do one of the following e.g. to test if it stopped in an
//                 ionisation chamber (assuming a particle pointer 'part'):
//       
//        if( !strcmp(part->GetStoppingDetector()->GetType(), "CI") ) { ... }
//               //test the type of the detector - see INDRA detector classes for the different types
//        if( part->GetStoppingDetector()->InheritsFrom("KVChIo") ) { ... }
//               //test the inheritance of the class of the stopping detector
//
//    GetChIo(), GetSi(), GetCsI()    -  return pointers to the detectors through which the particle passed
//    GetEnergyChIo(), GetEnergySi(), GetEnergyCsI()   -  return the energy measured in the detectors
//
//Identification/Calibration status and codes
//-------------------------------------------
//
//Identified particles have IsIdentified()=kTRUE.
//Calibrated particles have IsCalibrated()=kTRUE.
//
//The KVINDRACodes object fCodes is used to store information on the identification and
//calibration of the particle. You can access this object using GetCodes() and then use the
//methods of KVINDRACodes/KVINDRACodeMask to obtain the information.
//
//For example, you can obtain the VEDA identification code of a particle 'part' using
//
//       part.GetCodes().GetVedaIDCode()  [ = 0, 1, 2, etc.]
//
//Information on whether the particle's mass was measured :
//
//       part.IsAMeasured()  [ = kTRUE or kFALSE]
//
//Information on whether the particle's charge was measured :
//
//       part.IsZMeasured()  [ = kTRUE or kFALSE]
//
//You can also access the information stored in KVINDRACodes on the status codes returned
//by the different identification telescopes used to identify the particle:
//see GetIDSubCode() and GetIDSubCodeString().
//
//Masses of reconstructed nuclei
//------------------------------
//
//When the nucleus' A is not measured, we estimate it from the identified Z.
//By default the mass formula used is that of R.J. Charity (see KVNucleus::GetAFromZ).
//     ** Note: for data previous to the 5th campaign converted to the KaliVeda format
//     ** from old DSTs, we keep the masses from the Veda programme, corresponding to
//     ** KVNucleus mass option kVedaMass.
//IN ALL CASES THE RETURNED VALUE OF GetA() IS POSITIVE
//Use GetCodes().GetIsotopeResolve() to test whether A is measured or calculated.
//

void KVINDRAReconNuc::init()
{
	//default initialisations
	if (gDataSet)
		SetMassFormula(UChar_t(gDataSet->GetDataSetEnv("KVINDRAReconNuc.MassFormula",Double_t(kEALMass))));
}

KVINDRAReconNuc::KVINDRAReconNuc():fCodes()
{
   //default ctor
   init();
}

KVINDRAReconNuc::KVINDRAReconNuc(const KVINDRAReconNuc & obj):fCodes()
{
   //copy ctor
   init();
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   obj.Copy(*this);
#else
   ((KVINDRAReconNuc &) obj).Copy(*this);
#endif
}

KVINDRAReconNuc::~KVINDRAReconNuc()
{
   //dtor
   init();
   fCodes.Clear();
}

#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
void KVINDRAReconNuc::Copy(TObject & obj) const
#else
void KVINDRAReconNuc::Copy(TObject & obj)
#endif
{
   //
   //Copy this to obj
   //
   KVReconstructedNucleus::Copy(obj);
}

void KVINDRAReconNuc::Streamer(TBuffer & R__b)
{
   UInt_t R__s, R__c;
   if (R__b.IsReading()) {
      Version_t R__v = R__b.ReadVersion(&R__s, &R__c);
      KVReconstructedNucleus::Streamer(R__b);
      fCodes.Streamer(R__b);
      Float_t RealZ, RealA;
      if (R__v < 6) {
         R__b >> RealZ;
         R__b >> RealA;
         SetRealZ(RealZ);
         SetRealA(RealA);
      }
      if (R__v < 7) {
			// before version 7 (and before version 11 of KVReconstructedNucleus),
			// the kZMeasured and kAMeasured bits were not used.
			// for data written previous to this, we :
			//   SetZMeasured(kTRUE) for particles with IsIdentified()=kTRUE
			//   SetAMeasured(kTRUE) for particles with fCodes.GetIsotopeResolve()=kTRUE
			SetZMeasured(IsIdentified());
			SetAMeasured(fCodes.GetIsotopeResolve());
		}
      R__b.CheckByteCount(R__s, R__c, KVINDRAReconNuc::IsA());
   } else {
      KVINDRAReconNuc::Class()->WriteBuffer(R__b, this);
   }
}

void KVINDRAReconNuc::Print(Option_t * option) const
{

   KVReconstructedNucleus::Print(option);

   cout << "KVINDRAReconNuc: fRing=" << GetRingNumber() << " fModule=" <<
       GetModuleNumber() << endl;
   if (IsIdentified()) {
      cout << " =======> ";
      cout << " Z=" << GetZ() << " A=" << ((KVINDRAReconNuc *) this)->
          GetA();
      cout << " Zreal=" << GetRealZ();
      if (const_cast <
          KVINDRAReconNuc * >(this)->GetCodes().GetIsotopeResolve())
         cout << " Areal=" << ((KVINDRAReconNuc *) this)->
             GetRealA();
      cout << endl << " Identification code = " << ((KVINDRAReconNuc *) this)->
          GetCodes().GetIDStatus() << endl;
   } else {
      cout << "(unidentified)" << endl;
   }
   if (GetIDTelescopes() && GetIDTelescopes()->GetSize()) {
      cout << " ID Sub-codes: " << endl;
      TIter next(GetIDTelescopes());
      KVIDTelescope *idt;
      while ((idt = (KVIDTelescope *) next())) {
         cout << idt->GetName() << " : " << GetIDSubCodeString(idt->
                                                               GetType())
             << endl;
      }
   }
   if (IsCalibrated()) {
      cout << " Total Energy = " << GetEnergy() << " MeV,  Theta=" << GetTheta() << " Phi=" << GetPhi() << endl;
      cout << " Target energy loss correction :  " << GetTargetEnergyLoss() << " MeV" << endl;
      cout << " Calibration code = " << ((KVINDRAReconNuc *) this)->
          GetCodes().GetEStatus() << endl;
   } else {
      cout << "(uncalibrated)" << endl;
   }
   if (!IsIdentified()) {
      cout << "Analysis : ";
      switch (GetStatus()) {
      case 0:
         cout <<
             "Particle alone in group, or identification independently of other"
             << endl;
         cout << "particles in group is directly possible." << endl;
         break;

      case 1:
         cout <<
             "Particle reconstructed after identification of others in group"
             << endl;
         cout <<
             "and subtraction of their calculated energy losses in ChIo."
             << endl;
         break;

      case 2:
         cout <<
             "Particle identification estimated after arbitrary sharing of"
             << endl;
         cout <<
             "energy lost in ChIo between several reconstructed particles."
             << endl;
         break;

      case 3:
         cout <<
             "Particle stopped in first stage of telescope. Estimation of minimum Z."
             << endl;
         break;

      default:
         cout << GetStatus() << endl;
         break;
      }
   }
   cout <<
       "-------------------------------------------------------------------------------"
       << endl;
}


//______________________________________________________________________________

void KVINDRAReconNuc::EnergyIdentification()
{


   //make a clone of the entire telescope structure through which the particle passed
   KVGroup *grp = new KVGroup;
   const KVRList *det_list = GetDetectorList();
   TIter nxt_det(det_list);
   KVDetector *det;
   KVTelescope *kvt, *last_kvt, *new_kvt;
   last_kvt = 0;

   while ((det = (KVDetector *) nxt_det())) {   //loop over hit detectors
      kvt = det->GetTelescope();        //get associated telescope
      if (kvt != last_kvt) {    //if telescope not already in group
         new_kvt = (KVTelescope *) kvt->Clone();        //clone the telescope
         //copy angular position of telescope (needed for detecting particles)
         new_kvt->SetPolarMinMax(kvt->GetThetaMin(), kvt->GetThetaMax());
         new_kvt->SetAzimuthalMinMax(kvt->GetPhiMin(), kvt->GetPhiMax());
         grp->Add(new_kvt);     //add to group
         last_kvt = kvt;
      }
   }
   grp->Sort();                 //put telescopes in right order for detection of particles

   //measured energy losses
   KVDetector *fEres_det = (KVDetector *) det_list->At(0);
   KVDetector *fdE_det = (KVDetector *) det_list->At(1);
   Float_t fEres = fEres_det->GetEnergy();
   Float_t fdE = fdE_det->GetEnergy();

   //find equivalent eres,de detectors in cloned telescopes
   KVDetector *eres_guess = grp->GetDetector(fEres_det->GetName());
   KVDetector *de_guess = grp->GetDetector(fdE_det->GetName());

   //minimum incident energy of particle is sum of E_res + dE
   Float_t Emin = fEres + fdE;
   // if chio-si/csi then the missing energy is the losses in the two mylar windows
   // if si-csi then the missing energy is the losses in the chio + the two mylar windows
   // empirically, the mylar energy loss is < the energy lost in the chio
   // so estimate that Emax = Emin + 2*energy lost in chio
   // N.B. if phoswich or si-csi (ring 1) then there is no chio and no missing energy !!!
   Float_t Emax;
   if (gIndra->GetChIoOf(fEres_det->GetName())) {
      Emax =
          Emin +
          2.0 * (gIndra->GetChIoOf(fEres_det->GetName())->GetEnergy());
   } else {
      Emax = Emin;
   }

   //scan nuclei from proton to ...
   UInt_t Zmin = 1;
   UInt_t Zmax = 90;

   KVNucleus nuc;               //test particle

   Float_t Ebest = 0.0;
   UInt_t Zbest = 0;
   //error used to determine best solution
   Float_t chi2min = -1.;
   Float_t chi2;

   // first scan: Z=1,2,3,4,5,6,7,8,9,10,15,...,85,90  Einc = Emin to Emax in 10 steps
   Float_t Einc = Emin;
   Float_t dE = (Emax - Emin) / 10.;
   while (Einc <= Emax) {
      for (UInt_t Z = Zmin; Z <= 9; Z++) {

         //set up incident particle with given energy and Z
         nuc.Clear();
         nuc.SetZ(Z);           //given A of valley of stability
         //the particle is given the incident energy and a randomly directed
         //momentum within the angular range of the telescope in which it stopped
         nuc.SetRandomMomentum(Einc,
                               fEres_det->GetTelescope()->GetThetaMin(),
                               fEres_det->GetTelescope()->GetThetaMax(),
                               fEres_det->GetTelescope()->GetPhiMin(),
                               fEres_det->GetTelescope()->GetPhiMax(),
                               "random");

         //simulate detection in telescopes of group
         grp->DetectParticle(&nuc);

         //calculate difference between simulated and detected energy losses
         chi2 = pow((eres_guess->GetEnergy() - fEres), 2) +
             pow((de_guess->GetEnergy() - fdE), 2);

         chi2 = TMath::Sqrt(chi2);

         if (chi2 < chi2min || chi2min < 0) {   //look for smallest chi2 solution
            chi2min = chi2;
            Zbest = Z;
            Ebest = Einc;
         }
         grp->Reset();          //reset energy losses
      }
      for (UInt_t Z = 10; Z <= Zmax; Z += 5) {

         //set up incident particle with given energy and Z
         nuc.Clear();
         nuc.SetZ(Z);           //given A of valley of stability
         //the particle is given the incident energy and a randomly directed
         //momentum within the angular range of the telescope in which it stopped
         nuc.SetRandomMomentum(Einc,
                               fEres_det->GetTelescope()->GetThetaMin(),
                               fEres_det->GetTelescope()->GetThetaMax(),
                               fEres_det->GetTelescope()->GetPhiMin(),
                               fEres_det->GetTelescope()->GetPhiMax(),
                               "random");

         //simulate detection in telescopes of group
         grp->DetectParticle(&nuc);

         //calculate difference between simulated and detected energy losses
         chi2 = pow((eres_guess->GetEnergy() - fEres), 2) +
             pow((de_guess->GetEnergy() - fdE), 2);

         chi2 = TMath::Sqrt(chi2);

         if (chi2 < chi2min || chi2min < 0) {   //look for smallest chi2 solution
            chi2min = chi2;
            Zbest = Z;
            Ebest = Einc;
         }
         grp->Reset();          //reset energy losses
      }
      Einc += dE;               //10 steps for first scan
   }

   //second scan

   if (Zbest > 4)
      Zmin = Zbest - 4;
   else
      Zmin = 1;
   Zmax = ((Zbest + 4) > 92) ? 92 : Zbest + 4;

   Ebest = 0.0;
   Zbest = 0;
   UInt_t Abest = 0;
   //error used to determine best solution
   chi2min = -1.;
   //define histogram for checking identification

   Einc = Emin;
   while (Einc <= Emax) {
      for (UInt_t Z = Zmin; Z <= Zmax; Z++) {
         UInt_t Amin = nuc.GetAFromZ((UInt_t) Z, nuc.GetMassFormula());
         if (Amin < 4)
            Amin = 1;
         UInt_t Amax = Amin + 6;
         for (UInt_t A = Amin; A <= Amax; A++) {
            if (A >= Z) {
               //set up incident particle with given energy and Z
               nuc.Clear();
               nuc.SetZ(Z);
               nuc.SetA(A);
               //the particle is given the incident energy and a randomly directed
               //momentum within the angular range of the telescope in which it stopped
               nuc.SetRandomMomentum(Einc,
                                     fEres_det->GetTelescope()->
                                     GetThetaMin(),
                                     fEres_det->GetTelescope()->
                                     GetThetaMax(),
                                     fEres_det->GetTelescope()->
                                     GetPhiMin(),
                                     fEres_det->GetTelescope()->
                                     GetPhiMax(), "random");

               //simulate detection in telescopes of group
               grp->DetectParticle(&nuc);

               //calculate difference between simulated and detected energy losses
               chi2 = pow((eres_guess->GetEnergy() - fEres), 2) +
                   pow((de_guess->GetEnergy() - fdE), 2);

               chi2 = TMath::Sqrt(chi2);

               if (chi2 < chi2min || chi2min < 0) {     //look for smallest chi2 solution
                  chi2min = chi2;
                  Zbest = Z;
                  Abest = A;
                  Ebest = Einc;
               }

               grp->Reset();    //reset energy losses
            }
         }
      }
      Einc += dE;               //10 steps for second scan
   }

   //store result
   SetZ(Zbest);
   SetA(Abest);
   SetKE(Ebest);
   //delete cloned telescopes in test group (group doesn't own telescopes)
   grp->GetTelescopes()->Delete();
   //delete group
   delete grp;
   cout << "Energy Identification : Z=" << Zbest << " A=" << Abest << " E="
       << Ebest << endl;
}

//________________________________________________________________________________________

void KVINDRAReconNuc::Clear(Option_t * t)
{
   //reset nucleus' properties
   KVReconstructedNucleus::Clear(t);
   init();
   fCodes.Clear();
}

KVChIo *KVINDRAReconNuc::GetChIo()
{
   //Return pointer to the ChIo the particle passed through.
   //Pointer is null if not.
   KVChIo *chio = (KVChIo *) GetDetectorList()->FindObjectByType("CI");
   return chio;
}

KVSilicon *KVINDRAReconNuc::GetSi()
{
   //Return pointer to the Silicon the particle passed through.
   //Pointer is null if not.
   KVSilicon *chio =
       (KVSilicon *) GetDetectorList()->FindObjectByType("SI");
   return chio;
}

KVCsI *KVINDRAReconNuc::GetCsI()
{
   //Return pointer to the CsI the particle passed through.
   //Pointer is null if not.
   KVCsI *csi = (KVCsI *) GetDetectorList()->FindObjectByType("CSI");
   return csi;
}

Float_t KVINDRAReconNuc::GetEnergyChIo()
{
   //If this particle passed through a ChIo, return the energy measured
   KVChIo *chio = GetChIo();
   if (chio) {
      return chio->GetEnergy();
   }
   return 0.0;
}

Float_t KVINDRAReconNuc::GetEnergySi()
{
   //If this particle passed through a Si, return the energy measured
   KVSilicon *si = GetSi();
   if (si) {
      return si->GetEnergy();
   }
   return 0.0;
}

Float_t KVINDRAReconNuc::GetEnergyCsI()
{
   //If this particle passed through a CsI, return the energy measured
   KVCsI *csi = GetCsI();
   if (csi) {
      return csi->GetEnergy();
   }
   return 0.0;
}

Bool_t KVINDRAReconNuc::StoppedInChIo()
{
   //Returns kTRUE if particle stopped in ChIo detector

   if (!strcmp(GetStoppingDetector()->GetType(), "CI")) {
      return kTRUE;
   } else {
      return kFALSE;
   }
}

Bool_t KVINDRAReconNuc::StoppedInSi()
{
   //Returns kTRUE if particle stopped in Si detector

   if (!strcmp(GetStoppingDetector()->GetType(), "SI")) {
      return kTRUE;
   } else {
      return kFALSE;
   }
}

Bool_t KVINDRAReconNuc::StoppedInCsI()
{
   //Returns kTRUE if particle stopped in CsI detector

   if (!strcmp(GetStoppingDetector()->GetType(), "CSI")) {
      return kTRUE;
   } else {
      return kFALSE;
   }
}

Int_t KVINDRAReconNuc::GetIDSubCode(const Char_t * id_tel_type) const
{
   //Returns subcode/status code from identification performed in ID telescope of given type.
   //i.e. to obtain CsI R-L subcode use GetIDSubCode("CSI_R_L").
   //
   //The meaning of the subcodes is defined in the corresponding KVIDTelescope class description
   //i.e. for CsI R-L, look at KVIDCsI.
   //
   //The subcode is set for any attempted identification, not necessarily that which eventually
   //leads to the definitive identification of the particle.
   //i.e. in the example of the CsI R-L subcode, the particle in question may in fact be identified
   //by a Si-CsI telescope, because the CsI identification routine returned e.g. KVIDGCsI::kICODE7
   //(a gauche de la ligne fragment, Z est alors un Zmin et le plus probable).
   //
   //calling GetIDSubCode() with no type returns the identification subcode corresponding
   //to the identifying telescope (whose pointer is given by GetIdentifyingTelescope()).
   //
   //In case of problems (see KVReconstructedNucleus::GetIDSubCode()) value returned is -65535

   return KVReconstructedNucleus::GetIDSubCode(id_tel_type,
                                               const_cast <
                                               KVINDRAReconNuc *
                                               >(this)->GetCodes().
                                               GetSubCodes());
}

const Char_t *KVINDRAReconNuc::GetIDSubCodeString(const Char_t *
                                                  id_tel_type) const
{
   //Returns explanatory message concerning identification performed in ID telescope of given type.
   //(see GetIDSubCode())
   //
   //The subcode is set for any attempted identification, not necessarily that which eventually
   //leads to the definitive identification of the particle.
   //
   //calling GetIDSubCodeString() with no type returns the identification subcode message corresponding
   //to the identifying telescope (whose pointer is given by GetIdentifyingTelescope()).
   //
   //In case of problems:
   //       no ID telescope of type 'id_tel_type' :  "No identification attempted in id_tel_type"
   //       particle not identified               :  "Particle unidentified. Identifying telescope not set."

   return KVReconstructedNucleus::GetIDSubCodeString(id_tel_type,
                                                     const_cast <
                                                     KVINDRAReconNuc *
                                                     >(this)->GetCodes().
                                                     GetSubCodes());
}

//____________________________________________________________________________________________

void KVINDRAReconNuc::Identify()
{
   // INDRA-specifica particle identification.
   // Here we attribute the Veda6-style general identification codes depending on the
   // result of KVReconstructedNucleus::Identify and the subcodes from the different
   // identification algorithms:
   //
   //IDENTIFIED PARTICLES
         //Identified particles with ID code = 2 with subcodes 4 & 5
         //(masse hors limite superieure/inferieure) are relabelled
         //with kIDCode10 (identification entre les lignes CsI)
         //
  // UNIDENTIFIED PARTICLES
  //Unidentified particles receive the general ID code for non-identified particles (kIDCode14)
         //EXCEPT if their identification in CsI R-L gave subcodes 6 or 7
         //(Zmin) then they are relabelled "Identified" with IDcode = 9 (ident. incomplete dans CsI ou Phoswich (Z.min))
         //Their "identifying" telescope is set to the CsI ID telescope
   KVReconstructedNucleus::Identify();
   
   if ( IsIdentified() ) {
      
      /******* IDENTIFIED PARTICLES *******/
      if ( GetIdentifyingTelescope()->InheritsFrom("KVIDCsI") ) {   /**** CSI R-L IDENTIFICATION ****/
         
         //Identified particles with ID code = 2 with subcodes 4 & 5
         //(masse hors limite superieure/inferieure) are relabelled
         //with kIDCode10 (identification entre les lignes CsI)
         
         Int_t csi_subid = (Int_t) GetIDSubCode(GetCodes().GetSubCodes());
         if (csi_subid == KVIDGCsI::kICODE4 || csi_subid == KVIDGCsI::kICODE5) {
            SetIDCode(kIDCode10);
         }
         
      }
      
   }
   else
   {
      
      /******* UNIDENTIFIED PARTICLES *******/
      
      /*** general ID code for non-identified particles ***/
      SetIDCode( kIDCode14 );
      
      KVIDCsI* idtel = (KVIDCsI*)GetIDTelescopes()->FindObjectByType("CSI_R_L");
      if( idtel ){
         //Particles remaining unidentified are checked: if their identification in CsI R-L gave subcodes 6 or 7
         //(Zmin) then they are relabelled "Identified" with IDcode = 9 (ident. incomplete dans CsI ou Phoswich (Z.min))
         //Their "identifying" telescope is set to the CsI ID telescope
         Int_t csi_subid = (Int_t)idtel->GetIDSubCode( GetCodes().GetSubCodes() );
         if(csi_subid == KVIDGCsI::kICODE6 || csi_subid == KVIDGCsI::kICODE7){
            SetIsIdentified();
            SetIDCode( kIDCode9 );
            SetIdentifyingTelescope(idtel);
         }
      }
      
   }
}   

//_________________________________________________________________________________

void KVINDRAReconNuc::Calibrate()
{
   //Calculate and set the energy of a (previously identified) reconstructed particle.
   //This just calls KVReconstructedNucleus::Calibrate, and then sets the
   //Veda energy calibration code according to the result of KVIDTelescope::GetCalibStatus.

   KVReconstructedNucleus::Calibrate();
   KVIDTelescope* idt;
   if ( (idt = (KVIDTelescope*)fIDTelescope.GetObject()) ){
      if( idt->GetCalibStatus() == KVIDTelescope::kCalibStatus_OK )
         SetECode( kECode1 );
      else if( idt->GetCalibStatus() == KVIDTelescope::kCalibStatus_Calculated )
         SetECode( kECode11 );
      else if( idt->GetCalibStatus() == KVIDTelescope::kCalibStatus_Multihit )
         SetECode( kECode4 );
      else if( idt->GetCalibStatus() == KVIDTelescope::kCalibStatus_Coherency )
         SetECode( kECode5 );
      else if( idt->GetCalibStatus() == KVIDTelescope::kCalibStatus_NoCalibrations )
         SetECode( kECode0 );
   }
}