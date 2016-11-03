//Created by KVClassFactory on Tue Jan 27 11:37:39 2015
//Author: ,,,

#include "KVFAZIA.h"
#include "KVGeoImport.h"
#include "KVSignal.h"
#include "KVFAZIADetector.h"
#include "KVGroup.h"
#include "KVFAZIABlock.h"
#include "KVDetectorEvent.h"
#include "KVTarget.h"
#include "TSystem.h"
#include "KVDataSet.h"
#include "KVConfig.h"

//#include "TGeoBox.h"
#include "TGeoCompositeShape.h"
#include "TGeoEltu.h"


ClassImp(KVFAZIA)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVFAZIA</h2>
<h4>Base class for description of the FAZIA set up</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////
KVFAZIA* gFazia;

KVFAZIA::KVFAZIA()
{
   // Default constructor
   SetName("FAZIA");
   SetTitle(ClassName());
   fStartingBlockNumber = 0;
   gFazia = this;
   fDetectorLabels = "";
   fSignalTypes = "QL1,I1,QH1,Q2,I2,Q3";
   SetGeometryImportParameters();
}

KVFAZIA::~KVFAZIA()
{
   // Destructor
}

void KVFAZIA::AddDetectorLabel(const Char_t* label)
{
   if (fDetectorLabels == "") fDetectorLabels += label;
   else if (!fDetectorLabels.Contains(label)) fDetectorLabels += Form(",%s", label);
}

void KVFAZIA::GenerateCorrespondanceFile()
{
   fDetectorLabels = "SI1,SI2,CSI";
#ifdef WITH_GNU_INSTALL
   fCorrespondanceFile.Form("%s/%s-%s.names", KVBase::WorkingDirectory(), gSystem->Getenv("USER"), ClassName());
#else
   fCorrespondanceFile.Form("%s/%s-%s.names", KVBase::GetDATADIRFilePath(), gSystem->Getenv("USER"), ClassName());
#endif
   Info("GenerateCorrespondanceFile", "Creation de %s", fCorrespondanceFile.Data());
   KVEnv env;

   SetNameOfDetectors(env);
   if (env.GetTable() && env.GetTable()->GetEntries() > 0) {
      env.AddCommentLine(Form("Automatic generated file by %s::GenerateCorrespondanceFile", ClassName()));
      env.AddCommentLine("Make link between geometric ROOT objects and detector names");
      env.WriteFile(fCorrespondanceFile.Data());
   }
   fDetectorLabels = "";
}

void KVFAZIA::SetNameOfDetectors(KVEnv& env)
{
   //define the format of detectors name
   // label-index
   // where index = block*100+quartet*10+telescope
   // example :
   // SI1-123 is the Silicon 1 of the block 1, the quartet 2 and the telescope 3
   //

   for (Int_t bb = fStartingBlockNumber; bb < fNblocks; bb += 1) {
      for (Int_t qq = 1; qq <= 4; qq += 1) {
         for (Int_t tt = 1; tt <= 4; tt += 1) {
            fDetectorLabels.Begin(",");
            while (!fDetectorLabels.End()) {
               KVString sdet = fDetectorLabels.Next();
               env.SetValue(
                  Form("BLOCK_%d_QUARTET_%d_%s-T%d", bb, qq, sdet.Data(), tt),
                  Form("%s-%d", sdet.Data(), bb * 100 + qq * 10 + tt)
               );
            }
         }
      }
   }
}

void KVFAZIA::GetGeometryParameters()
{
   //Called by the Build method
   Info("GetGeometryParameters", "to be defined in child class ...");
}

void KVFAZIA::BuildFAZIA()
{
   //Called by the Build method
   Info("BuildFAZIA", "to be defined in child class ...");

}

