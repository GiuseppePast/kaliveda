//Created by KVClassFactory on Tue Apr 16 09:45:50 2013
//Author: John Frankland,,,

#ifndef __KVMultiDetArray_H
#define __KVMultiDetArray_H

#include "KVGeoStrucElement.h"
#include "KVUniqueNameList.h"
#include "TGraph.h"
#include "TGeoManager.h"
#include "KVNucleus.h"
#include "KVDetector.h"

#include <KVGeoDNTrajectory.h>

class KVIDGraph;
class KVTarget;
class KVTelescope;
class KVIDTelescope;
class KVACQParam;
class KVReconstructedEvent;
class KVDetectorEvent;
class KVGroup;
class KVEvent;
class KVNameValueList;
class KVReconstructedNucleus;
class KVList;
class KVGeoNavigator;
class KVRangeTableGeoNavigator;
class KVUpDater;

class KVMultiDetArray : public KVGeoStrucElement {

   friend class KVGeoImport;

protected:

   static Bool_t fCloseGeometryNow;
   static Bool_t fBuildTarget;

   KVTarget* fTarget;          //target used in experiment
   enum {
      kIsRemoving = BIT(14),     //flag set during call to RemoveLayer etc.
      kParamsSet = BIT(15),      //flag set when SetParameters called
      kIsBuilt = BIT(16),        //flag set when Build() is called
      kIsBeingDeleted = BIT(17), //flag set when dtor is called
      kIDParamsSet = BIT(18),    //flag set when SetRunIdentificationParameters called
      kCalParamsSet = BIT(19)    //flag set when SetRunCalibrationParameters called
   };

   TList* fStatusIDTelescopes;//! used by GetStatusIDTelescopes
   TList* fCalibStatusDets;//! used by GetStatusIDTelescopes

   KVDetectorEvent* fHitGroups;          //!   list of hit groups in simulation
   KVSeqCollection* fIDTelescopes;       //->deltaE-E telescopes in groups
   KVSeqCollection* fACQParams;          //list of data acquisition parameters associated to detectors

   TString fDataSet;            //!name of associated dataset, used with MakeMultiDetector()
   UInt_t fCurrentRun;          //Number of the current run used to call SetParameters
   KVUpDater* fUpDater;          //!used to set parameters for multidetector

   Bool_t fSimMode;             //!=kTRUE in "simulation mode" (use for calculating response to simulated events)

   Bool_t fROOTGeometry;//!=kTRUE use ROOT geometry

   Int_t fFilterType;//! type of filtering (used by DetectEvent)

   KVRangeTableGeoNavigator* fNavigator;//! for propagating particles through array geometry

   KVUniqueNameList fTrajectories;//! list of all possible trajectories through detectors of array

   virtual void RenumberGroups();
   virtual void BuildGeometry()
   {
      AbstractMethod("BuildGeometry");
   };
   virtual void MakeListOfDetectors();
   virtual void SetACQParams();

   virtual Int_t GetIDTelescopes(KVDetector*, KVDetector*, TCollection* list);

   int try_all_doubleID_telescopes(KVDetector* de, KVDetector* e, TCollection* l);
   bool try_a_doubleIDtelescope(TString uri, KVDetector* de, KVDetector* e, TCollection* l);
   bool try_upper_and_lower_doubleIDtelescope(TString uri, KVDetector* de, KVDetector* e, TCollection* l);
   int try_all_singleID_telescopes(KVDetector* d, TCollection* l);
   bool try_a_singleIDtelescope(TString uri, KVDetector* d, TCollection* l);
   bool try_upper_and_lower_singleIDtelescope(TString uri, KVDetector* d, TCollection* l);
   virtual void set_up_telescope(KVDetector* de, KVDetector* e, KVIDTelescope* idt, TCollection* l);
   virtual void set_up_single_stage_telescope(KVDetector* det, KVIDTelescope* idt, TCollection* l);

   virtual void SetCalibrators();

   virtual void GetAlignedIDTelescopesForDetector(KVDetector* det, TCollection* list);
   virtual void GetIDTelescopesForGroup(KVGroup* grp, TCollection* tel_list);
   virtual void PrepareModifGroup(KVGroup* grp, KVDetector* dd);
   virtual void SetPresent(KVDetector* det, Bool_t present = kTRUE);
   virtual void SetDetecting(KVDetector* det, Bool_t detecting = kTRUE);

