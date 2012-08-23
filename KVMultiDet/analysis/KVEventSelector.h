//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Fri Apr  1 10:33:24 2011 by ROOT version 5.29/01
// from TTree events/events
// found on file: events.root
//////////////////////////////////////////////////////////

#ifndef KVEventSelector_h
#define KVEventSelector_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TSelector.h>
#include "KVEvent.h"
#include "KVGVList.h"
#include "KVString.h"
#include "KVParticleCondition.h"
#include "TDatime.h"
#include "KVHashList.h"
#include <TH3.h>
#include <TH2.h>
#include <TProfile2D.h>
#include "KVParameterList.h"

class KVEventSelector : public TSelector {

protected :
   enum {
      kDeleteGVList = BIT(14),
   };

   TTree          *fChain;   //!pointer to the analyzed TTree or TChain

   // Declaration of leaf types
   KVEvent     *Event;

   // List of branches
   TBranch        *b_Event;   //!

   //List of global variables
   KVGVList *gvlist;            //!

   KVString fBranchName; //name of branch which contains events to analyse

   KVParticleCondition* fPartCond;//(optional) conditions for selecting particles

   Bool_t fFirstEvent;//set to kFALSE after first event is read

   Long64_t fEventsRead;//number of events read
   Long64_t fEventsReadInterval;//interval at which to print number of events read
   Long64_t fTreeEntry;//current tree entry number

   KVHashList* lhisto;           //->!
   KVHashList* ltree;            //->!

   Bool_t fNotifyCalled; // avoid multiple calls to Notify/InitRun
   
   //parsed list of options given to TTree::Process
   KVParameterList<KVString> fOptionList;

   void FillTH1(TH1* h1, Double_t one, Double_t two);
   void FillTProfile(TProfile* h1, Double_t one, Double_t two, Double_t three);
   void FillTH2(TH2* h2, Double_t one, Double_t two, Double_t three);
   void FillTProfile2D(TProfile2D* h2, Double_t one, Double_t two, Double_t three, Double_t four);
   void FillTH3(TH3* h3, Double_t one, Double_t two, Double_t three, Double_t four);
   
   void ParseOptions();

public:
   KVEventSelector(TTree * /*tree*/ = 0) : gvlist(0), fBranchName("data"), fPartCond(0), fFirstEvent(kTRUE),
      fEventsRead(0), fEventsReadInterval(100), fNotifyCalled(kFALSE) {
      lhisto = new KVHashList();
      ltree = new KVHashList();
   }
   virtual ~KVEventSelector() {
      //delete global variable list if it belongs to us, i.e. if created by a call to GetGVList
      if (TestBit(kDeleteGVList)) {
         delete gvlist;
         gvlist = 0;
         ResetBit(kDeleteGVList);
      }
      SafeDelete(fPartCond);
      lhisto->Clear();
      delete lhisto;
      lhisto = 0;
      ltree->Clear();
      delete ltree;
      ltree = 0;
   };
   void SetEventsReadInterval(Long64_t N) { fEventsReadInterval = N; };
   virtual Int_t   Version() const {
      return 2;
   }
   virtual void    Begin(TTree *tree);
   virtual void    SlaveBegin(TTree *tree);
   virtual void    Init(TTree *tree);
   virtual Bool_t  Notify();
   virtual Bool_t  Process(Long64_t entry);
   virtual Int_t   GetEntry(Long64_t entry, Int_t getall = 0) {
      return fChain ? fChain->GetTree()->GetEntry(entry, getall) : 0;
   }
   virtual void    SetObject(TObject *obj) {
      fObject = obj;
   }
   virtual void    SetInputList(TList *input) {
      fInput = input;
   }
   virtual TList  *GetOutputList() const {
      return fOutput;
   }
   virtual void    SlaveTerminate();
   virtual void    Terminate();

   void SetBranchName(const Char_t* n) {
      fBranchName = n;
   };
   const Char_t* GetBranchName() const {
      return fBranchName.Data();
   };
   virtual void SetAnalysisFrame()
   {
   };

