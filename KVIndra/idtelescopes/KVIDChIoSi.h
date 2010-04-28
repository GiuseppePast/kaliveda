/***************************************************************************
                          KVIDChIoSi.h  -  description
                             -------------------
    begin                : Fri Feb 20 2004
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

#ifndef KVIDChIoSi_H
#define KVIDChIoSi_H

#include "KVIDTelescope.h"
#include "KVIDGChIoSi.h"

class KVIDChIoSi:public KVIDTelescope {

protected:

   KVIDGChIoSi* ChIoSiGrid;//!
	KVDetector *fchio;//!
	KVDetector *fsi;//!
	Double_t fsipgped;//!
	Double_t fchiopgped;//!


 public:

    KVIDChIoSi();
    virtual ~ KVIDChIoSi();

   virtual Bool_t Identify(KVReconstructedNucleus *);

   Double_t GetIDMapX(Option_t * opt = "");
   Double_t GetIDMapY(Option_t * opt = "");

   virtual void Initialize();

   const Char_t *GetIDSubCodeString(KVIDSubCode & concat) const;

    ClassDef(KVIDChIoSi, 1)     //INDRA identification using ChIo-Si matrices
};

#endif