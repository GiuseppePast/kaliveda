/*
$Id: KVDataSet.cpp,v 1.41 2009/03/11 14:22:41 franklan Exp $
$Revision: 1.41 $
$Date: 2009/03/11 14:22:41 $
$Author: franklan $
*/

#include "TROOT.h"
#include "TMethodCall.h"
#include "KVDataSet.h"
#include "KVDataRepository.h"
#include "KVDataRepositoryManager.h"
#include "KVDataSetManager.h"
#include "KVMultiDetArray.h"
#include "KVDataBase.h"
#include "TSystem.h"
#include "KVDataAnalysisTask.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "KVDBSystem.h"
#include "KVDBRun.h"
#include "TEnv.h"
#include "KVUpDater.h"
#include "KVAvailableRunsFile.h"
#include "KVNumberList.h"
#include "TPluginManager.h"

ClassImp(KVDataSet)
/////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVDataSet</h2>
<h4>Describes a set of experimental data which may be analysed with KaliVeda</h4>
<p>A dataset brings together all the elements necessary for the
analysis of data taken during an experiment or campaign of experiments
using the same experimental configuration. These are:
</p>
<ul>
  <li>the files containing the data in its raw state</li>
  <li>files containing partially and/or completely calibrated data</li>
  <li>a description of the experimental set-up (detectors, geometry, etc.)</li>
  <li>a database describing the different runs &amp; reaction systems
studied, detector parameters (status, calibration parameters, etc.)</li>
</ul>

<h3><a name="init"></a>I. Initialisation</h3>

<p>Different datasets are listed in the configuration file <span
 style="font-family: monospace;">$KVROOT/KVFiles/.kvrootrc</span>. This file is read by
 <a style="font-family: monospace;" href="KVDataSetManager.html#KVDataSetManager:Init">KVDataSetManager::Init()</a>,
which sets up and handles all the datasets for a given data repository
(see <a href="KVDataRepository.html"><span
 style="font-family: monospace;">KVDataRepository</span></a>).
The syntax for adding a dataset to this list is described in the file.
</p>
<p>After creation, the availability of this dataset for analysis is
decided based on whether data files associated with it are present or
not in the data repository, and, optionally, whether the user's name
appears in the associated user group (see <a
 style="font-family: monospace;" href="#KVDataSet:CheckAvailable">KVDataSet::CheckAvailable()</a>).
Datasets which are available for analysis by the user have <span
 style="font-family: monospace;"><a href="#KVDataSet:IsAvailable">IsAvailable()</a>==kTRUE</span>.
</p>
<p>All possible <a href="KVDataAnalysisTask.html">data analysis tasks</a>
for this dataset are then determined based on the available types of
data in the repository (see <a style="font-family: monospace;"
 href="#KVDataSet:SetAnalysisTasks">SetAnalysisTasks()</a>) and the
default list of known analysis tasks (see <a
 style="font-family: monospace;"
 href="KVDataSetManager.html#KVDataSetManager:ReadTaskList">KVDataSetManager::ReadTaskList()</a>).
Data analysis tasks can be chosen &amp; performed for any available
dataset by using a <a style="font-family: monospace;"
 href="KVDataAnalyser.html">KVDataAnalyser</a>.
</p>

<h3><a name="what"></a>II. What's In A Dataset ?</h3>

<h4>a. Experimental set-up: multidetector array</h4>
<p>The experimental set-up for each dataset is handled by a child class
of <a style="font-family: monospace;" href="KVMultiDetArray.html">KVMultiDetArray</a>,
which can describe anything from a single detector
to a 4&#960; (pi)-multidetector of charged particles such as INDRA. Each
dataset is uniquely associated to its experimental set-up by defining a
<a href="http://root.cern.ch/root/html/TPluginManager.html">plugin</a>
of the <span style="font-family: monospace;">KVMultiDetArray</span>
class:
</p>
<pre>+Plugin.KVMultiDetArray: name_of_dataset  name_of_class  name_of_plugin_library  constructor_to_call</pre>
<p>See file <span style="font-family: monospace;">$KVROOT/KVFiles/.kvrootrc</span>
for examples concerning the default datasets.
There you will also see that the multidetector class can be further
"tweaked" by defining dataset-dependent plugins for the <a
 href="KVIDTelescope.html"><span style="font-family: monospace;">KVIDTelescope</span></a>
class (see <a
 href="KVMultiDetArray.html#KVMultiDetArray:GetIDTelescopes"><span
 style="font-family: monospace;">KVMultiDetArray::GetIDTelescopes</span></a>).
</p>
<p>The experimental set-up for the dataset can be created when
necessary by using <a href="#KVDataSet:BuildMultiDetector"><span
 style="font-family: monospace;">KVDataSet::BuildMultiDetector()</span></a>.
</p>

<h4>b. Runs, systems and calibration parameter database</h4>

<p>A child class of <a href="KVDataBase.html"><span
 style="font-family: monospace;">KVDataBase</span></a> handles all
stored information on the different experimental runs, reaction systems
studied, run-dependent detector characteristics, calibration
parameters, etc. The link between dataset and database is defined by a <a
 href="http://root.cern.ch/root/html/TPluginManager.html">plugin</a>:
</p>

<pre>+Plugin.KVDataBase: name_of_dataset  name_of_class  name_of_plugin_library  constructor_to_call</pre>

<p>See file <span style="font-family: monospace;">$KVROOT/KVFiles/.kvrootrc</span>
for examples concerning the default datasets.</p>

<p>Such databases are constructed from a number of different source
files (runlist, system list, etc.) which are kept in the directory <span
 style="font-family: monospace;">$KVROOT/KVFiles/name_of_dataset</span>
(string returned by <a style="font-family: monospace;"
 href="#KVDataSet:GetDataSetDir">GetDataSetDir()</a>).
The database object, once constructed from these files (see <a
 href="KVDataBase.html#KVDataBase:Build"><span
 style="font-family: monospace;">KVDataBase::Build()</span></a>), is
saved in a ROOT file in the same directory. The default name for this
file is given by the environment variable
</p>
<pre>DataSet.DatabaseFile:   DataBase.root</pre>
<p>in <span style="font-family: monospace;">$KVROOT/KVFiles/.kvrootrc</span>.
There you will also see examples of definitions of dataset-specific
source file names and formats.
</p>
<p>To acces the database associated with the dataset, use <a
 style="font-family: monospace;" href="#KVDataSet:GetDataBase">GetDataBase()</a>.<br>
</p>

<h4>c. Data files</h4>

<p>All files of experimental data belonging to this dataset are kept
under the <span style="font-family: monospace;">data_repository_subdirectory</span>
defined in the configuration of the dataset. The name of this
directory is returned by <a style="font-family: monospace;"
 href="#KVDataSet:GetDatapathSubdir">GetDatapathSubdir()</a>. There are
four different types of data, each of which is kept in a different
subdirectory of the dataset's repository directory:
</p>
<ul>
      <li><span style="font-family: monospace;">raw/</span>&nbsp;&nbsp;&nbsp;
The raw data written during the experiment by the experimental
acquisition system
      </li>
      <li><span style="font-family: monospace;">recon/</span>&nbsp;&nbsp;&nbsp;
