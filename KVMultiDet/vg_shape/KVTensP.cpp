//
//Author: Daniel Cussol 
//
// 18/02/2004:
// Creation d'une classe Variable Globale.
// Tenseur des moments, base sur KVTenseur3.
//

#include "Riostream.h"
#include "KVTensP.h"

ClassImp(KVTensP)
//////////////////////////////////////////////////////////////////////////////////
// This class fills, diagonalizes the momentum tensor for particles in an
// event and returns several useful values. This is based on the KVTenseur3 class.
//
//By default, only particles with Z >=  Zmin (default value=3) are included in
//the calculation. Use SetZmin(0) or SetZmin(1) to include all particles.
//
//Use SetPartLabel() in order to use only particles with a given label.
//Note that the condition on the particle's Z is still used, thus
//if e.g., SetZmin(10) and  SetPartLabel("alpha") are used, only
//particles with the name/label "alpha" and Z>=10 will be included. 
//
//By default the tensor is calculated from particle momenta
//in the default (laboratory) frame.
//However, this can be changed by giving the name of the desired frame
//to either the constructor
//      KVTensP* tp = new KVTensP("my_tensor", "CM"); // calculate in "CM" frame
//or by using method SetFrame
//      tp->SetFrame( "QP" ); // calculate in "QP" frame
//
// The names of the variables and the corresponding indices are as follows:
//
//      Name            Index                   Meaning
//----------------------------------------------------------------------------
//      ThetaFlot       0       (default)       Theta flow (in degrees between -90 and +90)
//      PhiPlan         1                       Phi of the reaction plane (in degrees between 0 and 360)
//      Vap1            2                       Value of the 1st eigenvalue
//      Vap2            3                       Value of the 2nd eigenvalue
//      Vap3            4                       Value of the 3rd eigenvalue
//      Sphericite      5                       Sphericity
//      Coplanarite     6                       Coplanarity
//      Directivite     7       (not computed)  Directivity
//
// All these values can be obtained by calling the GetValuePtr() method which
// returns an array of Double_t containing the values.
//
// Here are examples on how to obtain the values.
//
// KVTensP *ptens=new KVTensP();
// KVTensP tens;
// ...
// Double_t tf=ptens->GetValue();  |
//          tf=ptens->GetValue(0); |--> Theta Flow
//          tf=tens("ThetaFlot");  |
// Double_t phiplane=tens(1);---------> Phi of reaction plane
// Double_t copla=ptens->GetValue("Coplanarite");--> Coplanarity
//
// Double_t *values=ptens->GetValuePtr();
// Double_t s=values[5] --------------> Sphericity
//          tf=values[0] -------------> Theta Flow
//
// Look at KVVarGlob class to have an example of use.
//
//
Int_t KVTensP::nb = 0;
Int_t KVTensP::nb_crea = 0;
Int_t KVTensP::nb_dest = 0;

//_________________________________________________________________
void KVTensP::init_KVTensP(void)
{
   nb++;
//
// Initialisation des champs de KVTensP
// Cette methode privee n'est appelee par les createurs
//
   nb_crea++;

   tenseurP = new KVTenseur3();
   SetZmin(3);
   fLabel = "";
   SetNameIndex("ThetaFlot", 0);
   SetNameIndex("PhiPlan", 1);
   SetNameIndex("Vap1", 2);
   SetNameIndex("Vap2", 3);
   SetNameIndex("Vap3", 4);
   SetNameIndex("Sphericite", 5);
   SetNameIndex("Coplanarite", 6);
   SetNameIndex("Directivite", 7);
}


//_________________________________________________________________
KVTensP::KVTensP(void):KVVarGlob()
{
//
// Createur par default
//
   Char_t *nom = new Char_t[80];

   init_KVTensP();
   sprintf(nom, "KVTensP_%d", nb_crea);
   SetName(nom);
   SetTitle(nom);
#ifdef DEBUG_KVTensP
   cout << nb << " crees...(defaut) " << endl;
#endif
   delete[]nom;
}

//_________________________________________________________________
KVTensP::KVTensP(Char_t * nom):KVVarGlob(nom)
{
//
// Constructeur avec un nom
//
   init_KVTensP();
#ifdef DEBUG_KVTensP
   cout << nb << " crees...(nom) " << endl;
#endif
}

//_________________________________________________________________
KVTensP::KVTensP(Char_t * nom, const Char_t * frame):KVVarGlob(nom)
{
   //Set name of object, and name of frame for calculation of tensor

   init_KVTensP();
#ifdef DEBUG_KVTensP
   cout << nb << " crees...(nom) " << endl;
#endif
   fFrame = frame;
}

//_________________________________________________________________
KVTensP::KVTensP(const KVTensP & a)
{
// 
// Contructeur par Copy
//
   init_KVTensP();
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   a.Copy(*this);
#else
   ((KVTensP &) a).Copy(*this);
#endif
#ifdef DEBUG_KVTensP
   cout << nb << " crees...(Copy) " << endl;
#endif
}

