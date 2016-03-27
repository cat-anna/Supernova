#include "IVolume.h"

IVolume_p GetIVolumeFile(IVolume_p vol, const char* file){
	if(!vol)return 0;
	if(!file || !*file)return (IVolume_p)vol->Children;

	char* p = (char*)file;
	if(*p == '\\')p++;

	IVolume_p ret = (IVolume_p)vol->Children;
	if(!ret)return 0;

    while(*p) {
        if(strcmp2delim(vol->Name, p, '\\') == 0) {
           p = strchr(p, '\\');
           if(p)p++;
           else return ret;
           vol = (IVolume_p)vol->Children;
        } else {
        	vol = (IVolume_p)vol->Next;
        }
    }
    return ret;
}

uint32 EnumIVolumeFiles(IVolume_p *vol){
	if(!vol || !*vol)return 0;
	IVolume_p v = *vol;
	v = v->Next;
	*vol = v;
	return v != 0;
}