   void CalculateReconstructionTrajectories();
   void DeduceIdentificationTelescopesFromGeometry();
   void AddTrajectory(KVGeoDNTrajectory* d)
   {
      fTrajectories.Add(d);
   }
   void AssociateTrajectoriesAndNodes();

public:
   void CreateGeoManager(Double_t dx = 500, Double_t dy = 500, Double_t dz = 500)
   {
      if (!gGeoManager) {

         new TGeoManager(GetName(), Form("%s geometry for dataset %s", GetName(), fDataSet.Data()));

         TGeoMaterial* matVacuum = gGeoManager->GetMaterial("Vacuum");
         if (!matVacuum) {
            matVacuum = new TGeoMaterial("Vacuum", 0, 0, 0);
            matVacuum->SetTitle("Vacuum");
         }
         TGeoMedium* Vacuum = gGeoManager->GetMedium("Vacuum");
         if (!Vacuum) Vacuum = new TGeoMedium("Vacuum", 1, matVacuum);
         TGeoVolume* top = gGeoManager->MakeBox("WORLD", Vacuum,  dx, dy, dz);
         gGeoManager->SetTopVolume(top);
      }

   }
   void SetGeometry(TGeoManager*);
   TGeoManager* GetGeometry() const;
   KVGeoNavigator* GetNavigator() const;
   void SetNavigator(KVGeoNavigator* geo);

   // filter types. values of fFilterType
   enum EFilterType {
      kFilterType_Geo,
      kFilterType_GeoThresh,
      kFilterType_Full
   };
   KVMultiDetArray();
   virtual ~KVMultiDetArray();

   void SetFilterType(Int_t t)
   {
      fFilterType = t;
   };
   void init();

   virtual void Build(Int_t run = -1);
   virtual void CreateIDTelescopesInGroups();

   virtual void Clear(Option_t* opt = "");

   virtual KVTelescope* GetTelescope(const Char_t* name) const;
   virtual KVGroup* GetGroupWithDetector(const Char_t*);
   virtual KVGroup* GetGroup(const Char_t*);
   virtual KVGroup* GetGroupWithAngles(Float_t /*theta*/, Float_t /*phi*/)
   {
      return 0;
   }
   void RemoveGroup(KVGroup*);
   void RemoveGroup(const Char_t*);
   void ReplaceDetector(const Char_t* name, KVDetector* new_kvd);

   void AddACQParam(KVACQParam*);
   KVSeqCollection* GetACQParams()
   {
      return fACQParams;
   };
   KVACQParam* GetACQParam(const Char_t* name)
   {
      return (KVACQParam*) fACQParams->FindObject(name);
   };
   virtual void SetArrayACQParams();

   virtual void DetectEvent(KVEvent* event, KVReconstructedEvent* rec_event, const Char_t* detection_frame = "");
   virtual Int_t FilteredEventCoherencyAnalysis(Int_t round, KVReconstructedEvent* rec_event);
   virtual void GetDetectorEvent(KVDetectorEvent* detev, TSeqCollection* fired_params = 0);
   virtual void ReconstructEvent(KVReconstructedEvent*, KVDetectorEvent*);
   KVNameValueList* DetectParticle_TGEO(KVNucleus* part);
   virtual KVNameValueList* DetectParticle(KVNucleus* part)
   {
      return DetectParticle_TGEO(part);
   }
   void DetectParticleIn(const Char_t* detname, KVNucleus* kvp);

   KVIDTelescope* GetIDTelescope(const Char_t* name) const;
   KVSeqCollection* GetListOfIDTelescopes() const
   {
      return fIDTelescopes;
   };
   KVList* GetIDTelescopeTypes();
   KVSeqCollection* GetIDTelescopesWithType(const Char_t* type);
   virtual void SetDetectorThicknesses();

   void SetTarget(const Char_t* material, const Float_t thickness);
   void SetTarget(KVTarget* target);
   void SetTargetMaterial(const Char_t* material);
   void SetTargetThickness(const Float_t thickness);
   KVTarget* GetTarget()
   {
      return fTarget;
   };

   virtual Double_t GetTargetEnergyLossCorrection(KVReconstructedNucleus*);