Reconstructed data in which the (uncalibrated) energy losses measured
in different detectors of the set-up have been associated into
(unidentified) particles.
      </li>
      <li><span style="font-family: monospace;">ident/</span>&nbsp;&nbsp;&nbsp;
Reconstructed data of the previous type to which some or all of the
necessary identification and calibration algorithms have been applied
in order to identify and calibrate the previously reconstructed
particles.
      </li>
      <li><span style="font-family: monospace;">root/</span>&nbsp;&nbsp;&nbsp;
Fully-identified and calibrated reconstructed events resulting from the
application of coherency algorithms to the last-produced <span
 style="font-family: monospace;">ident</span> data files.<br>
      </li>
</ul>
<p>It should be noted that the last 3 types of data are obligatorily
stored in <a href="http://root.cern.ch/root/html/TTree.html">TTrees</a>
written in ROOT files. They also have exactly the same structure, i.e.
they are constituted of <a href="KVReconstructedEvent.html">events</a>
containing one or more <a href="KVReconstructedNucleus.html">particles</a>.
The algorithm for reconstructing raw data, i.e. for creating <span
 style="font-family: monospace;">recon</span> files from <span
 style="font-family: monospace;">raw</span> files, is described in <a
 href="KVReconstructedEvent.html#KVReconstructedEvent:ReconstructEvent"><span
 style="font-family: monospace;">KVReconstructedEvent::ReconstructEvent</span></a>.
Both this process, and also <span style="font-family: monospace;">recon</span>&#8594;<span
 style="font-family: monospace;">ident</span>, <span
 style="font-family: monospace;">ident</span>&#8594;<span
 style="font-family: monospace;">root</span> are among the data
analysis tasks predefined (for INDRA data) in <span
 style="font-family: monospace;">$KVROOT/KVFiles/.kvrootrc</span>.
</p>

<p>This classification of data is meant to reflect the chronology of a
typical process of data reduction &amp; analysis. The first step is to
create <span style="font-family: monospace;">recon</span> files from
the raw data using the <span style="font-family: monospace;">raw</span>&#8594;<span
 style="font-family: monospace;">recon</span> analysis task. This can
be performed as soon as the description of the experimental set-up (<a
 href="KVMultiDetArray.html"><span style="font-family: monospace;">KVMultiDetArray</span></a>
class) has been implemented and the bare minimum of source files for
setting up the database (runlist) are available. The advantage of
working with recon files is that (i) the event reconstruction is
performed once, in the <span style="font-family: monospace;">raw</span>&#8594;<span
 style="font-family: monospace;">recon </span>step, so subsequent
analysis is that much faster; (ii) the data is stored in TTrees in ROOT
files which, unless the acquisition system is based on ROOT, means that
they can be analysed much more efficiently and on more platforms than
the original raw files.
</p>
<!-- */
// --> END_HTML
/////////////////////////////////////////////////
KVDataSet *gDataSet;

KVDataSet::KVDataSet()
{
   //Default constructor
   fDBase = 0;
   fUpDater = 0;
   fRepository = 0;
   fDataBase = 0;
}

KVDataSet::~KVDataSet()
{
   if (fUpDater)
      delete fUpDater;
   fUpDater = 0;
   if (fDBase) {
      delete fDBase;
      fDBase = 0;
   }
//    if (fDataBase) {
//       delete fDataBase;
//       fDataBase = 0;
//    }
   fTasks.Delete();
}

KVMultiDetArray *KVDataSet::BuildMultiDetector() const
{
   //Create & build an instance of the multidetector class needed for this dataset using static
   //method KVMultiDetArray::MakeMultiDetector(const Char_t*).
   //
   //The name of the dataset must correspond to the name of one of the Plugin.KVMultiDetArray
   //plugins defined in the $KVROOT/KVFiles/.kvrootrc configuration file

   return KVMultiDetArray::MakeMultiDetector(GetName());
}

KVAvailableRunsFile *KVDataSet::GetAvailableRunsFile(const Char_t * type)
{
   //Returns available runs file object for given data 'type' (="raw", "recon", "ident", "root")
   //Object will be created and added to internal list if it does not exist

   KVAvailableRunsFile *avrf =
       (KVAvailableRunsFile *) fAvailableRuns.FindObjectByName(type);
   if (!avrf) {
      avrf = fRepository->NewAvailableRunsFile(type, this);
      fAvailableRuns.Add(avrf);
   }
   return avrf;
}

const Char_t *KVDataSet::GetDBFileName() const
{
   //Returns name of file containing database for dataset.
   //If this is not set explicitly with SetDBFileName(), the default value defined by DataSet.DatabaseFile
   //in $KVROOT/KVFiles/.kvrootrc will be returned.

   if (fDBFileName != "")
      return fDBFileName.Data();
   return gEnv->GetValue("DataSet.DatabaseFile", "");
}

const Char_t *KVDataSet::GetDBName() const
{
   //Returns name of database object in database file.
   //If this is not set explicitly with SetDBName(), we use the name of the dataset by default

   if (fDBName != "")
      return fDBName.Data();
   return GetName();
}

//_______________________________________________________________//

void KVDataSet::OpenDBFile(const Char_t * full_path_to_dbfile)
{
   //Open the database from a file on disk.

   if (fDBase) {
      delete fDBase;
      fDBase = 0;
   }

   TDirectory *work_dir = gDirectory;   //keep pointer to current directory
   fDBase = new TFile(full_path_to_dbfile, "READ");

   if (fDBase->IsOpen()) {
      fDataBase = (KVDataBase *) fDBase->Get(GetDBName());
      if (!fDataBase) {
         Error("OpenDBFile", "%s not found in file %s", GetDBName(),
               GetDBFileName());
      }
		else
		{
			fDataBase->ReadObjects( fDBase ); // read any associated objects
		}
      work_dir->cd();           //back to initial working directory
      //delete fDBase; fDBase=0; do not close database file -
      //we need the TProcessID object in the file for references/links to work
   }
}

//_______________________________________________________________//

const Char_t* KVDataSet::GetFullPathToDB()
{
   // Returns full path to file where database is written on disk ($KVROOT/db/[dataset]/DataBase.root)

   TString dbfile = GetDBFileName();
   static TString dbfile_fullpath;
   TString tmp;
   AssignAndDelete(dbfile_fullpath, gSystem->ConcatFileName(GetKVRootDir(), "db"));
   AssignAndDelete(tmp, gSystem->ConcatFileName(dbfile_fullpath.Data(), GetName()));
   AssignAndDelete(dbfile_fullpath, gSystem->ConcatFileName(tmp.Data(), dbfile.Data()));
   return dbfile_fullpath.Data();
}

//_______________________________________________________________//

