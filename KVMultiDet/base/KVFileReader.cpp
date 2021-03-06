//Created by KVClassFactory on Tue Jul 13 11:52:58 2010
//Author: Eric Bonnet

#include "KVFileReader.h"

ClassImp(KVFileReader)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVFileReader</h2>
<h4>Manage the reading of file</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVFileReader::KVFileReader()
{
   // Default constructor
   init();
}

KVFileReader::~KVFileReader()
{
   // Destructor
   if (toks) delete toks;
   toks = 0;
}

//______________________
KVFileReader::KVFileReader(const KVFileReader& obj) : KVBase()
{
   //copy ctor
   init();
   obj.Copy(*this);
}

//___________________________________________________________________________________

void KVFileReader::Copy(TObject& obj) const
{
   //Copy this to obj
   //Redefinition of KVBase::Copy

   KVBase::Copy(obj);
}
