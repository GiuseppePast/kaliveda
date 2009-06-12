/***************************************************************************
$Id: KVList.cpp,v 1.31 2009/03/11 14:46:46 franklan Exp $
                          kvlist.cpp  -  description
                             -------------------
    begin                : Sat May 18 2002
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

#include "TROOT.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "TRegexp.h"
#include "TClass.h"
#include "TMethodCall.h"
#include "TDirectory.h"
#include "TKey.h"

#include "KVList.h"
#include "KVBase.h"
#include "KVString.h"

#include "Riostream.h"

ClassImp(KVList)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVList</h2>
<h4>Modified TList class</h4>
<h3>Object ownership</h3>
The KVList owns it objects by default: any objects placed in a KVList
will be deleted when the KVList is deleted, goes out of scope, or if
methods Clear() or Delete() are called. To avoid this, either use the
argument to the constructor:<br>
<pre>KVList list_not_owner(kFALSE);//list does not own its objects<br></pre>
or use the SetOwner() method to change the ownership.<br>
<h3>Signals &amp; slots: monitoring list modification</h3>
If SendModifiedSignals(kTRUE)�is called, then every time an object is added to or removed from the list, it will emit
the signal "Modified()". This can be used to monitor any changes to the
state of the list.<br>
<h3>FindObjectBy...</h3>
In addition to the standard TList::FindObject(const Char_t* name) and
TList::FindObject(TObject*) methods, KVList adds methods to find
objects based on several different properties, such as type, label,
number, or class. Note that type, label and number are only defined for
objects derived from KVBase; if the list contains a mixture of TObject-
and KVBase-derived objects, the TObject-derived objects will be ignored
if type, label or number are used to search.<br>
Finally, the very general FindObjectWithMethod method can search for an
object using any valid method.<br>
<h3>Sublists</h3>
The GetSubListWith... methods allow to generate new lists containing a
subset of the objects in the main list, according to their name, label,
type, etc. These sublists do not own their objects (they are supposed
to be owned by the main list), and must be deleted by the user in order
to avoid memory leaks.<br>
<h3>Execute</h3>
The Execute methods can be used to execute a given method with the same
arguments for every object in the list.<br>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////


//_______________________________________________________________________________
KVList::KVList(Bool_t owner)
{
   //Default ctor. If owner=kTRUE (default), makes TList own its objects:
   //they will be deleted if Clear() or Delete() are used, or if the list is
   //deleted itself.
   SetOwner(owner);
	ResetBit(kSignals);
}

//_______________________________________________________________________________

KVList::~KVList()
{
}

//_______________________________________________________________________________
KVList::KVList(const KVList & obj)
{
   //copy ctor
   SetOwner(kTRUE);
	ResetBit(kSignals);
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   obj.Copy(*this);
#else
   ((KVList &) obj).Copy(*this);
#endif
}



//_______________________________________________________________________________
KVList *KVList::GetSubListWithClass(const TClass* _class)
{
	// Create and fill a (sub)list with objects in this list of the given class.
	//
	//  *** WARNING *** : DELETE the KVList returned by this method after using it !!!
	
	KVList *sublist = new KVList(kFALSE); 
	if (_class) {
   	TObjLink *lnk = FirstLink();
   	while (lnk) {
      	if ( _class == lnk->GetObject()->IsA() ) sublist->Add(lnk->GetObject());
      	lnk = lnk->Next();
		}
   }
	
	return sublist;
}

//_______________________________________________________________________________
KVList *KVList::GetSubListWithClass(const Char_t* class_name)
{
	// Create and fill a (sub)list with objects in this list of the given class.
	//
	//  *** WARNING *** : DELETE the KVList returned by this method after using it !!!
	
	if (class_name) {
      return GetSubListWithClass(TClass::GetClass(class_name));
   }
	else return NULL;
}

//_______________________________________________________________________________
KVList *KVList::GetSubListWithMethod(const Char_t* retvalue,const Char_t* method)
{
	// Create and fill a (sub)list with objects in this list for which the
	// given method returns the given return value:
	//   e.g. if method = "GetName" and retvalue = "john", we return the
	//    (sub)list of objects in this list for which GetName() returns "john".
	//
	//  *** WARNING *** : DELETE the KVList returned by this method after using it !!!
	//
   // For each object of the list, the existence of the given method is checked using TMethodCall::IsValid()
	// if the method is valid and the return value is equal to the input one (retvalue) object is added to the subKVList
	// return type supported are those defined in TMethodCall::ReturnType()
	 
	KVList *sublist = new KVList(kFALSE); 		//owner=kFALSE (see KVList::KVList(Bool_t owner))
	if (retvalue && method) {
		KVString RV(retvalue); KVString MTH(method);
		Bool_t wildcard = RV.Contains("*");
      TObjLink *lnk = FirstLink();
      while (lnk) {
		//cout << "lnk->GetObject()->IsA() = " << lnk->GetObject()->IsA()->GetName() << endl;
	      TMethodCall mt;
         mt.InitWithPrototype(lnk->GetObject()->IsA(), MTH.Data(),"");
			if (mt.IsValid()) {
			//cout << "it is valid" << endl;
				if (mt.ReturnType()==TMethodCall::kString){
					Char_t *ret; 
					mt.Execute(lnk->GetObject(),"",&ret);
					if (!wildcard) {if (RV==ret) sublist->Add(lnk->GetObject());}
					else { if (KVString(ret).Match(RV)) sublist->Add(lnk->GetObject()); }
				}
				else if (mt.ReturnType()==TMethodCall::kLong){
					Long_t ret;
					mt.Execute(lnk->GetObject(),"",ret);
					if (ret==RV.Atoi()) sublist->Add(lnk->GetObject());
				}
				else if (mt.ReturnType()==TMethodCall::kDouble){
					Double_t ret;
					mt.Execute(lnk->GetObject(),"",ret);
					if (ret==RV.Atof()) sublist->Add(lnk->GetObject());
				}
				else cout << "this type is not supported " << mt.ReturnType() << endl;
			}
			lnk = lnk->Next();
	   }
   }	
	return sublist;
}

//_______________________________________________________________________________
KVList *KVList::GetSubListWithName(const Char_t* retvalue)
{
	// Create and fill a (sub)list with all objects in this list whose name
	// (i.e. string returned by GetName()) is "retvalue"
	//
	//  *** WARNING *** : DELETE the KVList returned by this method after using it !!!
	
	return GetSubListWithMethod(retvalue,"GetName");
}

//_______________________________________________________________________________
KVList *KVList::GetSubListWithLabel(const Char_t* retvalue)
{
	// Create and fill a (sub)list with all objects in this list whose label
	// (i.e. string returned by GetLabel()) is "retvalue"
	//
	//  *** WARNING *** : DELETE the KVList returned by this method after using it !!!
	
	return GetSubListWithMethod(retvalue,"GetLabel");
}

//_______________________________________________________________________________
KVList *KVList::GetSubListWithType(const Char_t* retvalue)
{
	// Create and fill a (sub)list with all objects in this list whose type
	// (i.e. string returned by GetType()) is "retvalue"
	//
	//  *** WARNING *** : DELETE the KVList returned by this method after using it !!!
	
	return GetSubListWithMethod(retvalue,"GetType");
}

//_______________________________________________________________________________
KVList* KVList::MakeListFromFile(TFile *file)
{
	//Static method create a list containing all objects contain of a file
	//The file can be closed after this method, objects stored in the
	//list still remains valid
	//if file=NULL, the current directory is considered
	//
	//  *** WARNING *** : DELETE the KVList returned by this method after using it !!!
	
	KVList *ll = new KVList(kFALSE);

	TKey* key=0;
	if (!file){
		TIter next_ps(gDirectory->GetListOfKeys());
		while ( (key = (TKey *) next_ps()) ) ll->Add(key->ReadObj());
	}
	else {
		/*
		if (file->GetList()->GetEntries()<file->GetListOfKeys()->GetEntries())
			file->ReadAll();
		TObjLink *lnk = file->GetList()->FirstLink();
		while (lnk) {
			ll->Add(lnk->GetObject());
			lnk=lnk->Next();
		}
		*/
		TIter next_ps(file->GetListOfKeys());
		while ( (key = (TKey *) next_ps()) ) ll->Add(key->ReadObj());
		
	}
	return ll;
}