void KVDataSet::SaveDataBase()
{
   // Write the database to disk (ROOT file).
   // It will be written in the directory
   //   $KVROOT/db/[dataset name]
   // If the directory does not exist, it will be created.
   //
   // # Default name of database file containing informations on runs, systems, calibration parameters etc.
   // DataSet.DatabaseFile:        DataBase.root
   // # Default name of database object in file
   // DataSet.DatabaseName:        DataBase

   TString dbfile_fullpath = GetFullPathToDB();
   TString tmp = gSystem->DirName( dbfile_fullpath.Data() );
   // create $KVROOT/db/[dataset] subdirectory if necessary
   if( gSystem->AccessPathName( tmp.Data() ) ){
      //make directory db/[dataset]
      if( gSystem->mkdir( tmp.Data() )==-1 ){
          //is $KVROOT/db missing ?
          TString tmp2 = gSystem->DirName(tmp.Data());
          if( gSystem->mkdir(tmp2.Data())==-1){
            Error("SaveDataBase", "Cannot create directory %s required to save database",
               tmp2.Data());
            return;
          }
          //try again
          if( gSystem->mkdir( tmp.Data() )==-1 ){
              Error("SaveDataBase", "Cannot create directory %s required to save database",
              tmp.Data());
              return;
          }
      }
   }

   WriteDBFile(dbfile_fullpath.Data());
}

//_______________________________________________________________//

void KVDataSet::WriteDBFile(const Char_t * full_path_to_dbfile)
{
   //PRIVATE METHOD
   //Write the database to disk.

   TDirectory *work_dir = gDirectory;   //keep pointer to current directory
   if (fDBase) {
      delete fDBase;
      fDBase = 0;
   }
   if (!fDataBase) {
      Error("WriteDBFile", "Database has not been built");
		return;
   }
   fDBase = new TFile(full_path_to_dbfile, "recreate");
   fDBase->cd();                //set as current directory (maybe not necessary)
   fDataBase->Write(GetDBName());    //write database to file with given name
	fDataBase->WriteObjects( fDBase ); //write any associated objects
   work_dir->cd();              //back to initial working directory
}

KVDataBase *KVDataSet::GetDataBase(Option_t * opt)
{
   //Returns pointer to database associated with this dataset.
   //Opens, updates or creates database file if necessary
	//(the database is automatically rebuilt if the source files are
	//more recent than the last database file).
   //
   //If opt="update":
   // close and delete database if already open
   // regenerate database from source files
   //Use this option to force the regeneration of the database.

   TString _opt(opt);
   _opt.ToUpper();
   if (_opt == "UPDATE") {
      OpenDataBase(_opt.Data());
   } else {
      OpenDataBase();
   }
   return fDataBase;
}

void KVDataSet::OpenDataBase(Option_t * opt)
{
   //Open the database for this dataset.
	//If the database does not exist or is older than the source files
	//in $KVROOT/KVFiles/[dataset], the database is automatically rebuilt
	//(see DataBaseNeedUpdate()).
   //Use opt="UPDATE" to force rebuilding of the database.
   //
   //First, we look in $KVROOT/db/[dataset] to see if the database file exists
   //(if no database file name given, use default name for database file defined in
   //$KVROOT/KVFiles/.kvrootrc).
   //If so, we open the database contained in the file, after first loading the required plugin
   //library if needed.
   //
   //The name of the dataset must correspond to the name of one of the Plugin.KVDataBase
   //plugins defined in the $KVROOT/KVFiles/.kvrootrc configuration file

	Bool_t is_glob_db =kFALSE;
   //if option="update" or database out of date or does not exist, (re)build the database
   if ( (!strcmp(opt, "UPDATE")) || DataBaseNeedsUpdate() ) {
		//check if it is the currently active database (gDataBase),
		//in which case we must 'cd()' to it after rebuilding
		Info("OpenDataBase", "Updating database file");
		is_glob_db = (fDataBase == gDataBase);
      if (fDataBase) {
         delete fDataBase;
         fDataBase = 0;
      }
      if (fDBase) {
         delete fDBase;
         fDBase = 0;
      }
      fDataBase = KVDataBase::MakeDataBase(GetDBName());
      SaveDataBase();
		if(fDataBase && is_glob_db) fDataBase->cd();
   }
	else if( !fDataBase ){
		// if database is not in memory at this point, we need to
		// open the database file and read in the database

		//load plugin for database
		if (!LoadPlugin("KVDataBase", GetName())) {
			Error("GetDataBase", "Cannot load required plugin library");
			return;
		}
   	//look for database file in dataset subdirectory
   	TString dbfile_fullpath = GetFullPathToDB();
		//open database file
   	OpenDBFile(dbfile_fullpath.Data());
	}
}

//___________________________________________________________________________________________________________________

void KVDataSet::ls(Option_t * opt) const
{
   //Print dataset information
   cout << "Dataset name=" << GetName() << " (" << GetTitle() << ")";
   if (IsAvailable()) {
      cout << "  [ AVAILABLE: ";
      if (HasRaw())
         cout << "raw ";
      if (HasRecon())
         cout << "recon ";
      if (HasIdent())
         cout << "ident ";
      if (HasPhys())
         cout << "root ";
      cout << "]";
   } else
      cout << "  [UNAVAILABLE]";
   cout << endl;
}

void KVDataSet::Print(Option_t * opt) const
{
   //Print dataset information
   //If option string contains "tasks", print numbered list of tasks that can be performed
   //If option string contains "data", print list of available data types

   TString Sopt(opt);
   Sopt.ToUpper();
   if (Sopt.Contains("TASK")) {
      if ( !GetNtasks() ) {
         cout << "                    *** No available analysis tasks ***"
             << endl;
         return;
      } else {
         for (int i = 1; i <= GetNtasks(); i++) {
            KVDataAnalysisTask *dat = GetAnalysisTask(i);
            cout << "\t" << i << ". " << dat->GetTitle() << endl;
         }
      }
      cout << endl;
   } else if (Sopt.Contains("DATA")) {
      cout << "Available data types: " << fSubdirs.
          Strip(TString::kTrailing, ',').Data() << endl;
   } else {
      ls(opt);
   }
}

void KVDataSet::CheckAvailable()
{
   //Check if this data set is physically present and available for analysis.
   //In other words we check if the value of GetDatapathSubdir() is a subdirectory
   //of the given data repository
   //If so, we proceed to check for the existence of the following subdirectories:
   //      raw, recon, ident, root
   //If none of them exists, the dataset will be reset to 'unavailable'
   //Otherwise the corresponding flags are set.
   //
   //Note that if SetUserGroups has been called with a list of groups allowed to read this data,
   //the current user's name (gSystem->GetUserInfo()->fUser) will be used to check if the
   //dataset is available. The user name must appear in the group defined by SetUserGroups.

   SetAvailable(fRepository->CheckSubdirExists(GetDatapathSubdir()));
   if (!IsAvailable())
      return;
   //check subdirectories
   fSubdirs = "";
   if (fRepository->CheckSubdirExists(GetDatapathSubdir(), "raw"))
      fSubdirs += "raw,";
   if (fRepository->CheckSubdirExists(GetDatapathSubdir(), "recon"))
      fSubdirs += "recon,";
   if (fRepository->CheckSubdirExists(GetDatapathSubdir(), "ident"))
      fSubdirs += "ident,";
   if (fRepository->CheckSubdirExists(GetDatapathSubdir(), "root"))
      fSubdirs += "root";
   //check at least one subdir exists
   SetAvailable(HasRaw() || HasRecon() || HasIdent() || HasPhys());
   //check user name against allowed groups
   if (!CheckUserCanAccess()) {
      SetAvailable(kFALSE);
      return;
   }
}