//_________________________________________________________________
KVTensP::~KVTensP(void)
{
// 
// Destructeur
//
#ifdef DEBUG_KVTensP
   cout << "Destruction de " << GetName() << "..." << endl;
#endif
   nb--;

   nb_dest++;

   if (tenseurP)
      delete tenseurP;
}

//_________________________________________________________________
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
void KVTensP::Copy(TObject & a) const
#else
void KVTensP::Copy(TObject & a)
#endif
{
//
// Methode de Copy
//
   KVVarGlob::Copy(a);
   ((KVTensP &) a).SetZmin(GetZmin());
   ((KVTensP &) a).SetTensor(GetTensor());
}

//_________________________________________________________________
KVTensP & KVTensP::operator =(const KVTensP & a)
{
//
// Operateur =
//
#ifdef DEBUG_KVTensP
   cout << "Copy par egalite de " << a.GetName() << "..." << endl;
#endif
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   a.Copy(*this);
#else
   ((KVTensP &) a).Copy(*this);
#endif
#ifdef DEBUG_KVTensP
   cout << "Nom de la Copy par egalite: " << GetName() << endl;
#endif
   return *this;
}

//_________________________________________________________________
void KVTensP::Init(void)
{
// methode d'initialisation des
// variables internes
   tenseurP->Reset();
}

//_________________________________________________________________
void KVTensP::Reset(void)
{
// Remise a zero avant le
// traitement d'un evenement
   tenseurP->Reset();
}

//_________________________________________________________________

void KVTensP::Fill(KVNucleus * c)
{
   //Add the particle's contribution to the momentum tensor.
   //
   //This particle will be rejected if:
   //(1) it has Z < Zmin
   //(2) it does not have the name/label defined with SetPartLabel**
   //
   // **if no label defined, only Z>=Zmin condition is used.
   //
   //This uses the 3 cartesian components of the particle's velocity vector
   //with a weight equal to the particle's A.
   //
   //By default the velocity vector in the default (laboratory) frame is used.
   //To calculate in another frame, use KVVarGlob::SetFrame() before
   //calculation begins

   //if particle label has been set, check name of particle
   //if not, accept all
   Bool_t ok = ((fLabel != "") ? (fLabel == c->GetName()) : kTRUE);

   //check Z of particle
   if ((c->GetZ() >= GetZmin()) && ok) {
      Double_t poids = (Double_t) c->GetA();
      tenseurP->Fill(c->GetFrame(fFrame.Data())->GetV(), poids);
   }
}

//_________________________________________________________________
Double_t KVTensP::getvalue_void(void) const
{
	// On retourne la valeur de la variable Theta_flot.
   return tenseurP->GetThetaFlot();
}

//_________________________________________________________________
Double_t *KVTensP::GetValuePtr(void)
{
// On retourne un tableau de valeurs. il est organise comme suit 
//
// Index  Meaning
//--------------------------------------
// 0      Theta flow
// 1      Phi of the reaction plane
// 2      Value of the 1st eigenvalue
// 3      Value of the 2nd eigenvalue
// 4      Value of the 3rd eigenvalue
// 5      Sphericity
// 6      Coplanarity
// 7      Directivity
//
//  USER MUST DELETE ARRAY AFTER USING !!!

   Double_t *v = new Double_t[8];
   v[0] = tenseurP->GetThetaFlot();
   v[1] = tenseurP->GetPhiPlan();
   v[2] = tenseurP->GetVap(1);
   v[3] = tenseurP->GetVap(2);
   v[4] = tenseurP->GetVap(3);
   v[5] = tenseurP->GetSphericite();
   v[6] = tenseurP->GetCoplanarite();
   v[7] = tenseurP->GetDirectivite();
   return v;
}

//_________________________________________________________________
Double_t KVTensP::getvalue_int(Int_t i)
{
// on retourne la ieme valeur du tableau
//
// Index   Meaning
//---------------------------------------
// 0       Theta flow
// 1       Phi of the reaction plane
// 2       Value of the 1st eigenvalue
// 3       Value of the 2nd eigenvalue
// 4       Value of the 3rd eigenvalue
// 5       Sphericity
// 6       Coplanarity
// 7       Directivity
//
//
   Double_t rval = 0;
   switch (i) {
   case 0:
      rval = tenseurP->GetThetaFlot();
      break;

   case 1:
      rval = tenseurP->GetPhiPlan();
      break;

   case 2:
      rval = tenseurP->GetVap(1);
      break;

   case 3:
      rval = tenseurP->GetVap(2);
      break;

   case 4:
      rval = tenseurP->GetVap(3);
      break;

   case 5:
      rval = tenseurP->GetSphericite();
      break;

   case 6:
      rval = tenseurP->GetCoplanarite();
      break;

   case 7:
      rval = tenseurP->GetDirectivite();
      break;

   default:
      rval = 0.;
      break;
   }
   return rval;
}


//_________________________________________________________________

void KVTensP::SetTensor(const KVTenseur3 * KVT)
{
   //copy tenseur values into "this" tenseur
   KVT->Copy(*tenseurP);
}