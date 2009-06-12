#define KVSelector_cxx
// The class definition in KVSelector.h has been generated automatically
// by the ROOT utility TTree::MakeSelector().
//
// This class is derived from the ROOT class TSelector.
// The following members functions are called by the TTree::Process() functions:
//    Begin():       called everytime a loop on the tree starts,
//                   a convenient place to create your histograms.
//    Notify():      this function is called at the first entry of a new Tree
//                   in a chain.
//    Process():     called for each event. In this function you decide what 
//                   to read and you fill your histograms.
//    Terminate():   called at the end of a loop on the tree,
//                   a convenient place to draw/fit your histograms.
//
//   To use this file, try the following session on your Tree T
//
// Root > T->Process("KVSelector.C")
// Root > T->Process("KVSelector.C","some options")
// Root > T->Process("KVSelector.C+")
//
#include "KVSelector.h"
#include "TH2.h"
#include "TStyle.h"
#include "TString.h"
#include "TEnv.h"
#include "TSystem.h"
#include "TClass.h"
#ifdef __WITHOUT_TMACRO
#warning "KVDataSelector source files can not be automatically backed-up \
with this version of ROOT (no TMacro class). If you lose the sources for your \
selection, you will not be able to regenerate them."
#else
#include "TMacro.h"
#endif
#include "TROOT.h"
#include "KVBase.h"
#include "KVDataSet.h"
#include "KVAvailableRunsFile.h"
#include "TPluginManager.h"
#include "KVClassFactory.h"
#include "KVDataAnalyser.h"
#include "KVINDRAReconNuc.h"

KVString KVSelector::fBranchName = "INDRAReconEvent";
   
ClassImp(KVSelector)
//______________________________________________________________________________
//KVSelector
//
//Basic analysis class derived from TSelector for the analysis of TChains of
//KVINDRAReconEvent objects (ROOT files containing INDRA data).
//
//The user's analysis class, derived from KVSelector, must define the following
//methods:
//
//      void    InitAnalysis()
//
//              Called once at the beginning of the analysis.
//              Can be used to define histograms, TTrees, etc.
//              Also the place to define the list of global variables to be used.
//
//      void    InitRun();
//
//              Called at the beginning of each new run.
//              Define code masks for "good" particles here.
//
//      Bool_t  Analysis();
//
//              The analysis of the event.
//              Return value: kTRUE normally, kFALSE skips to the next run (?)
//
//      void    EndRun();
//
//              Called at the end of each run.
//
//      void    EndAnalysis();
//
//              Called at the end of the analysis.
//              Write histograms, TTrees to TFiles, etc.
//
//Use the static method KVSelector::Make("MySelector") in order to generate a
//template for your analysis class with the correct declarations for the above
//methods. MySelector.C and MySelector.h will be generated.
//
//Using Global Variables
//======================
//Many of the most frequently used global variables have been implemented as
//KaliVeda classes, all of which derive from the base class KVVarGlob. The KVSelector
//class provides a semi-automatic way to calculate and use these variables, via
//an internal list of global variables (KVGVList class). In order to use a
//global variable in your analysis, declare it in your InitAnalysis() method,
//its value will automatically be calculated for each event analysed.
//Note that only particles with KVINDRAReconNuc::IsOK()==kTRUE are included in the calculation.
//
//For example, let us suppose you want to calculate the isotropy ratio Riso for
//each event. In InitAnalysis() you should add the line:
//
//     AddGV( "KVRiso", "riso" );
//
//This will create a new KVRiso global variable object with the name "riso"
//(used to retrieve the object from the list when you want to know its value)
//and add it to the internal list of global variables to be calculated.
//
//In your event-by-event Analysis() method, you can retrieve the value of
//the global variable using e.g.
//
//     Double_t rapport = GetGV("riso")->GetValue();
//
//This gives the value of the isotropy ratio for the currently analysed event.
//
//
//More examples can be found in the AnalyseCamp1 analysis class.
//
void KVSelector::Begin(TTree * tree)
{
   // Function called before starting the event loop.
   // When running with PROOF Begin() is only called in the client.
   // Initialize the tree branches.

   if (fChain != tree) {
      Init(tree);
   }
//
// Get the option and the name of the DataSelector if needed
//
   TString option = GetOption();
   if (option.BeginsWith("DataSelector=")) {
      option.Remove(0, 13);
      SetDataSelector(option.Data());
   } else {
      SetDataSelector();
   }

//
// Builds a TEventList by adding the contents of the lists for each run
//
   BuildEventList();

   totentry = 0;

   InitAnalysis();              //user initialisations for analysis

   if (gvlist) {
      gvlist->Init();
   }

   if (fKVDataSelector)         // Init of the KVDataSelector if needed
   {
      fKVDataSelector->Init();
   }

   cout << endl << "Beginning processing of TChain :" << endl;
   fChain->ls();

   //start stopwatch, after first resetting it (in case this is not the first time the analysis is run)
   fTimer->Start(kTRUE);
}