void KVDataSet::AddSubdir(const Char_t * subdir)
{
   //Add 'subdir' to the list of available subdirectories for this dataset.
   //Does nothing if 'subdir' is already in list.
   if (!fSubdirs.Contains(subdir)) {
      fSubdirs += subdir;
      if (strcmp(subdir, "root"))
         fSubdirs += ",";
   }
}

void KVDataSet::SetAnalysisTasks(const KVList * task_list)
{
   //Add to fTasks list any data analysis task in list 'task_list" whose pre-requisite
   //data subdirectory is present for this dataset. Any dataset-specific "tweaking" of the
   //task is done here.

   fTasks.Delete();
   TIter nxt(task_list);
   KVDataAnalysisTask *dat;
   while ((dat = (KVDataAnalysisTask *) nxt())) {
      if (HasSubdir(dat->GetPrereq())){
         //make new copy of default analysis task
         KVDataAnalysisTask* new_task = new KVDataAnalysisTask( *dat );
         //check if any dataset-specific parameters need to be changed
         new_task->SetParametersForDataSet(this);
         fTasks.Add( new_task );
      }
   }
}

Int_t KVDataSet::GetNtasks() const
{
   //Returns the number of tasks associated to dataset which are compatible
   //with the available data

   return fTasks.GetSize();
}

KVDataAnalysisTask *KVDataSet::GetAnalysisTask(Int_t k) const
{
   //Return kth analysis task in list of available tasks.
   //k=[1, GetNtasks()] and corresponds to the number shown next to the title of the task when
   //KVDataSet::Print("tasks") is called
   return (KVDataAnalysisTask *) fTasks.At(k - 1);
}

TList *KVDataSet::GetListOfAvailableSystems(const Char_t *datatype, KVDBSystem * systol)
{
   //Create and fill a list of available systems for this dataset and the given datatype
   //(="raw", "recon", "ident" or "root").
   //This uses the database associated to the dataset.
   //USER MUST DELETE THE LIST AFTER USE.
   //
   //For each system in the list we set the number of available runs : this number
   //can be retrieved with KVDBSystem::GetNumberRuns()
   //
   //If systol!=0 then in fact the list contains a list of runs for the given system which are available.
   //
   //If no systems are defined for the dataset then we return a list of available runs
   //for the given datatype

   //open the available runs file for the data type
   if( !GetAvailableRunsFile(datatype) ){
      Error("GetListOfAvailableSystems(const Char_t*)",
            "No available runs file for type %s", datatype);
      return 0;
   }
   return GetAvailableRunsFile(datatype)->GetListOfAvailableSystems(systol);
}

TList *KVDataSet::GetListOfAvailableSystems(KVDataAnalysisTask* datan, KVDBSystem * systol)
{
   //Create and fill a list of available systems for this dataset and the prerequisite
   //datatype for the given data analysis task.
   //This uses the database associated to the dataset.
   //USER MUST DELETE THE LIST AFTER USE.
   //
   //For each system in the list we set the number of available runs : this number
   //can be retrieved with KVDBSystem::GetNumberRuns()
   //
   //If systol!=0 then in fact the list contains a list of runs for the given system which are available.
   //
   //If no systems are defined for the dataset then we return a list of available runs
   //for the given datatype

   return GetListOfAvailableSystems(datan->GetPrereq(), systol);
}

void KVDataSet::SetName(const char *name)
{
   //Set name of dataset. Also sets name of calibration/database directory for data set, according
   //to :
   //      $KVROOT/KVFiles/<name_of_dataset>
   TNamed::SetName(name);
   AssignAndDelete(fCalibDir,
                   gSystem->ConcatFileName(KVBase::GetKVFilesDir(), name));
}

const Char_t *KVDataSet::GetDataSetDir() const
{
   //Returns full path to directory containing database and calibration/identification parameters etc.
   //for this dataset. This directory has the following name:
   //      $KVROOT/KVFiles/<name_of_dataset>
   return fCalibDir.Data();
}

void KVDataSet::cd()
{
   //Data analysis can only be performed if the data set in question
   //is "activated" or "selected" using this method.
   //At the same time, the data repository, dataset manager and database associated with
   //this dataset also become the "active" ones (pointed to by the respective global
   //pointers, gDataRepository, gDataBase, etc. etc.)

   gDataSet = this;
   fRepository->cd();
   GetDataBase()->cd();
}

//__________________________________________________________________________________________________________________

const Char_t *KVDataSet::GetDataSetEnv(const Char_t * type, const Char_t* defval) const
{
   //Will look for gEnv->GetValue "name_of_dataset.type"
   //then simply "type" if no dataset-specific value is found.
   //If neither resource is defined, return the "defval" default value (="" by default)

   TString temp;
   temp.Form("%s.%s", GetName(), type);
   if( gEnv->Defined(temp.Data()) ) return gEnv->GetValue(temp.Data(), "");
   return gEnv->GetValue(type, defval);
}

//__________________________________________________________________________________________________________________

Double_t KVDataSet::GetDataSetEnv(const Char_t * type, Double_t defval) const
{
   //Will look for gEnv->GetValue "name_of_dataset.type"
   //then simply "type" if no dataset-specific value is found.
   //If neither resource is defined, return the "defval" default value

   TString temp;
   temp.Form("%s.%s", GetName(), type);
   if( gEnv->Defined(temp.Data()) ) return gEnv->GetValue(temp.Data(), 0.0);
   return gEnv->GetValue(type, defval);
}

//__________________________________________________________________________________________________________________

Bool_t KVDataSet::GetDataSetEnv(const Char_t * type, Bool_t defval) const
{
   //Will look for gEnv->GetValue "name_of_dataset.type"
   //then simply "type" if no dataset-specific value is found.
   //If neither resource is defined, return the "defval" default value

   TString temp;
   temp.Form("%s.%s", GetName(), type);
   if( gEnv->Defined(temp.Data()) ) return gEnv->GetValue(temp.Data(), kFALSE);
   return gEnv->GetValue(type, defval);
}

//__________________________________________________________________________________________________________________

KVUpDater *KVDataSet::GetUpDater()
{
   //Return pointer to 'KVUpDater' object required to set run parameters of multidetector
   //array associated to this dataset.
   //
   //The name of the dataset must correspond to the name of one of the Plugin.KVUpDater
   //plugins defined in the $KVROOT/KVFiles/.kvrootrc configuration file

   return (fUpDater ? fUpDater
           : (fUpDater = KVUpDater::MakeUpDater(GetName())));
}

//__________________________________________________________________________________________________________________

TObject *KVDataSet::OpenRunfile(const Char_t * type, Int_t run)
{
   //Open file containing data of given "type" ("raw", "recon", "ident", "root")
   //for given run number of this dataset.
   //Returns a pointer to the opened file; if the file is not available, we return 0.
   //The user must cast the returned pointer to the correct class, which will
   //depend on the data type and the dataset (see $KVROOT/KVFiles/.kvrootrc)

   return Open(type,run);
}

//__________________________________________________________________________________________________________________

