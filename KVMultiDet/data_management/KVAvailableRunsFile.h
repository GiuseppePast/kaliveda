/*
$Id: KVAvailableRunsFile.h,v 1.5 2008/02/07 09:25:39 franklan Exp $
$Revision: 1.5 $
$Date: 2008/02/07 09:25:39 $
*/

//Created by KVClassFactory on Fri May  5 10:46:40 2006
//Author: franklan

#ifndef __KVAVAILABLERUNSFILE_H
#define __KVAVAILABLERUNSFILE_H

#include <KVBase.h>
#include "Riostream.h"

#include "TDatime.h"
#include "TString.h"
#include "KVNumberList.h"
#include "KVLockfile.h"
#include "KVDatime.h"

class KVDBSystem;
class KVDataSet;
class TList;
class KVList;

class KVAvailableRunsFile:public KVBase {

 protected:
   ifstream fRunlist;     //for reading runlist file
   KVLockfile runlist_lock;   //for locking runlist file

   KVDataSet *fDataSet;         //dataset to which this file belongs

   virtual const Char_t *GetFileName();
   virtual Bool_t OpenAvailableRunsFile();
   virtual void CloseAvailableRunsFile();

   Bool_t IsFileOpen() {
      return fRunlist.is_open();
   };

 public:

   KVAvailableRunsFile();
   KVAvailableRunsFile(const Char_t * type);
   KVAvailableRunsFile(const Char_t * type, KVDataSet * parent);
   virtual ~ KVAvailableRunsFile();

   virtual const Char_t *GetDataType() {
      return GetName();
   }
   virtual Int_t IsRunFileName(const Char_t * filename);
	virtual Bool_t ExtractDateFromFileName(const Char_t* name, KVDatime& date);
   virtual void Update();
   virtual Bool_t CheckAvailable(Int_t run);
   virtual Int_t Count(Int_t run);
   virtual Bool_t GetRunInfo(Int_t run, TDatime & modtime,
                             TString & filename);
   virtual void GetRunInfos(Int_t run, KVList * dates, KVList * names);
   virtual const Char_t *GetFileName(Int_t run);
   virtual TList *GetListOfAvailableSystems(const KVDBSystem * systol = 0);
   virtual KVNumberList GetRunList(const KVDBSystem * system = 0);

   virtual void Remove(Int_t run, const Char_t * filename = "");
   virtual void Add(Int_t run, const Char_t * filename);
   virtual KVDataSet *GetDataSet() const;
   virtual void SetDataSet(KVDataSet*);

   KVNumberList CheckMultiRunfiles();

   ClassDef(KVAvailableRunsFile, 1)     //Handles text files containing list of available runs for different datasets and types of data
};

#endif