void KVSelector::SlaveBegin(TTree * tree)
{
   // Function called before starting the event loop.
   // When running with PROOF SlaveBegin() is called in each slave
   // Initialize the tree branches.

   Init(tree);

   TString option = GetOption();

}

#ifdef __WITHOUT_TSELECTOR_LONG64_T
Bool_t KVSelector::Process(Int_t entry) //for ROOT versions < 4.01/01
#else
Bool_t KVSelector::Process(Long64_t entry)      //for ROOT versions > 4.00/08
#endif
{
   //Here the event is read into memory and checks are made on the number
   //of events read.
   //Particles not having the correct identification and calibration quality codes
   //are excluded from the analysis (IsOK()=kFALSE).
   //Particles which do not satisfy the additional selection criteria set with
   //SetParticleConditions() are excluded from the analysis (IsOK()=kFALSE).
   //If ChangeFragmentMasses() was called in InitAnalysis() or InitRun()
   //then the non-measured fragment masses will be recalculated with the
   //new mass formula and laboratory energies adjusted accordingly.
   //Then the reaction CM momenta of all "OK" particles are calculated and
   //the list of global variables (if one is defined) is filled.

   fTreeEntry = entry;

   if (!(totentry % 5000) && totentry)
      cout << " +++ " << totentry << " events processed +++ " << endl;

   fChain->GetTree()->GetEntry(fTreeEntry);

   //additional selection criteria ?
   if(fPartCond){
      KVNucleus* part=0;
      while( (part = (KVNucleus*)GetEvent()->GetNextParticle("ok")) ){
         
         part->SetIsOK( fPartCond->Test(part) );
         
      }
   }
   
   //change masses ?
   if( TestBit(kChangeMasses) ) GetEvent()->ChangeFragmentMasses(fNewMassFormula);
      
   totentry++;

   //calculate momenta of particles in reaction cm frame
   if (fCurrentRun->GetSystem()) {

       GetEvent()->SetFrame("CM",
                            fCurrentRun->GetSystem()->GetKinematics()->GetCMVelocity());

   }

   RecalculateGlobalVariables();


   Bool_t ok_anal = kTRUE;

   if (needToSelect) {
      ok_anal = fKVDataSelector->ProcessCurrentEntry(); //Data Selection and user analysis
   } else
      ok_anal = Analysis();     //user analysis

   // Testing whether EndRun() should be called
   if (AtEndOfRun()) {
      TString mes("End of run after ");
      mes += (totentry);
      mes += " events.";
      Info("Process", mes.Data());

      EndRun();                 //user routine end of run
      needToCallEndRun = kFALSE;

      // save the new Built TEventList
      if (needToSelect) {
         SaveCurrentDataSelection();
         needToSelect = kFALSE;
      }

      Info("Process", "delete data");
      delete data;
      data = 0;
      Info("Process", "delete gIndra");
      delete gIndra;            // Absolutely necessary to keep the coherence between
      gIndra = 0;               // the pointers to the detectors and the TRef's
      Info("Process", "delete OK");
   }

   return ok_anal;
}

void KVSelector::SlaveTerminate()
{
   // Function called at the end of the event loop in each PROOF slave.


}

void KVSelector::Terminate()
{
   // Function called at the end of the event loop.
   // When running with PROOF Terminate() is only called in the client.
   
   fTimer->Stop();//stop stopwatch straight away so that 'Events/CPU sec' etc. only
                           //includes time actually spent analysing data

   if (needToCallEndRun) {
      EndRun();
      delete data;
      data = 0;
      delete gIndra;            // Absolutely necessary to keep the coherence between
      gIndra = 0;               // the pointers to the detectors and the TRef's
   }

   if (needToSelect) {
      SaveCurrentDataSelection();
   }

   cout << endl << " ====================== END ====================== " <<
       endl << endl;
   cout << "   Total number of events read     = " << totentry << endl;
   cout << "   Real time = " << fTimer->RealTime() << " sec." << endl;
   cout << "    CPU time = " << fTimer->CpuTime() << " sec." << endl;
   cout << "   Events/Real sec. = " << totentry /
       fTimer->RealTime() << endl;
   cout << "    Events/CPU sec. = " << totentry /
       fTimer->CpuTime() << endl;
   cout << endl << " ====================== END ====================== " <<
       endl << endl;

   if (fEvtList) {
      fChain->SetEventList(0);
      delete fEvtList;
      fEvtList = 0;
   }

   EndAnalysis();               //user end of analysis routine
}