const Char_t *KVDataSet::GetFullPathToRunfile(const Char_t * type,
                                              Int_t run)
{
   //Return full path to file containing data of given "type" ("raw", "recon", "ident", "root") for given run number
   //of this dataset. NB. only works for available run files, if their is no file in the repository for this run,
   //the returned path will be empty.
   //This path should be used with e.g. TChain::Add.

   //get name of file from available runs file
   TString file = GetRunfileName(type, run);
   if (file == "")
      return file.Data();
   return fRepository->GetFullPathToOpenFile(this, type, file.Data());
}

//__________________________________________________________________________________________________________________

const Char_t *KVDataSet::GetRunfileName(const Char_t * type, Int_t run)
{
   //Return name of file containing data of given "type" ("raw", "recon", "ident", "root")
   //for given run number of this dataset.
   //NB. only works for available run files, if there is no file in the repository for this run,
   //the returned path will be empty.

   //check data type is available
   if (!HasSubdir(type)) {
      Error("GetRunfileName",
            "No data of type \"%s\" available for dataset %s", type,
            GetName());
      return 0;
   }
   //get name of file from available runs file
   return GetAvailableRunsFile(type)->GetFileName(run);
}

//__________________________________________________________________________________________________________________

TDatime KVDataSet::GetRunfileDate(const Char_t * type, Int_t run)
{
   //Return date of file containing data of given "type" ("raw", "recon", "ident", "root")
   //for given run number of this dataset.
   //NB. only works for available run files, if there is no file in the repository for this run,
   //an error will be printed and the returned data is set to "Sun Jan  1 00:00:00 1995"
   //(earliest possible date for TDatime class).

   static TDatime date;
   date.Set(1995, 1, 1, 0, 0, 0);
   //check data type is available
   if (!HasSubdir(type)) {
      Error("GetRunfileDate",
            "No data of type \"%s\" available for dataset %s", type,
            GetName());
      return date;
   }
   //get date of file from available runs file
   TString filename;
   if (!GetAvailableRunsFile(type)->GetRunInfo(run, date, filename)) {
      Error("GetRunfileDate",
            "Runfile not found for run %d (data type: %s)", run, type);
   }
   return date;
}

//__________________________________________________________________________________________________________________

Bool_t KVDataSet::CheckRunfileAvailable(const Char_t * type, Int_t run)
{
   //We check the availability of the run by looking in the available runs file associated
   //with the given datatype.

   //check data type is available
   if (!HasSubdir(type)) {
      Error("CheckRunfileAvailable",
            "No data of type \"%s\" available for dataset %s", type,
            GetName());
      return 0;
   }
   return GetAvailableRunsFile(type)->CheckAvailable(run);
}

//___________________________________________________________________________

const Char_t *KVDataSet::GetBaseFileName(const Char_t * type, Int_t run)
{
   //PRIVATE METHOD: Returns base name of data file containing data for the run.
   //type = "raw", "recon", "ident" or "root"
   //The filename corresponds to one of the formats defined in $KVROOT/KVFiles/.kvrootrc:
   //
   //# Default formats for runfile names (run number is used to replace integer format)
   //DataSet.RunFileName.raw:    run%d.raw
   //DataSet.RunFileName.recon:    run%d.recon.root
   //DataSet.RunFileName.ident:    run%d.ident.root
   //DataSet.RunFileName.root:    run%d.root*
   //
   //The actual name of the file, if it has already been written in the data repository,
   //is contained in the available_runs.*.* file; if the file has not been written, or it is to
   //be updated, a new name will be generated from this base, adding the date and time
   //to the end.

   static TString tmp;
   //get format string
   TString fmt = GetDataSetEnv(Form("DataSet.RunFileName.%s", type));
   if (fmt != "") {
      tmp.Form(fmt, run);
   }
   return tmp.Data();
}

//___________________________________________________________________________

void KVDataSet::UpdateAvailableRuns(const Char_t * type)
{
   //Update list of available runs for given data 'type'
   //check data type is available
   if (!HasSubdir(type)) {
      Error("UpdateAvailableRuns",
            "No data of type \"%s\" available for dataset %s", type,
            GetName());
   }
   GetAvailableRunsFile(type)->Update();
}

//___________________________________________________________________________

TFile *KVDataSet::NewRunfile(const Char_t * type, Int_t run)
{
   //Create a new runfile for the dataset
   //The name of the new file will be a concatenation of GetBaseFileName(type,run)
   //and the current date and time (TDatime::AsSQLString).
   //Once the file has been filled, use CommitRunfile to submit it to the repository.

   TDatime now;
   TString tmp;
   tmp.Form("%s.%s", GetBaseFileName(type, run), now.AsSQLString());
   //turn any spaces into "_"
   tmp.ReplaceAll(" ", "_");
   return fRepository->CreateNewFile(this, type, tmp.Data());
}

//___________________________________________________________________________

void KVDataSet::DeleteRunfile(const Char_t * type, Int_t run, Bool_t confirm)
{
   //Delete the file for the given run of data type "type" from the repository.
   //By default, confirm=kTRUE, which means that the user will be asked to confirm
   //that the file should be deleted. If confirm=kFALSE, no confirmation will be asked
   //for and the file will be deleted straight away.
   //
   //WARNING: this really does DELETE files in the repository, they cannot be
   //retrieved once they have been deleted.

   //get name of file to delete
   TString filename = GetAvailableRunsFile(type)->GetFileName(run);
   if( filename == "" ){
      Error("DeleteRunfile", "Run %d of type %s does not exist.", run, type);
      return;
   }
   //delete file
   //add a safety condition for raw files
	if (!strcmp(type,"raw")) {
		Error("DeleteRunFile","raw files cannot be deleted");
		return;
	}
	fRepository->DeleteFile(GetDatapathSubdir(), type, filename.Data(), confirm);
   //was file deleted ? if so, remove entry from available runs file
   if(!fRepository->CheckFileStatus(GetDatapathSubdir(), type, filename.Data()))
      GetAvailableRunsFile(type)->Remove(run);
}

//___________________________________________________________________________

void KVDataSet::DeleteRunfiles(const Char_t * type, KVNumberList nl, Bool_t confirm)
{
   //Delete files corresponding to a list of runs of data type "type" from the repository.
   //By default, confirm=kTRUE, which means that the user will be asked to confirm
   //that each file should be deleted. If confirm=kFALSE, no confirmation will be asked
   //for and the file will be deleted straight away.
   //if "nl" is empty (default value) all runs of the dataset corresponding to the given type
	//will be deleted
   //WARNING: this really does DELETE files in the repository, they cannot be
   //retrieved once they have been deleted.

   if (nl.IsEmpty()) nl = GetRunList(type);
	if (nl.IsEmpty()) return;
	nl.Begin();
	while (!nl.End()){
		DeleteRunfile(type,nl.Next(),confirm);
	}
}

//___________________________________________________________________________

