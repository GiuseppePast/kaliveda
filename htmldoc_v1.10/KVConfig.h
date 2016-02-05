#ifndef KV_CONFIG__H
#define KV_CONFIG__H

// Configuration flags for current build set by options or
// depending on available external software
#define WITH_GIT_INFOS
/* #undef WITH_BZR_INFOS */
#define WITH_GNU_INSTALL
/* #undef WITH_RANGE_YANEZ */
/* #undef WITH_MODIFIED_RANGE_YANEZ */
#define WITH_GEMINI
#define WITH_OPENGL
/* #undef WITH_GRULIB */
#define WITH_FITLTG
#define WITH_BUILTIN_GRU
/* #undef CCIN2P3_XRD */
#define WITH_MULTICORE_CPU 4
#define WITH_BOOST

// Info on C++ standard used to compile code
#if __cplusplus > 199711L
#define WITH_CPP11
#if __cplusplus > 201103L
#define WITH_CPP14
#if __cplusplus > 201402L
#define WITH_CPP17
#endif
#endif
#endif

//-----------------------------------ROOT-version dependent compiler flags
//-----------------------------------to handle backwards compatibility
//
// if your code uses these functionalities, beware !!
// backwards compatibility is not assured !
// some solutions are given...


//---------------------------TGButton::SetEnabled available from v5.02/00 onwards
//SOLUTION:
//Include the following lines at the beginning of the code
//where SetEnabled is used:
//    #ifdef __WITHOUT_TGBUTTON_SETENABLED
/* #undef SetEnabled */
//       SetState( flag ? kButtonUp : kButtonDisabled )
//    #endif
//and then after the code add the following:
//    #ifdef __WITHOUT_TGBUTTON_SETENABLED
//    #undef SetEnabled
//    #endif
//Make sure that the argument is given even for default
//i.e. you must have SetEnabled(kTRUE), not SetEnabled()
//
//N.B. for signals and slots such as
//   obj->Connect("Toggled(Bool_t)", "TGTextButton", text_button, "SetEnabled(Bool_t)");
//you need to write your own intermediate slot which will receive
//the emitted signal and set the button state accordingly, as in this example:
//
//  #ifdef __WITHOUT_TGBUTTON_SETENABLED
//     void KVINDRARunSheetGUI::SetViewTreeEnabled(Bool_t on)
//     {
//	fViewTreeButton->SetState(on ? kButtonUp : kButtonDisabled );
//     }
//  #endif
//
//  #ifdef __WITHOUT_TGBUTTON_SETENABLED
//     cb->Connect("Toggled(Bool_t)", "KVINDRARunSheetGUI", this, "SetViewTreeEnabled(Bool_t)");
//     fViewTreeButton = tb;
//  #else
//     cb->Connect("Toggled(Bool_t)", "TGTextButton", tb, "SetEnabled(Bool_t)");
//  #endif
/* #undef __WITHOUT_TGBUTTON_SETENABLED */

//----------------------------TMacro class available from v5.04/00 onwards
/* #undef __WITHOUT_TMACRO */

//--------------------------------TSystem::FindFile available from v5.12/00 onwards
//SOLUTION:    use static method KVBase::FindFile  (same arguments and conventions)
/* #undef __WITHOUT_TSYSTEM_FINDFILE */

//--------------------------------------Corrections to KVString::IsDigit & IsFloat
//The original KaliVeda versions of IsDigit and IsFloat were corrected
//in ROOT version 5.13/05. The versions in KVString correspond to these
//corrected versions, and should be used instead of TString for ROOT
//versions anterior to 5.13/05
//SOLUTION:   use KVString instead of TString
/* #undef __WITH_KVSTRING_ISDIGIT */
/* #undef __WITH_KVSTRING_ISFLOAT */

//--------Extended KaliVeda TString::Atoi, Atof and IsWhitespace are in TString from ROOT v5.08/00 onwards
//SOLUTION:   use KVString instead of TString
/* #undef __WITH_KVSTRING_ATOI */
/* #undef __WITH_KVSTRING_ATOF */
/* #undef __WITH_KVSTRING_ISWHITESPACE */

//-------------------------TNetSystem(const char*, Bool_t = kTRUE) exists from v5.10 onwards
/* #undef __WITHOUT_TNETSYSTEM_CTOR_BOOL_T */

//-------------------------TGNumberEntry::SetNumStyle & SetNumAttr exist from v5.11.02 onwards
/* #undef __WITHOUT_TGNUMBERENTRY_SETNUMSTYLE */
/* #undef __WITHOUT_TGNUMBERENTRY_SETNUMATTR */

//-------------------------TGComboBox::RemoveAll & Select(Int_t, Bool_t=kTRUE) exist from v5.12.00 onwards
/* #undef __WITHOUT_TGCOMBOBOX_REMOVEALL */
/* #undef __WITHOUT_TGCOMBOBOX_SELECT_BOOL_T */

//------------------THtml::SetProductName added in v5.17.00 (? - check this)
/* #undef __WITHOUT_THTML_SETPRODUCTNAME */

//------------------TRefArray ctors and AddAt modified in v5.20.00
/* #undef __WITH_OLD_TREFARRAY */

//------------------------New TCollection::Print methods in v5.21.02
#define __WITH_NEW_TCOLLECTION_PRINT

//------------------------TCollection::Print(wildcard,option) from v4.01.04 to v5.20.00
/* #undef __WITH_TCOLLECTION_PRINT_WILDCARD */

//------------------TClonesArray::ConstructedAt method from v5.32.00 onwards
/* #undef __WITHOUT_TCA_CONSTRUCTED_AT */

//--------TString::IsBin/Oct/Dec, Itoa, Ltoa etc. are in TString from ROOT v5.33.02 onwards
//SOLUTION:   use KVString instead of TString
/* #undef __WITH_KVSTRING_ITOA */

#endif