void KVSelector::Make(const Char_t * kvsname)
{
   // Automatic generation of KVSelector-derived class for KaliVeda analysis
   KVClassFactory cf(kvsname,"User analysis class","KVSelector",kTRUE);
   cf.AddImplIncludeFile("KVINDRAReconNuc.h");
   cf.AddImplIncludeFile("KVBatchSystem.h");
   cf.AddImplIncludeFile("KVINDRA.h");
   cf.GenerateCode();
}

void KVSelector::SetGVList(KVGVList * list)
{
   //Use a user-defined list of global variables for the analysis.
   //In this case it is the user's responsibility to delete the list
   //at the end of the analysis.
   gvlist = list;
}

KVGVList *KVSelector::GetGVList(void)
{
   //Access to the internal list of global variables
   //If the list does not exist, it is created.
   //In this case it will be automatically deleted with the KVSelector object.
   if (!gvlist) {
      gvlist = new KVGVList;
      SetBit(kDeleteGVList);
   }
   return gvlist;
}

void KVSelector::AddGV(KVVarGlob * vg)
{
   //Add the global variable "vg" to the list of variables for the analysis.
   //This is equivalent to GetGVList()->Add( vg ).
   if (!vg)
      Error("AddGV(KVVarGlob*)", "KVVarGlob pointer is null");
   else
      GetGVList()->Add(vg);
}

KVVarGlob *KVSelector::GetGV(const Char_t * name) const
{
   //Access the global variable with name "name" in the list of variables
   //for the analysis.
   //This is equivalent to GetGVList()->GetGV( name ).

   return (const_cast < KVSelector * >(this)->GetGVList()->GetGV(name));
}

KVVarGlob *KVSelector::AddGV(const Char_t * class_name,
                             const Char_t * name)
{
   //Add a global variable to the list of variables for the analysis.
   //
   //"class_name" must be the name of a valid class inheriting from KVVarGlob, e.g. any of the default global
   //variable classes defined as part of the standard KaliVeda package (in libKVvVarGlob.so). See
   //"Class Reference" page on website for the available classes (listed by category under "Global Variables: ...").
   //
   //USER-DEFINED GLOBAL VARIABLES
   //The user may use her own global variables in an analysis class, without having to add them to the main libraries.
   //If the given class name is not known, it is assumed to be a user-defined class and we attempt to compile and load
   //the class from the user's source code. For this to work, the user must:
   //
   //      (1) add to the ROOT macro path the directory where her class's source code is kept, e.g. in $HOME/.rootrc
   //              add the following line:
   //
   //              +Unix.*.Root.MacroPath:      :$(HOME)/myVarGlobs
   //
   //      (2) for each user-defined class, add a line to $HOME/.kvrootrc to define a "plugin". E.g. for a class called MyNewVarGlob,
   //
   //              +Plugin.KVVarGlob:    MyNewVarGlob    MyNewVarGlob     MyNewVarGlob.cpp+   "MyNewVarGlob()"
   //
   //      It is assumed that MyNewVarGlob.h and MyNewVarGlob.cpp will be found in $HOME/myVarGlobs (in this example).
   //
   //"name" is a unique name for the new global variable object which will be created and added to the internal
   //list of global variables. This name can be used to retrieve the object (see GetGV) in the user's analysis.
   //
   //Returns pointer to new global variable object in case more than the usual default initialisation is necessary.

   KVVarGlob *vg = 0;
   TClass *clas = gROOT->GetClass(class_name);
   if (!clas) {
      //class not in dictionary - user-defined class ? Look for plugin.
      TPluginHandler *ph = KVBase::LoadPlugin("KVVarGlob", class_name);
      if (!ph) {
         //not found
         Error("AddGV(const Char_t*,const Char_t*)",
               "Called with class_name=%s.\nClass is unknown: not in standard libraries, and plugin (user-defined class) not found",
               class_name);
         return 0;
      } else {
         vg = (KVVarGlob *) ph->ExecPlugin(0);
      }
   } else if (!clas->InheritsFrom("KVVarGlob")) {
      Error("AddGV(const Char_t*,const Char_t*)",
            "%s is not a valid class deriving from KVVarGlob.",
            class_name);
      return 0;
   } else {
      vg = (KVVarGlob *) clas->New();
   }
   vg->SetName(name);
   AddGV(vg);
   return vg;
}

