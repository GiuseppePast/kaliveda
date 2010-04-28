/***************************************************************************
                          kvreconstructednucleus.h  -  description
                             -------------------
    begin                : Fri Oct 18 2002
    copyright            : (C) 2002 by Alexis Mignon
    email                : mignon@ganil.fr

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KVRECONSTRUCTEDNUCLEUS_H
#define KVRECONSTRUCTEDNUCLEUS_H

#include "KVNucleus.h"
#include "KVHashList.h"
#include "KVList.h"
#include "KVDetector.h"
#include "KVIDSubCodeManager.h"
#include "TArrayD.h"

class KVTelescope;
class KVGroup;
class KVIDTelescope;

class KVReconstructedNucleus:public KVNucleus {


protected:

    KVString fDetNames; // list of names of detectors through which particle passed
    KVHashList* fDetList; //! non-persistent list of pointers to detectors
    KVString fIDTelName;   // name of identification telescope which identified this particle (if any)
    KVIDTelescope* fIDTelescope; //! non-persistent pointer to identification telescope

    enum {
        kIsIdentified = BIT(18),  //flag set when identification of particle is complete
        kIsCalibrated = BIT(19),   //flag set when energy calibration of particle is complete
        kCoherency = BIT(20),   //particle created and/or identified by analysis of energy losses of other particles
        kZMeasured = BIT(21),   //the Z attributed to this nucleus was measured
        kAMeasured = BIT(22)   //the A attributed to this nucleus was measured
    };

    Int_t fNSegDet;              //!number of segmented/independent detectors hit by particle
    Int_t fAnalStatus;           //!status of particle after analysis of reconstructed event
    Float_t fRealZ;              //Z returned by identification routine
    Float_t fRealA;              //A returned by identification routine
    Double_t fTargetEnergyLoss;   //calculated energy lost in target

    TArrayD* fECalc; // calculated energy loss of particle in each detector

    virtual void MakeDetectorList();

public:

    KVReconstructedNucleus();
    KVReconstructedNucleus(const KVReconstructedNucleus &);
    void init();
    virtual ~ KVReconstructedNucleus();
    virtual void Print(Option_t * option = "") const;
    virtual void Clear(Option_t * option = "");
    virtual void Reconstruct(KVDetector * kvd);

    const KVSeqCollection *GetDetectorList() const
    {
        // Return pointer to list of detectors through which particle passed,
        // in reverse order (i.e. first detector in list is the one in which particle stopped).
        if (!fDetList) const_cast<KVReconstructedNucleus*>(this)->MakeDetectorList();
        return fDetList;
    };
    KVDetector *GetDetector(int i) const
    {
        //Returns the detector referenced by fDetectors.At(i)
        //If i=0, this is the detector in which the particle stopped.
        //For i>0 one obtains the names of the detectors through which the particle
        //must have passed before stopping, in inverse order (i.e. i=0 is the last
        //detector, as i increases we get the detectors closer to the target).
        if (i>=GetDetectorList()->GetEntries()) return 0;
        return (KVDetector *) GetDetectorList()->At(i);
    };
    void SetDetector(int i, KVDetector *);
    KVDetector *GetStoppingDetector() const
    {
        // Return pointer to the detector in which this particle stopped
        return GetDetector(0);
    };
    Int_t GetNumDet() const
    {
        // Returns number of detectors through which this particle has passed
        return GetDetectorList()->GetEntries();
    };

    Int_t GetNSegDet() const {
        return fNSegDet;
    };
    void SetNSegDet(Int_t seg) {
        fNSegDet = seg;
    };
    inline Int_t GetStatus() const
    {
        // Returns status of reconstructed particle as decided by analysis of the group (KVGroup) in
        // which the particle is reconstructed (see KVGroup::AnalyseParticles).
        // This status is used to decide whether identification of the particle can be attempted
        // straight away or if we need to wait until other particles in the same group have been
        // identified and calibrated (case of >1 particle crossing shared detector in a group).
        //
        //  Status = 0 :   identification is, in principle at least, possible straight away
        //  Status = 1 :   identification is, in principle, possible after identification of other particles
        //                         in the same group which have Status=0
        //  Status = 2 :   the energy loss in the shared detector of the group must be shared
        //                         (arbitrarily) between this and the other particle(s) with Status=2
        //  Status = 3 :   the particle has stopped in the first member of an identification
        //                         telescope; a minimum Z could be estimated from the measured energy loss.
        //                         (see KVDetector::FindZmin)
        return fAnalStatus;
    };

    inline void SetStatus(Int_t a) {
        fAnalStatus = a;
    }

    virtual void GetAnglesFromTelescope(Option_t * opt = "random");
    KVGroup *GetGroup() const
    {
        //Return pointer to group in which the particle is detected
        return (GetStoppingDetector() ?  GetStoppingDetector()->GetGroup() : 0);
    };

    KVTelescope *GetTelescope() const
    {
        //Return pointer to telescope (detector stack) in which the particle is detected
        return (GetStoppingDetector() ?  GetStoppingDetector()->GetTelescope() : 0);
    };

    void AddDetector(KVDetector *);

#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
    virtual void Copy(TObject &) const;
#else
    virtual void Copy(TObject &);
#endif

    KVSeqCollection *GetIDTelescopes() const
    {
        //Gets from detector in which particle stopped the list of all ID telescopes
        //made from the stopping detector and all those aligned in front of it.
        //The first ID telescope in the list is that in which the particle stopped.

        return GetStoppingDetector()->GetAlignedIDTelescopes();
    };
    virtual void Identify();
    virtual void Calibrate();

    KVIDTelescope *GetIdentifyingTelescope() const
    {
        return fIDTelescope;
    };
    void SetIdentifyingTelescope(KVIDTelescope *i)
    {
        fIDTelescope=i;
    };

    void SetElossCalc(KVDetector *, Float_t);
    Float_t GetElossCalc(KVDetector *) const;

    virtual void SetIDCode(UShort_t ) {
    };
    virtual void SetECode(UChar_t ) {
    };

    void SetIsIdentified()
    {
        //When the "identification" state of the particle is set, we add 1 identified particle and
        //subtract 1 unidentified particle from each detector in its list
        SetBit(kIsIdentified);
        const_cast<KVSeqCollection*>(GetDetectorList())->R__FOR_EACH(KVDetector, IncrementIdentifiedParticles) (1);
        const_cast<KVSeqCollection*>(GetDetectorList())->R__FOR_EACH(KVDetector, IncrementUnidentifiedParticles) (-1);
    };
    void SetIsCalibrated() {
        SetBit(kIsCalibrated);
    };
    void SetIsUnidentified()
    {
        //When the "identification" state of the particle is reset, i.e. it becomes an "unidentified particle",
        //we add 1 unidentified particle and subtract 1 identified particle from each detector in its list
        ResetBit(kIsIdentified);
        const_cast<KVSeqCollection*>(GetDetectorList())->R__FOR_EACH(KVDetector, IncrementIdentifiedParticles) (-1);
        const_cast<KVSeqCollection*>(GetDetectorList())->R__FOR_EACH(KVDetector, IncrementUnidentifiedParticles) (1);
    };
    void SetIsUncalibrated() {
        ResetBit(kIsCalibrated);
    };
    Bool_t IsIdentified() const {
        return TestBit(kIsIdentified);
    };
    Bool_t IsCalibrated() const {
        return TestBit(kIsCalibrated);
    };

    Int_t GetIDSubCode(const Char_t * id_tel_type,
                       KVIDSubCode & code) const;
    const Char_t *GetIDSubCodeString(const Char_t * id_tel_type,
                                     KVIDSubCode & code) const;

    void SetRealZ(Float_t Z) {
        fRealZ = Z;
    }
    void SetRealA(Float_t A) {
        fRealA = A;
    }
    Float_t GetRealZ() const {
        if (fRealZ > 0)
            return fRealZ;
        else
            return (Float_t) GetZ();
    }
    Float_t GetRealA() const {
        if (fRealA > 0)
            return fRealA;
        else
            return (Float_t) GetA();
    }
    virtual Float_t GetPID() const
    {
        //Return particle identification PID for this particle.
        //If "real A" has been set by identification routine, this is "real Z" + 0.2*("real A"-2*"real Z")
        //If only Z identification has been performed, it is the "real Z"
        if (GetRealA() > 0)
            return (GetRealZ() + 0.2 * (GetRealA() - 2. * GetRealZ()));
        return GetRealZ();
    };
    virtual void SetTargetEnergyLoss(Double_t e){
        // Set energy loss in target (in MeV) of reconstructed nucleus
        fTargetEnergyLoss=e;
    };
    virtual Double_t GetTargetEnergyLoss() const{
        // Return calculated energy loss in target of reconstructed nucleus (in MeV)
        return fTargetEnergyLoss;
    };

    virtual void SetZMeasured(Bool_t yes=kTRUE)
    {
        // Call with yes=kTRUE for reconstructed nuclei whose
        // atomic number, Z, was measured, not calculated
        SetBit(kZMeasured,yes);
    };
    virtual void SetAMeasured(Bool_t yes=kTRUE)
    {
        // Call with yes=kTRUE for reconstructed nuclei whose
        // mass number, A, was measured, not calculated
        SetBit(kAMeasured,yes);
    };
    virtual Bool_t IsZMeasured()
    {
        // Returns kTRUE for reconstructed nuclei whose
        // atomic number, Z, was measured, not calculated
        return TestBit(kZMeasured);
    };
    virtual Bool_t IsAMeasured()
    {
        // Returns kTRUE for reconstructed nuclei whose
        // mass number, A, was measured, not calculated
        return TestBit(kAMeasured);
    };

    ClassDef(KVReconstructedNucleus, 12)  //Nucleus detected by multidetector array
};

#endif