   Bool_t IsRemoving()
   {
      return TestBit(kIsRemoving);
   }

   virtual void SetPedestals(const Char_t*);

   virtual Bool_t IsBuilt() const
   {
      return TestBit(kIsBuilt);
   }
   static KVMultiDetArray* MakeMultiDetector(const Char_t* dataset_name, Int_t run = -1, TString classname = "KVMultiDetArray");

   Bool_t IsBeingDeleted()
   {
      return TestBit(kIsBeingDeleted);
   }
   KVUpDater* GetUpDater();
   virtual void SetParameters(UShort_t n);
   virtual void SetRunIdentificationParameters(UShort_t n);
   virtual void SetRunCalibrationParameters(UShort_t n);

   Bool_t ParamsSet()
   {
      return TestBit(kParamsSet);
   };
   Bool_t IDParamsSet()
   {
      return (TestBit(kIDParamsSet) || ParamsSet());
   };
   Bool_t CalParamsSet()
   {
      return (TestBit(kCalParamsSet) || ParamsSet());
   };
   UInt_t GetCurrentRunNumber() const
   {
      return fCurrentRun;
   };

   virtual void SetIdentifications();
   virtual void InitializeIDTelescopes();
   virtual void UpdateIDTelescopes();
   virtual void UpdateIdentifications();
   virtual void UpdateCalibrators();

   virtual Double_t GetTotalSolidAngle(void)
   {
      return 0;
   }

   TList* GetStatusOfIDTelescopes();
   TList* GetCalibrationStatusOfDetectors();
   void PrintStatusOfIDTelescopes();
   void PrintCalibStatusOfDetectors();


   virtual void SetSimMode(Bool_t on = kTRUE)
   {
      // Set simulation mode of array (and of all detectors in array)
      // If on=kTRUE (default), we are in simulation mode (calculation of energy losses etc.)
      // If on=kFALSE, we are analysing/reconstruction experimental data
      fSimMode = on;
      const_cast<KVSeqCollection*>(GetDetectors())->Execute("SetSimMode", Form("%d", (Int_t)on));
   };
   virtual Bool_t IsSimMode() const
   {
      // Returns simulation mode of array:
      //   IsSimMode()=kTRUE : we are in simulation mode (calculation of energy losses etc.)
      //   IsSimMode()=kFALSE: we are analysing/reconstruction experimental data
      return fSimMode;
   };

   virtual Double_t GetPunchThroughEnergy(const Char_t* detector, Int_t Z, Int_t A);
   virtual TGraph* DrawPunchThroughEnergyVsZ(const Char_t* detector, Int_t massform = KVNucleus::kBetaMass);
   virtual TGraph* DrawPunchThroughEsurAVsZ(const Char_t* detector, Int_t massform = KVNucleus::kBetaMass);

   virtual void SetROOTGeometry(Bool_t on = kTRUE);
   Bool_t IsROOTGeometry() const
   {
      // Return kTRUE if the geometry of this multidetector is described by ROOT geometry
      // i.e. if the detectors have ROOT geometry volume & shape informations
      // (we only test the first detector)

      if (!GetDetectors()->GetEntries()) return kFALSE;
      KVDetector* d = (KVDetector*)GetDetectors()->First();
      return d->ROOTGeo();
   }
   void CalculateDetectorSegmentationIndex();
   virtual void AnalyseGroupAndReconstructEvent(KVReconstructedEvent* recev, KVGroup* grp);
   virtual void SetGridsInTelescopes(UInt_t run);
   void FillListOfIDTelescopes(KVIDGraph* gr) const;

   void Draw(Option_t* option = "");
   const TSeqCollection* GetTrajectories() const
   {
      // Get list of all possible trajectories for particles traversing array
      return &fTrajectories;
   }

   void CheckROOTGeometry()
   {
      // Turns on ROOT geometry if not already in use
      if (!IsROOTGeometry()) SetROOTGeometry();
   }
   void MakeHistogramsForAllIDTelescopes(KVSeqCollection* list, Int_t dimension = 100);
   void FillHistogramsForAllIDTelescopes(KVSeqCollection* list);

   void SetDetectorTransparency(Char_t);

   ClassDef(KVMultiDetArray, 7) //Base class for multidetector arrays
};

//................  global variable
R__EXTERN KVMultiDetArray* gMultiDetArray;

#endif
