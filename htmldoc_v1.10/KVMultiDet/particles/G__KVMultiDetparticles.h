/********************************************************************
* G__KVMultiDetparticles.h
* CAUTION: DON'T CHANGE THIS FILE. THIS FILE IS AUTOMATICALLY GENERATED
*          FROM HEADER FILES LISTED IN G__setup_cpp_environmentXXX().
*          CHANGE THOSE HEADER FILES AND REGENERATE THIS FILE.
********************************************************************/
#ifdef __CINT__
#error G__KVMultiDetparticles.h/C is only for compilation. Abort cint.
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#define G__ANSIHEADER
#define G__DICTIONARY
#define G__PRIVATE_GVALUE
#include "G__ci.h"
#include "FastAllocString.h"
extern "C" {
extern void G__cpp_setup_tagtableG__KVMultiDetparticles();
extern void G__cpp_setup_inheritanceG__KVMultiDetparticles();
extern void G__cpp_setup_typetableG__KVMultiDetparticles();
extern void G__cpp_setup_memvarG__KVMultiDetparticles();
extern void G__cpp_setup_globalG__KVMultiDetparticles();
extern void G__cpp_setup_memfuncG__KVMultiDetparticles();
extern void G__cpp_setup_funcG__KVMultiDetparticles();
extern void G__set_cpp_environmentG__KVMultiDetparticles();
}


#include "TObject.h"
#include "TMemberInspector.h"
#include "KVNuclData.h"
#include "KVChargeRadius.h"
#include "KVElementDensityTable.h"
#include "KVMassExcessTable.h"
#include "KVLifeTimeTable.h"
#include "KVAbundance.h"
#include "KVNDTManager.h"
#include "KVNucleus.h"
#include "KVAbundanceTable.h"
#include "KVElementDensity.h"
#include "KVNuclDataTable.h"
#include "KVParticleCondition.h"
#include "KVLifeTime.h"
#include "KVParticle.h"
#include "KVSimNucleus.h"
#include "KVChargeRadiusTable.h"
#include "KVMassExcess.h"
#include <algorithm>
namespace std { }
using namespace std;

#ifndef G__MEMFUNCBODY
#endif

extern G__linked_taginfo G__G__KVMultiDetparticlesLN_TClass;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_TBuffer;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_TMemberInspector;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_TObject;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_TNamed;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_TCollection;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_TString;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_vectorlEROOTcLcLTSchemaHelpercOallocatorlEROOTcLcLTSchemaHelpergRsPgR;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_reverse_iteratorlEvectorlEROOTcLcLTSchemaHelpercOallocatorlEROOTcLcLTSchemaHelpergRsPgRcLcLiteratorgR;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_TObjArray;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_vectorlETVirtualArraymUcOallocatorlETVirtualArraymUgRsPgR;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_reverse_iteratorlEvectorlETVirtualArraymUcOallocatorlETVirtualArraymUgRsPgRcLcLiteratorgR;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_KVNuclData;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_KVNuclDatacLcLEStatusBits;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_KVChargeRadius;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_TMap;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_iteratorlEbidirectional_iterator_tagcOTObjectmUcOlongcOconstsPTObjectmUmUcOconstsPTObjectmUaNgR;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_TSeqCollection;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_maplEstringcOTObjArraymUcOlesslEstringgRcOallocatorlEpairlEconstsPstringcOTObjArraymUgRsPgRsPgR;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_KVString;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_NDT;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_NDTcLcLkey;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_NDTcLcLvalue;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_KVNuclDataTable;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_KVElementDensity;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_KVElementDensityTable;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_KVMassExcess;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_KVMassExcessTable;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_KVSeqCollection;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_KVHashList;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_KVNameValueList;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_KVLifeTime;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_KVLifeTimecLcLEStatusBits;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_KVLifeTimeTable;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_KVAbundance;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_KVList;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_KVNDTManager;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_TMatrixTBaselEfloatgR;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_TMatrixTBaselEdoublegR;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_TVectorTlEfloatgR;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_TVectorTlEdoublegR;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_TElementActionTlEfloatgR;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_TElementPosActionTlEfloatgR;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_TMatrixTlEfloatgR;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_TMatrixTRow_constlEfloatgR;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_TMatrixTRowlEfloatgR;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_TMatrixTDiag_constlEfloatgR;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_TMatrixTColumn_constlEfloatgR;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_TMatrixTFlat_constlEfloatgR;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_TMatrixTSub_constlEfloatgR;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_TMatrixTSparseRow_constlEfloatgR;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_TMatrixTSparseDiag_constlEfloatgR;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_TMatrixTColumnlEfloatgR;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_TMatrixTDiaglEfloatgR;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_TMatrixTFlatlEfloatgR;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_TMatrixTSublEfloatgR;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_TMatrixTSparseRowlEfloatgR;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_TMatrixTSparseDiaglEfloatgR;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_TRotation;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_TVector3;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_TLorentzRotation;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_TLorentzVector;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_KVBase;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_KVUniqueNameList;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_KVParticleCondition;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_KVParticle;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_KVParticlecLcLdA;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_KVNumberList;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_KVNucleus;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_KVClassFactory;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_KVNucleuscLcLdA;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_KVAbundanceTable;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_KVSimNucleus;
extern G__linked_taginfo G__G__KVMultiDetparticlesLN_KVChargeRadiusTable;

/* STUB derived class for protected member access */