void KVFAZIA::BuildTarget()
{

   KVMaterial target_holder_mat("Al");
   new TGeoBBox("TARGET_FRAME", 3., 3., 0.1 / 2.);
   new TGeoEltu("TARGET_HOLE", 2., 2., 0.1 / 2.);
   TGeoCompositeShape* cs = new TGeoCompositeShape("TARGET_FRAME", "TARGET_FRAME - TARGET_HOLE");
   TGeoVolume* target_frame = new TGeoVolume("TARGET_FRAME", cs, target_holder_mat.GetGeoMedium());
   gGeoManager->GetTopVolume()->AddNode(target_frame, 1);

   KVTarget* T = GetTarget();
   if (T) {
      KVMaterial* targMat = (KVMaterial*)T->GetLayers()->First();
      TGeoVolume* target = gGeoManager->MakeEltu("TARGET", targMat->GetGeoMedium(), 2., 2., targMat->GetThickness() / 2.);
      gGeoManager->GetTopVolume()->AddNode(target, 1);
   }
}

void KVFAZIA::Build(Int_t run)
{
   // Build the FAZIA array
   GetGeometryParameters();
   GenerateCorrespondanceFile();

   CreateGeoManager();

   BuildFAZIA();

   if (fBuildTarget)
      BuildTarget();

   if (fCloseGeometryNow) {
      gGeoManager->DefaultColors();
      gGeoManager->CloseGeometry();
   }
   KVGeoImport imp(gGeoManager, KVMaterial::GetRangeTable(), this, kTRUE);
   imp.SetDetectorPlugin(ClassName());
   imp.SetNameCorrespondanceList(fCorrespondanceFile.Data());
   // any additional structure name formatting definitions
   DefineStructureFormats(imp);

   // the following parameters are optimized for a 12-block compact
   // geometry placed at 80cm with rings 1-5 of INDRA removed.
   // make sure that the expected number of detectors get imported!
   imp.ImportGeometry(fImport_dTheta, fImport_dPhi, fImport_ThetaMin, fImport_PhiMin, fImport_ThetaMax, fImport_PhiMax);

   SetCalibrators();
   SetIdentifications();

   SetDetectorThicknesses();
   SetBit(kIsBuilt);

   if (run != -1) {
      SetParameters(run);
   }
}

void KVFAZIA::SortIDTelescopes()
{
   KVDetector* det = 0;
   TIter next(GetDetectors());
   while ((det = (KVDetector*)next())) {
      ((KVFAZIADetector*)det)->SortIDTelescopes();
   }

}
void KVFAZIA::GetDetectorEvent(KVDetectorEvent* detev, TSeqCollection* signals)
{
   // First step in event reconstruction based on current status of detectors in array.
   // Fills the given KVDetectorEvent with the list of all groups which have fired.
   // i.e. loop over all groups of the array and test whether KVGroup::Fired() returns true or false.
   //
   // If the list of fired acquisition parameters 'signals' is given, KVMultiDetArray::GetDetectorEvent
   // is called
   //

   if (signals) {
      // list of fired acquisition parameters given
      TIter next_par(signals);

      KVSignal* par = 0;
      KVDetector* det = 0;
      KVGroup* grp = 0;
      while ((par = (KVSignal*)next_par())) {
         if (!(par->GetN() > 0))
            Info("GetDetectorEvent", "%s empty", par->GetName());
         par->DeduceFromName();
//          if (!(det = GetDetector(par->GetDetectorName()))) {
//             det = GetDetector(KVFAZIADetector::GetNewName(par->GetDetectorName()));
//          }
         det = GetDetector(par->GetDetectorName());
         if (det) {
            ((KVFAZIADetector*)det)->SetSignal(par, par->GetType());
            if ((!(((KVFAZIADetector*)det)->GetSignal(par->GetType())->GetN() > 0)))
               Warning("Error", "%s %s empty signal is returned", det->GetName(), par->GetType());
            if ((grp = det->GetGroup())  && !detev->GetGroups()->FindObject(grp)) {
               detev->AddGroup(grp);
            }
         } else {
            Error("GetDetectedEvent", "Unknown detector %s !!!", par->GetDetectorName());
         }
      }
   } else {
      KVMultiDetArray::GetDetectorEvent(detev, 0);
   }

}