//____________________________________________________________________________

void KVSelector::RecalculateGlobalVariables()
{
   //Use this method if you change e.g. the acceptable particle identification codes in your
   //Analysis() method and want to recalculate the values of all global variables
   //for your new selection.
   //
   //WARNING: the global variables are calculated automatically for you for each event
   //before method Analysis() is called. In order for the correct particles to be included in
   //this calculation, make sure that at the END of Analysis() you reset the selection
   //criteria.
   //
   //i.e. if in your InitAnalysis() you have:
   //      GetEvent()->AcceptIDCodes(kIDCode2|kIDCode3|kIDCode4);
   //
   //at the beginning of Analysis() the variables have been calculated for all particles
   //with codes 2-4. If you store these values somewhere and then change the code mask:
   //
   //      GetEvent()->AcceptIDCodes(kIDCode2|kIDCode3|kIDCode4|kIDCode6);
   //
   //you can recalculate the global variable list for particles with codes 2-4 & 6:
   //
   //      RecalculateGlobalVariables();
   //
   //then at the end of Analysis(), after storing the new values of the variables, you
   //should reset the code mask:
   //
   //      GetEvent()->AcceptIDCodes(kIDCode2|kIDCode3|kIDCode4);
   //
   //which will be used for the next event read for processing.

   if (gvlist) {
      // 1st step: Reset global variables
      gvlist->Reset();

      //2nd step: loop over particles with correct codes
      //          and fill global variables
      KVINDRAReconNuc *n1 = 0;
      // calculate 1-body variables
      if( gvlist->Has1BodyVariables() ){
         while ((n1 = GetEvent()->GetNextParticle("ok"))) {
            gvlist->Fill(n1);
         }
      }
      KVINDRAReconNuc *n2 = 0;
      // calculate 2-body variables
      // we use every pair of particles (including identical pairs) in the event
      if( gvlist->Has2BodyVariables() ){
         Int_t N = GetEvent()->GetMult();
         for( int i1 = 1; i1 <= N ; i1++ ){
            for( int i2 = 1 ; i2 <= N ; i2++ ){
               n1 = GetEvent()->GetParticle(i1);
               n2 = GetEvent()->GetParticle(i2);
               if( n1->IsOK() && n2->IsOK() )
                  gvlist->Fill2(n1,n2);
            }
         }
      }
      // calculate N-body variables
      if( gvlist->HasNBodyVariables() ) gvlist->FillN( GetEvent() );
   }
}

//____________________________________________________________________________

#ifdef __WITHOUT_TSELECTOR_LONG64_T
Int_t KVSelector::GetTreeEntry() const
#else
Long64_t KVSelector::GetTreeEntry() const
#endif
{
   //During event analysis (i.e. inside the Analysis() method), this gives the current TTree
   //entry number, in other words the argument passed to TSelector::Process(Long64_t entry).

   return fTreeEntry;
}

