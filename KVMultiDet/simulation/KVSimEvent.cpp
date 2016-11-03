//Created by KVClassFactory on Wed Jun 30 15:10:22 2010
//Author: bonnet

#include "KVSimEvent.h"

#include <KVSimNucleus.h>
using namespace std;

ClassImp(KVSimEvent)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVSimEvent</h2>
<h4>Classe dérivée de KVEvent pour la gestion d'événements issus de simulations</h4>
<!-- */
// --> END_HTML
//Cette classe est couplée à KVSimNucleus
////////////////////////////////////////////////////////////////////////////////

//___________________________
KVSimEvent::KVSimEvent(Int_t mult, const char* classname): KVEvent(mult, classname)
{
   // Default constructor
}

//___________________________
KVSimEvent::~KVSimEvent()
{
   //destructeur
}

void KVSimEvent::Print(Option_t* t) const
{
   //Print a list of all particles in the event with some characteristics.
   //Optional argument t can be used to select particles (="ok", "groupname", ...)

   cout << "\nKVSimEvent with " << ((KVSimEvent*) this)->GetMult(t) << " particles :" << endl;
   cout << "------------------------------------" << endl;
   fParameters.Print();
   KVSimNucleus* frag = 0;
   const_cast<KVSimEvent*>(this)->ResetGetNextParticle();
   while ((frag = (KVSimNucleus*)((KVSimEvent*) this)->GetNextParticle(t))) {
      frag->Print();
      cout << "   Position: (" << frag->GetPosition().x() << "," << frag->GetPosition().y() << "," << frag->GetPosition().z() << ")" << endl;
   }

}

Double_t KVSimEvent::GetTotalCoulombEnergy() const
{
   // Calculate total Coulomb potential energy of all particles in event
   // Units are MeV.

   Double_t Vtot = 0;
   for (KVEvent::Iterator it = begin(); it != end(); ++it) {
      KVEvent::Iterator it2(it);
      for (++it2; it2 != end(); ++it2) {
         TVector3 D12 = static_cast<KVSimNucleus&>(*it).GetPosition() -
                        static_cast<KVSimNucleus&>(*it2).GetPosition();
         Vtot += (*it).GetZ() * (*it2).GetZ() * KVNucleus::e2 / D12.Mag();
      }
   }
   return Vtot;

}