   KVEvent* GetEvent() const {
      return Event;
   }

   /* user entry points */
   virtual void InitAnalysis() {
      AbstractMethod("InitAnalysis");
   };
   virtual void InitRun() {
      AbstractMethod("InitRun");
   };
   virtual Bool_t Analysis() {
      //AbstractMethod("Analysis");
      return kTRUE;
   };
   virtual void EndRun() {
      AbstractMethod("EndRun");
   };
   virtual void EndAnalysis() {
      AbstractMethod("EndAnalysis");
   };
   //handling global variables for analysis
   virtual void SetGVList(KVGVList * list) {
      //Use a user-defined list of global variables for the analysis.
      //In this case it is the user's responsibility to delete the list
      //at the end of the analysis.
      gvlist = list;
   };
   virtual KVGVList *GetGVList(void) const {
      //Access to the internal list of global variables
      //If the list does not exist, it is created.
      //In this case it will be automatically deleted with the KVSelector object.
      if (!gvlist) {
         const_cast < KVEventSelector *>(this)->gvlist = new KVGVList;
         const_cast < KVEventSelector *>(this)->SetBit(kDeleteGVList);
      }
      return gvlist;
   };
   virtual void AddGV(KVVarGlob *vg) {
      //Add the global variable "vg" to the list of variables for the analysis.
      //This is equivalent to GetGVList()->Add( vg ).
      if (!vg)
         Error("AddGV(KVVarGlob*)", "KVVarGlob pointer is null");
      else
         GetGVList()->Add(vg);
   };
   virtual KVVarGlob *AddGV(const Char_t * class_name, const Char_t * name);
   virtual KVVarGlob *GetGV(const Char_t *name) const {
      //Access the global variable with name "name" in the list of variables
      //for the analysis.
      //This is equivalent to GetGVList()->GetGV( name ).

      return GetGVList()->GetGV(name);
   };
   virtual void RecalculateGlobalVariables();
   Bool_t AtEndOfRun(void) {
      // Check whether the end of run is reached for the current tree
      return (fTreeEntry + 1 == fChain->GetTree()->GetEntries());
   };

   virtual void SetParticleConditions(const KVParticleCondition&);

   virtual void CreateHistos();
   virtual void CreateTrees();

   void FillHisto(KVString sname, Double_t one, Double_t two = 1, Double_t three = 1, Double_t four = 1);
   void FillTree(KVString sname = "");

   KVHashList* GetHistoList();
   KVHashList* GetTreeList();

   TH1* GetHisto(const Char_t* name);
   TTree* GetTree(const Char_t* name);

   virtual void WriteHistoToFile(KVString filename = "FileFromKVSelector.root", Option_t* option = "recreate");
   virtual void WriteTreeToFile(KVString filename = "FileFromKVSelector.root", Option_t* option = "recreate");

   virtual void SetOpt(const Char_t* option, const Char_t* value);   
   virtual Bool_t IsOptGiven(const Char_t* option);
   virtual KVString& GetOpt(const Char_t* option) const;
   virtual void UnsetOpt(const Char_t* opt);
   
   ClassDef(KVEventSelector, 0);
};

#endif

#ifdef KVEventSelector_cxx
void KVEventSelector::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set object pointer
   Event = 0;
   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fChain->SetMakeClass(1);

   Info("Init", "Analysing data in branch : %s", GetBranchName());
   fChain->SetBranchAddress(GetBranchName() , &Event, &b_Event);

   TDatime now;
   Info("Init", "Analysis begins at %s", now.AsString());

   InitAnalysis();              //user initialisations for analysis
}

Bool_t KVEventSelector::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   if (fNotifyCalled) return kTRUE; // avoid multiple calls at beginning of analysis
   fNotifyCalled = kTRUE;

   Info("Notify", "Beginning analysis of file %s (%lld events)", fChain->GetCurrentFile()->GetName(), fChain->GetTree()->GetEntries());

   InitRun();                   //user initialisations for run

   return kTRUE;
}

#endif // #ifdef KVEventSelector_cxx