KVNumberList KVDataSet::GetRunList_DateSelection(const Char_t * type,TDatime* min,TDatime* max)
{
   //Returns list of runs after date / time selection
	//Run generated between ]min;max[ are selected
	//if min=NULL	runs with date <max are selected
	//if max=NULL runs with date >min are selected
	//if max and min are NULL returns empty KVNumberList

	if (!min && !max) return 0;

	if (min) printf("date minimum %s\n",min->AsString());
	if (max) printf("date maximum %s\n",max->AsString());

	KVNumberList numb;

	TList* ll = GetListOfAvailableSystems(type);
	KVDBSystem* sys=0;
	KVDBRun* run=0;
	TList* lrun=0;
	for (Int_t nl=0;nl<ll->GetEntries();nl+=1){
		sys = (KVDBSystem* )ll->At(nl);
		sys->ls();
		lrun = GetListOfAvailableSystems(type,sys);

		for (Int_t nr=0;nr<lrun->GetEntries();nr+=1){
			run =(KVDBRun* )lrun->At(nr);

			if (min && max){
				if (*min<run->GetDatime() && run->GetDatime()<*max){
					numb.Add(run->GetNumber());
				}
			}
			else if (min){
				if (*min<run->GetDatime()){
					numb.Add(run->GetNumber());
				}
			}
			else if (max){
				if (run->GetDatime()<*max){
					numb.Add(run->GetNumber());
				}
			}
		}
	}

	return numb;

}

//___________________________________________________________________________

void KVDataSet::CommitRunfile(const Char_t * type, Int_t run, TFile * file)
{
   //Commit a runfile previously created with NewRunfile() to the repository.
   //Any previous version of the runfile will be deleted.
   //The available runs list for this data 'type'  is updated.

   //keep name of file for updating available runs list
   TString newfile = gSystem->BaseName(file->GetName());

   fRepository->CommitFile(file);
   //update list of subdirectories of dataset,
   //in case this addition has created a new subdirectory
   CheckAvailable();
   //check if previous version of file exists
   //get name of file from available runs file
   //note that when the file is the first of a new subdirectory, GetAvailableRunsFile->GetFileName
   //will cause the available runs file to be created, and it will contain one entry:
   //the new file!
   TString oldfile = GetAvailableRunsFile(type)->GetFileName(run);
   if (oldfile != "" && oldfile != newfile) {
      //delete previous version - no confirmation
      fRepository->DeleteFile(GetDatapathSubdir(), type, oldfile.Data(),
                              kFALSE);
      //remove entry from available runs file
      GetAvailableRunsFile(type)->Remove(run);
   }
   if (oldfile != newfile) {
      //add entry for new run in available runs file
      GetAvailableRunsFile(type)->Add(run, newfile.Data());
   }
}

//___________________________________________________________________________

Bool_t KVDataSet::CheckUserCanAccess()
{
   //if fUserGroups has been set with SetUserGroups(), we check that the current user's name
   //(gSystem->GetUserInfo()->fUser) appears in at least one of the groups in the list.
   //Returns kFALSE if user's name is not found in any of the groups.
   //if fUserGroups="" (default), we return kTRUE for all users.

   if (fUserGroups == "")
      return kTRUE;             /* no groups set, all users have access */

   //split into array of group names
   TObjArray *toks = fUserGroups.Tokenize(' ');
   TObjString *group_name;
   TIter next_name(toks);
   while ((group_name = (TObjString *) next_name())) {
      //for each group_name, we check if the user's name appears in the group
      if (fRepository->GetDataSetManager()->
          CheckUser(group_name->String().Data())) {
         delete toks;
         return kTRUE;
      }
   }
   delete toks;
   return kFALSE;
}

//___________________________________________________________________________

void KVDataSet::SetRepository(KVDataRepository * dr)
{
   //Set pointer to data repository in which dataset is stored
   fRepository = dr;
}

//___________________________________________________________________________

KVDataRepository *KVDataSet::GetRepository() const
{
   //Get pointer to data repository in which dataset is stored
   return fRepository;
}

//___________________________________________________________________________

void KVDataSet::CheckMultiRunfiles(const Char_t * data_type)
{
   //Check all runs for a given datatype and make sure that only one version
   //exists for each run. If not, we print a report on the runfiles which occur
   //multiple times, with the associated date and file name.

   KVNumberList doubles =
       GetAvailableRunsFile(data_type)->CheckMultiRunfiles();
   if (doubles.IsEmpty()) {
      cout << "OK. No runs appear more than once." << endl;
   } else {
      cout << "Runs which appear more than once: " << doubles.
          AsString() << endl << endl;
      //print dates and filenames for each run

      Int_t nRuns;
      Int_t *run_numbers = doubles.GetArray(nRuns);
      KVList filenames, dates;
      for (Int_t idx = 0; idx < nRuns; idx++) {

         //get infos for current run = run_numbers[idx]
         GetAvailableRunsFile(data_type)->GetRunInfos(run_numbers[idx],
                                                      &dates, &filenames);

         cout << "Run " << run_numbers[idx] << " : " << dates.
             GetEntries() << " files >>>>>>" << endl;
         for (int i = 0; i < dates.GetEntries(); i++) {

            cout << "\t" << ((TObjString *) filenames.At(i))->String().
                Data() << "\t" << ((TObjString *) dates.At(i))->String().
                Data() << endl;

         }
      }
      delete[]run_numbers;
   }
}

//___________________________________________________________________________

void KVDataSet::CleanMultiRunfiles(const Char_t * data_type, Bool_t confirm)
{
   //Check all runs for a given datatype and make sure that only one version
   //exists for each run. If not, we print a report on the runfiles which occur
   //multiple times, with the associated date and file name, and then we
   //destroy all but the most recent version of the file in the repository, and
   //update the runlist accordingly.
   //By default, we ask for confirmation before deleting each file.
   //Call with confirm=kFALSE to delete WITHOUT CONFIRMATION (DANGER!! WARNING!!!)

   KVAvailableRunsFile *ARF = GetAvailableRunsFile(data_type);
   KVNumberList doubles = ARF->CheckMultiRunfiles();
   if (doubles.IsEmpty()) {
      cout << "OK. No runs appear more than once." << endl;
   } else {
      cout << "Runs which appear more than once: " << doubles.
          AsString() << endl << endl;
      //print dates and filenames for each run

      Int_t nRuns;
      Int_t *run_numbers = doubles.GetArray(nRuns);
      KVList filenames, dates;
      for (Int_t idx = 0; idx < nRuns; idx++) {

         //get infos for current run = run_numbers[idx]
         ARF->GetRunInfos(run_numbers[idx], &dates, &filenames);

         TDatime most_recent("1998-12-25 00:00:00");
         Int_t i_most_recent = 0;
         cout << "Run " << run_numbers[idx] << " : " << dates.
             GetEntries() << " files >>>>>>" << endl;
         for (int i = 0; i < dates.GetEntries(); i++) {

            //check if run is most recent
            TDatime rundate(((TObjString *) dates.At(i))->String().Data());
            if (rundate > most_recent) {

               most_recent = rundate;
               i_most_recent = i;

            }
         }
         //Now, we loop over the list again, this time we destroy all but the most recent
         //version of the runfile
         for (int i = 0; i < dates.GetEntries(); i++) {

            if (i == i_most_recent) {
               cout << "KEEP : ";
            } else {
               cout << "DELETE : ";
            }
            cout << "\t" << ((TObjString *) filenames.At(i))->String().
                Data() << "\t" << ((TObjString *) dates.At(i))->String().
                Data() << endl;
            if (i != i_most_recent) {
               //delete file from repository forever and ever
               fRepository->DeleteFile(GetDatapathSubdir(), data_type,
                                       ((TObjString *) filenames.At(i))->
                                       String().Data(), confirm);
               //remove file entry from available runlist
               ARF->Remove(run_numbers[idx],
                           ((TObjString *) filenames.At(i))->String().
                           Data());
            }
         }
      }
      delete[]run_numbers;
   }
}

