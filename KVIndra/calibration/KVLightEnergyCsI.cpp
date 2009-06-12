/***************************************************************************
$Id: KVLightEnergyCsI.cpp,v 1.10 2008/10/07 15:55:20 franklan Exp $
                          KVLightEnergyCsI.cpp  -  description
                             -------------------
    begin                : 18/5/2004
    copyright            : (C) 2004 by J.D. Frankland
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
#include "KVLightEnergyCsI.h"
#include "TMath.h"
#include "KVCsI.h"

ClassImp(KVLightEnergyCsI);

////////////////////////////////////////////////////////////////////////////
//Light-energy calibration for INDRA CsI detectors.
//
//The function Double_t CalculLumiere(Double_t * x, Double_t * par) (in the source file KVLightEnergyCsI.cpp)
//is used to parameterize the total light output as a function of deposited energy (in MeV).
//
//This function has 3 parameters, a1, a2 & a3. In order to correctly reproduce the light-energy
//relationship for all ions, two parameterizations have to be used: one for Z=1 and another for Z>1.
//The parameter a3 normally has a fixed value (a3=6), but this is not "hard-coded" : it should be fixed
//when fitting data.

//___________________________________________________________________________

Double_t CalculLumiere(Double_t * x, Double_t * par)
{
   //Calcul de la lumiere totale a partir de Z, A d'une particule et son energie
   // 
   // x[0] = energie (MeV)
   // par[0] = a1
   // par[1] = a2
   // par[2] = a3 
   // par[3] = Z
   // par[4] = A

   Double_t Z = par[3];
   Double_t A = par[4];
   Double_t energie = x[0];
   Double_t c1 = par[0];
   Double_t c2 = Z * Z * A * par[1];
   Double_t c3 = A * par[2];
   Double_t c4 = 0.385; // was = 0.27
   Double_t T = 8 * A;
   Double_t c4_new = c4 / (1. + TMath::Exp((c3 - energie) / T));
   Double_t c0 = c4 / (1. + TMath::Exp(c3 / T));
   
   Double_t lumcalc = c1 * energie;
   if( c2 > 0.0 ){
      Double_t xm = -c1 * c0 * c2 * TMath::Log(c2 / (c2 + c3));
      lumcalc = lumcalc - c1* c2 * TMath::Log(1. + energie / c2)+ c1 * c2 * c4_new * TMath::Log((energie + c2) /(c3 + c2)) + xm;
   }
       
   return lumcalc;
}

TF1 KVLightEnergyCsI::fLight("fLight_CsI", CalculLumiere, 0., 10000., 5);

//__________________________________________________________________________

void KVLightEnergyCsI::init()
{
   //default initialisations
   SetType("Light-Energy CsI");
   SetA(1);
   SetZ(1);
}

KVLightEnergyCsI::KVLightEnergyCsI():KVCalibrator(3)
{
   init();
}

//___________________________________________________________________________
KVLightEnergyCsI::KVLightEnergyCsI(KVDetector * kvd):KVCalibrator(3)
{
   //Create an electronic calibration object for a specific detector (*kvd)
   init();
   SetDetector(kvd);
}

//___________________________________________________________________________
Double_t KVLightEnergyCsI::Compute(Double_t light) const
{
   // Calculate the calibrated energy (in MeV) for a given total light output.
   // The Z and A of the particle should be given first using SetZ, SetA.
   // By default, Z=A=1 (proton).
   //
   // This is done by inversion of the light-energy function using TF1::GetX.

   //set parameters of light-energy function
   Double_t par[5];
   for (int i = 0; i < 3; i++)
      par[i] = GetParameter(i);
   par[3] = (Double_t) fZ;
   par[4] = (Double_t) fA;
   fLight.SetParameters(par);
   
   //invert light vs. energy function to find energy
	Double_t xmin, xmax; fLight.GetRange(xmin,xmax);
   Double_t energy = fLight.GetX(light, xmin, xmax);

   return energy;
}


//___________________________________________________________________________
Double_t KVLightEnergyCsI::operator() (Double_t light) {
   //Same as Compute()

   return Compute(light);
}

//___________________________________________________________________________
Double_t KVLightEnergyCsI::Invert(Double_t energy)
{
   //Given the calibrated (or simulated) energy in MeV,
   //calculate the corresponding total light output according to the
   //calibration parameters (useful for filtering simulations).

   //set parameters of light-energy function
   Double_t par[5];
   for (int i = 0; i < 3; i++)
      par[i] = GetParameter(i);
   par[3] = (Double_t) fZ;
   par[4] = (Double_t) fA;
   fLight.SetParameters(par);

   return fLight.Eval(energy);
}