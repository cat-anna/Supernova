/*
	Obs³uga Master Boot Record
*/
#include "types.h"

struct cPartitionData
{
      u8 Status;          
      u8 StartCHS[3]; 
      u8 Type;
      u8 EndCHS[3];
      u32 FirstLBASector;
      u32 SectorsCount;        
}__attribute__((packed));

struct cMasterBootRecord
{
      u8 Code[446];       
      cPartitionData PartitonTable[4];    
      u8 Signature[2];
}__attribute__((packed));

inline void CylSecHeadEncode(u8* CHS, uint16_t Cylinder, u8 Sector, u8 Head)
{
	CHS[0] = Head;
	CHS[1] = ((Cylinder & 0xff00) << 6) | Sector;
	CHS[2] = ((Cylinder & 0xff) >> 8);
}

inline void CylSecHeadDecode(u8* CHS, uint16_t* Cylinder, u8* Sector, u8* Head)
{
	*Head = CHS[0];
    *Cylinder = CHS[2] | (((CHS[1] & 0xff) & 0xC0) << 2);
    *Sector = CHS[1] & 0x3F;		
}

/*
Function CylSecEncode(Cylinder, Sector : Word) : Word;
Begin
    CylSecEncode := (Lo(Cylinder) shl 8) or (Hi(Cylinder) shl 6) or Sector;
End;

Procedure CylSecDecode(Var Cylinder, Sector : Word; CylSec : Word);
Begin
    Cylinder := Hi(CylSec) or ((Lo(CylSec) and $C0) shl 2);
    Sector := (CylSec and $3F);
End;*/