//_______________________________________________________________________________
KVList* KVList::MakeListFromFileWithMethod(TFile *file,const Char_t* retvalue,const Char_t* method)
{
	//Static method create a list containing all objects whose "method" returns "retvalue" in a file
	//WARNING list has to be empty with KVList::Clear() method before closing file
	//if file=NULL, the current directory is considered
	//
	//  *** WARNING *** : DELETE the KVList returned by this method after using it !!!
	
	KVList* l1 = MakeListFromFile(file);
	KVList* l2 = l1->GetSubListWithMethod(retvalue,method);
	l1->Clear(); delete l1;
	return l2;

}

//_______________________________________________________________________________
KVList *KVList::MakeListFromFileWithClass(TFile *file,const TClass* _class)
{
	//Static method create a list containing all objects of given class in a file
	//WARNING list has to be empty with KVList::Clear() method before closing file
	//if file=NULL, the current directory is considered
	//
	//  *** WARNING *** : DELETE the KVList returned by this method after using it !!!
	
	KVList* l1 = MakeListFromFile(file);
	KVList* l2 = l1->GetSubListWithClass(_class);
	l1->Clear(); delete l1;
	return l2;
}

//_______________________________________________________________________________
KVList *KVList::MakeListFromFileWithClass(TFile *file,const Char_t* class_name)
{
	//Static method create a list containing all objects of given class in a file
	//WARNING list has to be empty with KVList::Clear() method before closing file
	//if file=NULL, the current directory is considered
	//
	//  *** WARNING *** : DELETE the KVList returned by this method after using it !!!
	
	KVList* l1 = MakeListFromFile(file);
   KVList* l2 = l1->GetSubListWithClass(class_name);
   l1->Clear(); delete l1;
	return l2;
}