//____________________________________________________________________________
void KVSelector::BuildEventList(void)
{
//
// Builds the event list of the TChain by adding the event lists of each TTree.
// The event list contain the entry number of the TChain
//

   if (fEvtList) {
      fEvtList->Reset();
      delete fEvtList;
      fEvtList = 0;
   }
   if (fKVDataSelector) {
      delete fKVDataSelector;
      fKVDataSelector = 0;
   }
   if (fTEVLexist) {
      delete[]fTEVLexist;
      fTEVLexist = 0;
   }

   if (fDataSelector.Length()) {
      Info("BuildEventList()",
           Form("Building TEventList for the KVDataSelector \"%s\".",
                GetDataSelector()));
      TObjArray *lof = 0;
      Long64_t *toff = 0;
      if (fTreeOffset) {
         cout << "Analysis from a chain of trees." << endl;
         lof = ((TChain *) fChain)->GetListOfFiles();
         toff = fTreeOffset;
      } else {
         cout << "Analysis from a single tree." << endl;
         lof = new TObjArray();
         lof->
             Add(new
                 TNamed("SingleRun", fChain->GetCurrentFile()->GetName()));
         toff = new Long64_t[2];
         toff[0] = 0;
         toff[1] = fChain->GetEntries();
      }

      fEvtList =
          new TEventList("chainEventList", "TEvent list for this TChain");


      // Opening the file which contains the TEventLists
      TString searchname(GetDataSelectorFileName());
      TString fname = searchname;
      cout << "File to open : " << fname.Data() << endl;
      TFile *fileDataSelector = 0;
      if (KVBase::FindFile("", searchname)) {
         //get lock on file to avoid interference with other processes
         if(dataselector_lock.Lock(fname.Data())) fileDataSelector = new TFile(fname.Data());
      } else {
         cout << "The file \"" << fname.
             Data() << "\" does not exist." << endl;
      }

      fTEVLexist = new Bool_t[lof->GetEntries()];
      for (Int_t tn = 0; tn < lof->GetEntries(); tn++) {
         fTEVLexist[tn] = kFALSE;
         TString fname(((TNamed *) lof->At(tn))->GetTitle());
         cout << fname.Data() << endl;
         Int_t nrun = GetRunNumberFromFileName(fname.Data());
         if (nrun) {
            cout << "Numero de run " << nrun << endl;
            cout << "Recherche de " << Form("%s_run%d;1",
                                            GetDataSelector(),
                                            nrun) << endl;
            TEventList *revtList = 0;
            if (fileDataSelector) {
               revtList =
                   (TEventList *) fileDataSelector->
                   Get(Form("%s_run%d;1", GetDataSelector(), nrun));
            }
            if (revtList) {
               for (Int_t i = 0; i < revtList->GetN(); i++) {
                  fEvtList->Enter(fTreeOffset[tn] + revtList->GetEntry(i));
               }
               fTEVLexist[tn] = kTRUE;
            } else {
               cout << "Liste introuvable..." << endl;
               for (Int_t i = fTreeOffset[tn]; i < fTreeOffset[tn + 1];
                    i++) {
                  fEvtList->Enter(i);
               }
            }
         } else {
            cout << "Run inconnu..." << endl;
         }
      }
      if (fileDataSelector) {
         cout << "Closing " << fileDataSelector->GetName() << endl;
         fileDataSelector->Close();
         dataselector_lock.Release();//unlock file
      }

      if (!fTreeOffset) {
         delete[]toff;
         lof->SetOwner(kTRUE);
         delete lof;
      }
      // Check if the TEventList is empty. If yes, all events will be read
      if (!fEvtList->GetN()) {
         Warning("BuildEventList()", "The TEventList is empty...");
         delete fEvtList;
         fEvtList = 0;
      }
   }

   fChain->SetEventList(fEvtList);
}

//____________________________________________________________________________
Bool_t KVSelector::AtEndOfRun(void)
{
//
// Check whether the end of run is reached for the current tree
//   

   Bool_t ok = (fTreeEntry + 1 == fChain->GetTree()->GetEntries());

   if (fEvtList) {
      Long64_t globEntry = 0;
      if (fTreeOffset)
         globEntry = ((TChain *) fChain)->GetChainEntryNumber(fTreeEntry);
      else
         globEntry = fTreeEntry;
      Int_t index = fEvtList->GetIndex(globEntry);
      Long64_t nextEntry = (fEvtList->GetEntry(index + 1));
      ok = ok || (nextEntry == -1);
      if (fTreeOffset) {
         ok = ok || (nextEntry >= fTreeOffset[fCurrentTreeNumber + 1]);
      }
   }

   return ok;
}