//___________________________________________________________________________

Bool_t KVDataSet::CheckRunfileUpToDate(const Char_t * data_type, Int_t run,
                                       KVDataRepository * other_repos)
{
   //Use this method to check whether the file of type "data_type" for run number "run"
   //in the data repository "other_repos" is more recent than the file contained in the data
   //repository corresponding to this dataset.
   //Returns kFALSE if file in other repository is more recent.

   if (!other_repos)
      return kTRUE;
   //get dataset with same name as this one from dataset manager of other repository
   KVDataSet *ds = other_repos->GetDataSetManager()->GetDataSet(GetName());
   if (!ds) {
      Error("CheckRunfileUpToDate",
            "Dataset \"%s\" not found in repository \"%s\"", GetName(),
            other_repos->GetName());
      return kFALSE;
   }
   //compare dates of the two runfiles
   if (GetRunfileDate(data_type, run) < ds->GetRunfileDate(data_type, run))
      return kFALSE;
   return kTRUE;
}

//___________________________________________________________________________

Bool_t KVDataSet::CheckRunfileUpToDate(const Char_t * data_type, Int_t run,
                                       const Char_t * other_repos)
{
   //Use this method to check whether the file of type "data_type" for run number "run"
   //in the data repository "other_repos" is more recent than the file contained in the data
   //repository corresponding to this dataset.
   //Returns kTRUE if no repository with name "other_repos" exists.
   //Returns kFALSE if file in other repository is more recent.

   KVDataRepository *_or =
       gDataRepositoryManager->GetRepository(other_repos);
   if (_or)
      return CheckRunfileUpToDate(data_type, run, _or);
   Error("CheckRunfileUpToDate",
         "No data repository known with this name : %s", other_repos);
   return kTRUE;
}

//___________________________________________________________________________

void KVDataSet::CheckUpToDate(const Char_t * data_type,
                              const Char_t * other_repos)
{
   //Check whether all files of type "data_type" for run number "run" in the data repository
   //are up to date (i.e. at least as recent) as compared to the files in data repository "other_repos".

   KVDataRepository *_or =
       gDataRepositoryManager->GetRepository(other_repos);
   if (!_or) {
      Error("CheckUpToDate",
            "No data repository known with this name : %s", other_repos);
      return;
   }
   KVNumberList runlist = GetAvailableRunsFile(data_type)->GetRunList();
   Int_t nRuns;
   Int_t *run_numbers = runlist.GetArray(nRuns);
   Int_t need_update = 0;
   for (Int_t idx = 0; idx < nRuns; idx++) {
      //check run
      if (!CheckRunfileUpToDate(data_type, run_numbers[idx], _or)) {
         cout << " *** run " << run_numbers[idx] << " needs update ***" <<
             endl;
         cout << "\t\tREPOSITORY: " << fRepository->
             GetName() << "\tDATE: " << GetRunfileDate(data_type,
                                                       run_numbers[idx]).
             AsString() << endl;
         cout << "\t\tREPOSITORY: " << other_repos << "\tDATE: " << _or->
             GetDataSetManager()->GetDataSet(GetName())->
             GetRunfileDate(data_type,
                            run_numbers[idx]).AsString() << endl;
         need_update++;
      }
   }
   if (!need_update) {
      cout << " *** All runfiles are up to date for data type " <<
          data_type << endl;
   }
   delete[]run_numbers;
}

//___________________________________________________________________________

KVNumberList KVDataSet::GetUpdatableRuns(const Char_t * data_type,
                                         const Char_t * other_repos)
{
   //Returns list of all runs of type "data_type" which may be updated
   //from the repository named "other_repos". See CheckUpToDate().

   KVNumberList updates;
   KVDataRepository *_or =
       gDataRepositoryManager->GetRepository(other_repos);
   if (!_or) {
      Error("CheckUpToDate",
            "No data repository known with this name : %s", other_repos);
      return updates;
   }
   KVNumberList runlist = GetAvailableRunsFile(data_type)->GetRunList();
   Int_t nRuns;
   Int_t *run_numbers = runlist.GetArray(nRuns);
   for (Int_t idx = 0; idx < nRuns; idx++) {
      //check run
      if (!CheckRunfileUpToDate(data_type, run_numbers[idx], _or)) {
         //run is out of date
         updates.Add(run_numbers[idx]);
      }
   }
   delete[]run_numbers;
   return updates;
}

//___________________________________________________________________________

KVNumberList KVDataSet::GetRunList(const Char_t * data_type,
                                   const KVDBSystem * system)
{
   //Returns list of all runs available for given "data_type"
   //If a pointer to a reaction system is given, only runs for the
   //given system will be included in the list.
   KVNumberList list;
   if (!HasSubdir(data_type)) {
      Error("GetRunList",
            "No data of type %s available. Runlist will be empty.",
            data_type);
   } else {
      list = GetAvailableRunsFile(data_type)->GetRunList(system);
   }
   return list;
}

//___________________________________________________________________________

KVDataAnalysisTask *KVDataSet::GetAnalysisTask(const Char_t* keywords) const
{
   //This method returns a pointer to the available analysis task whose description (title) contains
   //all of the whitespace-separated keywords (which may be regular expressions)
   //given in the string "keywords". The comparison is case-insensitive.
   //
   //WARNING: this method can only be used to access analysis tasks that are
   //available for this dataset, i.e. for which the corresponding prerequisite data type
   //is available in the repository.
   //For unavailable data/tasks, use GetAnalysisTaskAny(const Char_t*).
   //
   //EXAMPLES
   //Let us suppose that the current dataset has the following list of tasks:
   //
// root [2] gDataSet->Print("tasks")
//         1. Event reconstruction from raw data (raw->recon)
//         2. Analysis of raw data
//         3. Identification of reconstructed events (recon->ident)
//         4. Analysis of reconstructed events (recon)
//         5. Analysis of partially identified & calibrated reconstructed events (ident)
//         6. Analysis of fully calibrated physical data (root)
   //Then the following will occur:
   //
// root [14] gDataSet->GetAnalysisTask("raw->recon")->Print()
// KVDataAnalysisTask : Event reconstruction from raw data (raw->recon)
//
// root [10] gDataSet->GetAnalysisTask("analysis root")->Print()
// KVDataAnalysisTask : Analysis of fully calibrated physical data (root)
//

   //case-insensitive search for matches in list based on 'title' attribute
   return (KVDataAnalysisTask*)fTasks.FindObjectAny("title",keywords,kTRUE,kFALSE);
}

//___________________________________________________________________________