//_______________________________________________________________________________
TObject *KVList::FindObjectWithMethod(const Char_t* retvalue,const Char_t* method)
{
	// Find the first object in the list for which the given method returns the given return value:
	//   e.g. if method = "GetName" and retvalue = "john", we return the
	//   first object in this list for which GetName() returns "john".
	//
   // For each object of the list, the existence of the given method is checked using TMethodCall::IsValid()
	// if the method is valid and the return value is equal to the input one (retvalue) object is returned
	// return type supported are those defined in TMethodCall::ReturnType()
	
	if (retvalue && method) {
		KVString RV(retvalue); KVString MTH(method);
		Bool_t wildcard = RV.Contains("*");
      TObjLink *lnk = FirstLink();
      while (lnk) {
			TMethodCall mt;
         mt.InitWithPrototype(lnk->GetObject()->IsA(), MTH.Data(),"");
			if (mt.IsValid()) {
				if (mt.ReturnType()==TMethodCall::kString){
					Char_t *ret; 
					mt.Execute(lnk->GetObject(),"",&ret);
					if (!wildcard) {if (RV==ret) { return lnk->GetObject(); } }
					else { if (KVString(ret).Match(RV)) { return lnk->GetObject(); } }
				}
				else if (mt.ReturnType()==TMethodCall::kLong){
					Long_t ret;
					mt.Execute(lnk->GetObject(),"",ret);
					if (ret==RV.Atoi()) { return lnk->GetObject(); }
				}
				else if (mt.ReturnType()==TMethodCall::kDouble){
					Double_t ret;
					mt.Execute(lnk->GetObject(),"",ret);
					if (ret==RV.Atof()) { return lnk->GetObject(); }
				}
				else cout << "this type is not supported " << mt.ReturnType() << endl;
			}
			lnk = lnk->Next();
	   }
   }	
	return 0;
	
}

//_____________________________________________________________________________
TObject *KVList::FindObjectByType(const Char_t * type)
{
   // Find an object in this list using its type (i.e. return value of GetType()).
   // Requires a sequential scan till the object has been found.
   // Returns 0 if object with specified type is not found.
   
	return FindObjectWithMethod(type,"GetType");
}

//_____________________________________________________________________________
TObject *KVList::FindObjectByClass(const Char_t * class_name)
{
   // Find an object in this list using its class name (TObject::ClassName()).
   // The first object in the list with the correct class is returned.
   // Requires a sequential scan till the object has been found.
   // Returns 0 if object with specified class is not found.
   
   if (class_name) {
      TClass* _class = TClass::GetClass(class_name);
      return FindObjectByClass(_class);
   }
   return 0;
}

//_____________________________________________________________________________
TObject *KVList::FindObjectByClass(const TClass* _class)
{
   // Find an object in this list using its class (TObject::IsA()).
   // The first object in the list with the correct class is returned.
   // Requires a sequential scan till the object has been found.
   // Returns 0 if object with specified class is not found.
   
   if (_class) {
      TObjLink *lnk = FirstLink();
      while (lnk) {
         TObject *obj = (TObject *) lnk->GetObject();
         if ( _class == obj->IsA() ){
            return obj;
         }
         lnk = lnk->Next();
      }
   }
   return 0;
}

//_______________________________________________________________________________
TObject *KVList::FindObjectByLabel(const Char_t * label)
{
   // Find an object in this list using its label (return value of GetLabel).
   // Requires a sequential scan till the object has been found.
   // Returns 0 if object with specified label is not found.

	return FindObjectWithMethod(label,"GetLabel");   
}

//_______________________________________________________________________________
TObject *KVList::FindObjectByNumber(Int_t num)
{
   //Find an object in this list using its number (value returned by GetNumber()).

	return FindObjectWithMethod(Form("%d",num), "GetNumber");   
}

//_______________________________________________________________________________
TObject *KVList::FindObjectWithNameAndType(const Char_t * name, const Char_t * type)
{
   // Find an object in this list using its type and name. Requires a sequential
	// scan till the object has been found. Returns 0 if object with specified
	// name & type is not found.

	KVList* l1 = GetSubListWithType(type);
   TObject* ob = l1->FindObject(name);
   delete l1;
	return ob;
}

