//Created by KVClassFactory on Tue Jun 29 14:44:22 2010
//Author: bonnet

#ifndef __KVNAMEVALUELIST_H
#define __KVNAMEVALUELIST_H

#include "KVHashList.h"
#include "TNamed.h"

class KVNamedParameter;

class KVNameValueList : public TNamed
{

	KVHashList fList;//list of KVNamedParameter objects
	
	public:
   
	KVNameValueList();
	KVNameValueList(const Char_t* name, const Char_t* title="");
	KVNameValueList(const KVNameValueList&);
   virtual ~KVNameValueList();
	
	KVHashList* GetList() const;

	virtual void Clear(Option_t* opt = "");
	virtual void Print(Option_t* opt = "") const;
	virtual void ls(Option_t* opt = "") const { Print(opt); };
   
	void SetOwner(Bool_t enable = kTRUE);
	Bool_t IsOwner() const;
	
	void Copy(TObject& nvl) const;
	Int_t Compare(const TObject* nvl) const;
	
	void SetValue(const Char_t* name,const Char_t* value);
	void SetValue(const Char_t* name,Int_t value);
	void SetValue(const Char_t* name,Double_t value);
	
	KVNamedParameter* FindParameter(const Char_t* name) const;
	KVNamedParameter* GetParameter(Int_t idx) const;
	void RemoveParameter(const Char_t* name);
	Bool_t HasParameter(const Char_t* name);
	Int_t GetNameIndex(const Char_t* name);
	const Char_t* GetNameAt(Int_t idx) const;
	Int_t GetNpar() const;
	Int_t GetEntries() const { return GetNpar(); };
	
	Int_t GetIntValue(const Char_t* name) const;
	Double_t GetDoubleValue(const Char_t* name) const;
	const Char_t* GetStringValue(const Char_t* name) const;
	
	Int_t GetIntValue(Int_t idx) const;
	Double_t GetDoubleValue(Int_t idx) const;
	const Char_t* GetStringValue(Int_t idx) const;
	
	ClassDef(KVNameValueList,3)//A general-purpose list of parameters
};

#endif