TObject* KVDataSet::Open(const Char_t* type, Int_t run, Option_t *opt)
{
   //Open a file using plugin defined in $KVROOT/KVFiles/.kvrootrc
   //The default base classes for each type are defined as in this example:
   //
   // # Default base classes for reading runfiles of different types
   // DataSet.RunFileClass.raw:    KVRawDataReader
   // DataSet.RunFileClass.recon:    TFile
   // DataSet.RunFileClass.ident:    TFile
   // DataSet.RunFileClass.root:    TFile
   //
   //A different base class can be defined for a specific dataset/datatype
   //by adding a line to your $HOME/.kvrootrc like this:
   //
   //name_of_dataset.DataSet.RunFileClass.data_type:     BaseClassName
   //
   //The actual class to be used is then defined by plugins in $KVROOT/KVFiles/.kvrootrc,
   //for example
   //
   //Plugin.KVRawDataReader:    raw.INDRA*    KVINDRARawDataReader     KVIndra    "KVINDRARawDataReader()"
   //
   //which defines the plugin for raw data for all datasets whose name begins with "INDRA"
   //If no plugin is found for the base class defined by DataSet.RunFileClass, the base class is used.
   //
   //To actually open the file, each base class & plugin must define a method
   //    static BaseClass* Open(const Char_t* path, Option_t* opt="", ...)
   //which takes the full path to the file as argument (any other arguments taking default options)
   //and returns a pointer of the BaseClass type to the created object which can be used to read the file.

   TString fname = GetFullPathToRunfile(type, run);
   if (fname == "") return 0; //file not found

	// check connection to repository (e.g. SSH tunnel) in case of remote repository
   if(!fRepository->IsConnected()) return 0;

   //get base class for dataset & type
   KVString base_class = GetDataSetEnv( Form("DataSet.RunFileClass.%s", type) );

   //look for plugin specific to dataset & type
   TPluginHandler *ph = LoadPlugin( base_class.Data(), Form("%s.%s", type, GetName()));

   TClass* cl;
   if(!ph){
      //no plugin - use base class
      cl = gROOT->GetClass( base_class.Data() );
   }
   else
   {
      cl = gROOT->GetClass( ph->GetClass() );
   }

   //set up call to static Open method
   TMethodCall* methcall;
   if(strcmp(opt,"")){
      //Open with option
      methcall = new  TMethodCall(cl, "Open", Form("\"%s\", \"%s\"", fname.Data(), opt));
   }
   else
   {
      //Open without option
      methcall = new  TMethodCall(cl, "Open", Form("\"%s\"", fname.Data()));
   }

   if(!methcall->IsValid()){
      if(ph) Error("Open", "Open method for class %s is not valid", ph->GetClass());
      else Error("Open", "Open method for class %s is not valid", base_class.Data());
      delete methcall;
      return 0;
   }

   //open the file
   Long_t retval;
   methcall->Execute(retval);
   delete methcall;
   return ((TObject*)retval);
}

//___________________________________________________________________________

void KVDataSet::MakeAnalysisClass(const Char_t* task, const Char_t* classname)
{
   //Create a skeleton analysis class to be used for analysis of the data belonging to this dataset.
   //
   //  task = keywords contained in title of analysis task (see GetAnalysisTask(const Char_t*))
   //              (you do not need to include 'analysis', it is added automatically)
   //  classname = name of new analysis class
   //
   //Example:
   //  MakeAnalysisClass("raw", "MyRawDataAnalysis")
   //       -->  make skeleton raw data analysis class in files MyRawDataAnalysis.cpp & MyRawDataAnalysis.h
   //  MakeAnalysisClass("fully calibrated", "MyDataAnalysis")
   //       -->  make skeleton data analysis class in files MyDataAnalysis.cpp & MyDataAnalysis.h

   KVString _task = task; _task += " analysis";
   //We want to be able to write analysis classes even when we don't have any data
   //to analyse. Therefore we use GetAnalysisTaskAny.
   KVDataAnalysisTask* dat = GetAnalysisTaskAny( _task.Data() );
   if( !dat ) {
      Error("MakeAnalysisClass",
            "called for unknown or unavailable analysis task : %s", _task.Data());
      return;
   }
   if( !dat->WithUserClass() ) {
      Error("MakeAnalysisClass",
            "no user analysis class for analysis task : %s", dat->GetTitle());
      delete dat;
      return;
   }

   //all analysis base classes must define a static Make(const Char_t * classname)
   //which generates the skeleton class files.

   TClass* cl = 0x0;
   //has the user base class for the task been compiled and loaded ?
   if(dat->CheckUserBaseClassIsLoaded()) cl = gROOT->GetClass( dat->GetUserBaseClass() );
   else{
       delete dat; return;
   }

   delete dat;  // we have finished with dat: we must delete it

   //set up call to static Make method
   TMethodCall* methcall;
   methcall = new  TMethodCall(cl, "Make", Form("\"%s\"", classname));

   if(!methcall->IsValid()){
      Error("MakeAnalysisClass", "static Make(const Char_t*) method for class %s is not valid",
            cl->GetName());
      delete methcall;
      return;
   }

   //generate skeleton class
   methcall->Execute();
   delete methcall;
}

//___________________________________________________________________________

Bool_t KVDataSet::OpenDataSetFile(const Char_t* filename, ifstream &file)
{
   //Look for (and open for reading, if found) the named file in the directory which
   //contains the files for this dataset, i.e. $KVROOT/KVFiles/name_of_dataset
   return SearchAndOpenKVFile(filename, file, GetName());
}

//___________________________________________________________________________

KVDataAnalysisTask *KVDataSet::GetAnalysisTaskAny(const Char_t* keywords) const
{
   //This method returns a pointer to the analysis task whose description (title) contains
   //all of the whitespace-separated keywords (which may be regular expressions)
   //given in the string "keywords". The comparison is case-insensitive.
   //The analysis task does not need to be "available", i.e. the associated prerequisite
   //data type does not have to be present in the repository (see GetAnalysisTask).
   //
   //WARNING! WARNING! WARNING!
   //  *** the user must delete the KVDataAnalysisTask object returned by this method after use ***
   //

   //case-insensitive search for matches in list of all analysis tasks, based on 'title' attribute
   KVDataAnalysisTask* tsk =
      (KVDataAnalysisTask*) gDataSetManager->GetAnalysisTaskList()->FindObjectAny("title",keywords,kTRUE,kFALSE);
   if(!tsk){
      Error("GetAnalysisTaskAny", "No task found with the following keywords in its title : %s",
            keywords);
      return 0;
   }
   //make new copy of default analysis task
   KVDataAnalysisTask* new_task = new KVDataAnalysisTask( *tsk );
   //check if any dataset-specific parameters need to be changed
   new_task->SetParametersForDataSet( const_cast<KVDataSet*>(this) );
   return new_task; //must be deleted by user
}

//___________________________________________________________________________

Bool_t KVDataSet::DataBaseNeedsUpdate()
{
	// Returns kTRUE if database needs to be regenerated from source files,
	// i.e. if source files in $KVROOT/KVFiles/"name_of_dataset"
	// are more recent than DataBase.root

	TString pwd = gSystem->pwd();
	gSystem->cd( GetDataSetDir() );
	TString cmd = "make -q";
	Int_t ret = gSystem->Exec(cmd.Data());
	gSystem->cd( pwd.Data() );
	return (ret!=0);
}