void KVList::Print(Option_t * opt) const
{
   //Print description of all objects in list

   TIter next(this);
   TObject *obj;
   while ((obj = next())) {
#ifdef KV_DEBUG
      if (!obj)
         Error("Print", "Object does not exist: %l", (Long_t) obj);
#endif
      obj->Print(opt);
   }
}

#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
void KVList::Copy(TObject & obj) const
#else
void KVList::Copy(TObject & obj)
#endif
{
   //Copy this to obj.
   //If the KVList owns its objects, new clones of all objects in list are made
	//The copy list will also own its objects, and so will delete the clones

   TList::Copy(obj);            //in fact this calls TObject::Copy, no Copy method defined for collection classes
   KVList & copy = (KVList &) obj;
   //delete any pre-existing objects in copy list
   copy.Delete();
	copy.SetOwner(IsOwner());
   //clone list members
   TObject *b;
   TIter next(this);
   while ((b = next())) {
		if(IsOwner())
      	copy.Add(b->Clone());
		else
			copy.Add(b);
   }
}

//_______________________________________________________________________________

void KVList::Execute(const char *method, const char *params, Int_t * error)
{
   //Redefinition of TObject::Execute(const char *, const char *, Int_t *) method.
   //TObject::Execute is called for each object in the list in order, meaning that for each
   //object the method "method" is executed with arguments "params".
	
   TIter next(this);
   TObject *obj;
   while ((obj = next())) {
      obj->Execute(method, params, error);
   }
}

//_______________________________________________________________________________

void KVList::Execute(TMethod * method, TObjArray * params, Int_t * error)
{
   //Redefinition of TObject::Execute(TMethod *, TObjArray *, Int_t *) method.
   //TObject::Execute is called for each object in the list in order, meaning that for each
   //object the method "method" is executed with arguments "params".
	
   TIter next(this);
   TObject *obj;
   while ((obj = next())) {
      obj->Execute(method, params, error);
   }
}

//_______________________________________________________________________________

TObject* KVList::FindObjectAny(const Char_t *att, const Char_t *keys, Bool_t contains_all, Bool_t case_sensitive) const
{
   //Find an object in the list, if one of its characteristics 'att' contains any or all of
   //the keywords contained in the string 'keys'
   //
   // att = "name", "title", "class", "type", "label",
   //      WARNING: when using "type" or "label", any objects in list which do
   //                         not inherit from KVBase are ignored
   // keys = list of keywords, separated by spaces
   //        
   // contains_all = kFALSE (default) : object found if any keyword occurs in the characteristic 'att'
   // contains_all = kTRUE               : object found if all keywords occur in the characteristic 'att'
   //
   // case_sensitive = kTRUE (default) : case-sensitive comparison
   // case_sensitive = kFALSE               : ignore case of keywords

   int char_test=-1;
   enum { kName, kTitle, kClass, kType, kLabel };
   if( !strcmp(att,"name") ) char_test = kName;
   else if(!strcmp(att,"title")) char_test = kTitle;
   else if(!strcmp(att,"type")) char_test = kType;
   else if(!strcmp(att,"label")) char_test =kLabel;
   else if(!strcmp(att,"class")) char_test = kClass;
   else return 0;
   
   TString::ECaseCompare casecmp;
   if(case_sensitive) casecmp = TString::kExact;
   else casecmp = TString::kIgnoreCase;
   
   TString _keys(keys);
   TObjArray* keywords = _keys.Tokenize(' ');
   if(!keywords) return 0;
   int nkeys;
   if(!(nkeys = keywords->GetEntries())) {
      delete keywords; return 0;
   }
   
   int nmatches;
   TObjLink *lnk = FirstLink();
   TString _att;
   TObject *obj=0;
   KVBase *kvobj=0; 
   while (lnk) {
      
      obj = lnk->GetObject();
      if(char_test>kClass){
         kvobj = dynamic_cast<KVBase*>(obj);
         if(!kvobj) {
            lnk = lnk->Next();
            continue;
         }
      }
      switch(char_test){
         case kName:
            _att = obj->GetName();
            break;
         case kTitle:
            _att = obj->GetTitle();
            break;
         case kClass:
            _att = obj->ClassName();
            break;
         case kLabel:
            _att = kvobj->GetLabel();
            break;
         case kType:
            _att = kvobj->GetType();
            break;
      }
      nmatches=0;
      for(int i=0;i<nkeys;i++){         
         nmatches += ( _att.Contains( ((TObjString*)keywords->At(i))->String() , casecmp));
      }
      if( (nmatches && !contains_all) || ((nmatches==nkeys) && contains_all) ){
         delete keywords;
         return obj;
      }
      lnk = lnk->Next();
   }
   delete keywords;
   return 0;
}