//____________________________________________________________________________
void KVSelector::LoadDataSelector(void)
{
//
// Set the pointer of the KVDataSelector according to its name
//
   cout << "Loading KVDataSelector..." << endl;
   Bool_t deleteSources = kFALSE;
   if (fDataSelector.Length()) {
      fKVDataSelector = 0;
      TClass *clas = gROOT->GetClass(fDataSelector.Data());
      cout << clas << " / " << fDataSelector.Data() << endl;
      if (!clas) {
         //if the class is not present, first check whether the declaration and
         //the implementation files are present in the user's working directory
         TString fileC(Form("%s.cpp", fDataSelector.Data()));
         TString fileh(Form("%s.h", fDataSelector.Data()));
         
#ifdef __WITHOUT_TMACRO
         if (gSystem->AccessPathName(fileC.Data()) || gSystem->AccessPathName(fileh.Data())){
               Warning("LoadDataSelector(void)",
                       Form
                       ("No implementation and/or declaration file found for \"%s\".",
                        fDataSelector.Data()));            
#else
         TMacro mC;
         if (!mC.ReadFile(fileC.Data()) || !mC.ReadFile(fileh.Data())) {
            
            // Load .cpp and .h files from the TEventList's root file
            TString searchname(GetDataSelectorFileName());
            TString fname = searchname;
            TFile *fileDataSelector = 0;
            
            if (KVBase::FindFile("", fname)) {
               if(dataselector_lock.Lock(fname.Data())) fileDataSelector = new TFile(fname.Data());
            } else {
               cout << "The file \"" << fname.
                   Data() << "\" does not exist." << endl;
            }
            TMacro *macC = 0;
            TMacro *mach = 0;
            if (fileDataSelector) {
               macC = (TMacro *) fileDataSelector->Get(fileC.Data());
               mach = (TMacro *) fileDataSelector->Get(fileh.Data());
            }
            if (macC && mach) {
               macC->SaveSource(fileC.Data());
               cout << fileC.Data() << " re-generated" << endl;
               mach->SaveSource(fileh.Data());
               cout << fileh.Data() << " re-generated." << endl;
               deleteSources = kTRUE;
            } else {
               Warning("LoadDataSelector(void)",
                       Form
                       ("No implementation and/or declaration file found for \"%s\".",
                        fDataSelector.Data()));
            }
            if(fileDataSelector){
               fileDataSelector->Close();
               dataselector_lock.Release();
            }
#endif
         } else {
            cout << "Files " << fileC.Data() << " and " << fileh.Data() <<
                " found." << endl;
         }
         //class not in dictionary - user-defined class ? Add a plugin.
         gROOT->GetPluginManager()->AddHandler("KVDataSelector",
                                               fDataSelector.Data(),
                                               fDataSelector.Data(),
                                               Form("%s.cpp+",
                                                    fDataSelector.Data()),
                                               Form("%s()",
                                                    fDataSelector.Data()));
         TPluginHandler *ph =
             KVBase::LoadPlugin("KVDataSelector", fDataSelector.Data());
         if (!ph) {
            //not found
            Error("LoadDataSelector(void)",
                  "Called with class_name=%s.\nClass is unknown: not in standard libraries, and plugin (user-defined class) not found",
                  fDataSelector.Data());
         } else {
            fKVDataSelector = (KVDataSelector *) ph->ExecPlugin(0);
            fKVDataSelector->SetKVSelector(this);
            cout << "Apres Plugin " << fKVDataSelector->IsA()->
                GetName() << " : " << fKVDataSelector->
                GetKVSelector() << endl;
         }
         
      } 
      else if (!clas->InheritsFrom("KVDataSelector")) {
         Error("LoadDataSelector(void)",
               "%s is not a valid class deriving from KVDataSelector.",
               fDataSelector.Data());
      } else {
         fKVDataSelector = (KVDataSelector *) clas->New();
         fKVDataSelector->SetKVSelector(this);
         cout << "Apres clas->New() " << fKVDataSelector->IsA()->
             GetName() << " : " << fKVDataSelector->
             GetKVSelector() << endl;
      }
      if (deleteSources) {
         cout << "Removing files \"" << fDataSelector.
             Data() << "*.*\" ..." << endl;
         gSystem->
             Exec(Form
                  ("rm %s.[c,h]*", fDataSelector.Data(),
                   fDataSelector.Data()));
      }
   } else {
      fKVDataSelector = 0;
   }
   if (fKVDataSelector)
      fKVDataSelector->Init();
}

//____________________________________________________________________________
void KVSelector::SaveCurrentDataSelection(void)
{
//
// Save the newly built TEventList in the DataSelection root file
//
   if (fKVDataSelector) {
      TFile *curfile = gFile;
      TString option;
      TString searchname(GetDataSelectorFileName());
      TString fname = searchname;
      if (KVBase::FindFile("", searchname)) {
         option = "update";
      } else {
         option = "recreate";
      }
      cout << "File to open : " << fname.Data() << " : " << option.
          Data() << endl;
      TFile *fileDataSelector = 0;
      if(dataselector_lock.Lock(fname.Data())) fileDataSelector = new TFile(fname.Data(), option.Data());
      if (!fileDataSelector || (fileDataSelector && fileDataSelector->IsZombie())) {
         Error("SaveCurrentDataSelection(void)",
               Form
               ("Opening the file \"%s\" with the option \"%s\" is not possible.\nNothing saved.",
                fname.Data(), option.Data()));
         gFile = curfile;
         return;
      }
      cout << "Saving the TEventList \"" << fKVDataSelector->
          GetTEventList()->GetName() << "\"..." << endl;
      cout << fKVDataSelector->GetTEventList()->
          GetN() << " entries selected." << endl;
      fKVDataSelector->GetTEventList()->Write();
      
#ifndef __WITHOUT_TMACRO
      TObject
          * oC =
          fileDataSelector->
          Get(Form("%s.cpp;1", fKVDataSelector->IsA()->GetName()));
      TObject * oh =
          fileDataSelector->
          Get(Form("%s.h;1", fKVDataSelector->IsA()->GetName()));
      if (!oC || !oh) {
         cout << "Saving the source files..." << endl;
         TMacro mC(Form("%s.cpp", fKVDataSelector->IsA()->GetName()));
         mC.SetName(Form("%s.cpp", fKVDataSelector->IsA()->GetName()));
         mC.SetTitle(Form
                     ("Implementation file for the KVDataSelector \"%s\".",
                      fKVDataSelector->IsA()->GetName()));
         mC.Write();
         TMacro mh(Form("%s.h", fKVDataSelector->IsA()->GetName()));
         mh.SetName(Form("%s.h", fKVDataSelector->IsA()->GetName()));
         mh.SetTitle(Form
                     ("Declaration file for the KVDataSelector \"%s\".",
                      fKVDataSelector->IsA()->GetName()));
         mh.Write();
      }
#endif
      
      cout << "Done" << endl;
      fileDataSelector->Close();
      dataselector_lock.Release();
      gFile = curfile;
   } else {
      cout << "No TEventList to save..." << endl;
   }
}

//____________________________________________________________________________
Int_t KVSelector::GetRunNumberFromFileName(const Char_t * fileName)
{
   // Get the run number from the current tree file
   
   KVAvailableRunsFile *arf;
   //get type of data being analysed from analyser running the analysis
   arf = gDataSet->GetAvailableRunsFile( gDataAnalyser->GetDataType().Data() );
   return arf->IsRunFileName(fileName);
}

//____________________________________________________________________________
const Char_t *KVSelector::GetDataSelectorFileName(void)
{
//
// Gets the name of the file where the TEventLists and the KVDataSelectors 
// are stored.
//
   static TString fname;
   fname = "";
   fname =
       gEnv->GetValue("DataSelector.directory",
                      gSystem->WorkingDirectory());
   fname += "/";
   fname +=
       gEnv->GetValue("DataSelector.fileName", "ListOfDataSelector.root");
   cout << "List Of Data Selectors : " << fname.Data() << endl;
   return fname.Data();
}

//____________________________________________________________________________
   
void KVSelector::ChangeFragmentMasses(UInt_t mass_formula)
{
   //Call this method in your InitAnalysis() if you want to replace the masses of nuclei
   //in each event with masses calculated from the given formula (see KVNucleus::GetAFromZ).
   //
   //As each event is read, the masses of nuclei with calculated (not measured) masses
   //will be replaced with the new value, and the energy of the nucleus will be replaced with
   //a new value taking into account the effect of the change in mass on the CsI energy
   //calibration (if the particle stops in a CsI detector).
   //
   //NB: the "CM" frame momenta/energies/angles calculated automatically before your
   //Analysis() method is called will be calculated using the new masses and lab energies.
   //Any subsequent change to the masses and/or energies you make in Analysis() will not
   //change the "CM" momenta. If you change the laboratory energies etc. you must recalculate
   //the c.m. momenta by calling e.g. GetEvent()->SetFrame("CM", ... )
   SetBit(kChangeMasses);
   fNewMassFormula = mass_formula;
}

//____________________________________________________________________________
   
void KVSelector::SetParticleConditions(const KVParticleCondition& cond)
{
   //Use this method to set criteria for selecting particles (other than the identification
   //or calibration quality codes - see KVINDRAReconEvent::AcceptIDCodes and
   //KVINDRAReconEvent::AcceptECodes).
   //
   //The criteria defined in the KVParticleCondition object will be applied to every
   //particle which has the right quality codes and if they are not satisfied the particle's
   //"OK" flag will be set to false, i.e. the particle's IsOK() method will return kFALSE,
   //and the particle will not be included in iterations such as GetEvent()->GetNextParticle("OK").
   //Neither will the particle be included in the evaluation of any global variables or event
   //selection criteria (see KVDataSelector).
   //
   //This method must be called in the user's InitAnalysis() or InitRun() method.
   //
   //EXAMPLES:
   //To include only particles having a time marker between 90 and 110:
   //
   //       KVParticleCondition cd("_NUC_->GetTimeMarker()>=90 && _NUC_->GetTimeMarker()<=110");
   //       SetParticleConditions( cd );
   //
   //To include only Z=1 particles having a time marker between 90 and 110, Z>1 particles
   //with time markers between 80 and 120:
   //
   //       KVParticleCondition tm1("_NUC_->GetTimeMarker()>=90 && _NUC_->GetTimeMarker()<=110");
   //       KVParticleCondition tm2("_NUC_->GetTimeMarker()>=80 && _NUC_->GetTimeMarker()<=120");
   //       KVParticleCondition z1("_NUC_->GetZ()==1");
   //       KVParticleCondition zgt1("_NUC_->GetZ()>1");
   //       KVParticleCondition cd = (z1 && tm1) || (zgt1 && tm2);
   //       SetParticleConditions( cd );
   if(!fPartCond) fPartCond = new KVParticleCondition(cond);
   else *fPartCond = cond;
   //set name of class to which we cast. this is for optimization to work
   fPartCond->SetParticleClassName("KVINDRAReconNuc");
}

//____________________________________________________________________________
   
void KVSelector::CheckIfINDRAUpdateRequired()
{
   //  Updating calibration/identification parameters contained in data files.
   //  When recon/ident data files are created, a KVINDRA object containing all of the
   //  currently available identification & calibration parameters is written in the file.
   //  During subsequent analysis of these files (e.g. during recon->ident transformation)
   //  it is the parameters stored in the file which are used.
   //  If you want to use different parameters (e.g. because those in the file have become
   //  obsolete), it is possible by setting the following variables:
   // 
   //  [name of dataset].[data file type].UpdateIdentifications:    yes
   //  [name of dataset].[data file type].UpdateIDTelescopes:    yes
   //  [name of dataset].[data file type].UpdateCalibrations:    yes
   // 
   //  For example, to update the identification parameters during analysis of 'recon'
   //  data from the 5th INDRA campaign, add the following line in your .kvrootrc:
   // 
   //  INDRA_camp5.recon.UpdateIdentifications:   yes
   // 
   //  In this example, if a recon->ident task is launched, the identification parameters
   //  stored in the file will be ignored, and new identification parameters will be set
   //  according to the current values of INDRA_camp5.ActiveIdentifications and the
   //  variables giving the names of identification parameter files (see above).
   // 
   //  N.B. the parameters in the file being analysed will not be modified. However,
   //  if you do e.g. recon->ident with 'Update' activated, the INDRA object in the new
   //  'ident' file will contain the update parameters.
      
   Bool_t id_up = gDataSet->GetDataSetEnv( Form("%s.UpdateIdentifications",
      gDataAnalyser->GetDataType().Data()), kFALSE );
   Bool_t idtel_up = gDataSet->GetDataSetEnv( Form("%s.UpdateIDTelescopes",
      gDataAnalyser->GetDataType().Data()), kFALSE );
   Bool_t cal_up = gDataSet->GetDataSetEnv( Form("%s.UpdateCalibrations",
      gDataAnalyser->GetDataType().Data()), kFALSE );
   //update identification telescopes ?
   if(idtel_up){
      cout << endl << " +++ UPDATING IDENTIFICATION TELESCOPES OF INDRA +++ "<< endl << endl; 
      gIndra->UpdateIDTelescopes();
      gIndra->SetRunIdentificationParameters( fCurrentRun->GetNumber() );
   }
   //update identification parameters ?
   else if(id_up){
      cout << endl << " +++ UPDATING IDENTIFICATION PARAMETERS OF INDRA +++ "<< endl << endl; 
      gIndra->UpdateIdentifications();
      gIndra->SetRunIdentificationParameters( fCurrentRun->GetNumber() );
   }
   //update calibrations ?
   if(cal_up){
      cout << endl << " +++ UPDATING CALIBRATION PARAMETERS OF INDRA +++ " << endl<< endl; 
      gIndra->UpdateCalibrators();
      gIndra->SetRunCalibrationParameters( fCurrentRun->GetNumber() );
   }
}