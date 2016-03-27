#ifndef _IVOLUME_H
#define _IVOLUME_H

typedef struct{
    const unsigned char ssize;
    const char* Name;
    const unsigned fsize;
    const short flags;
    const unsigned time;
    void *fdata;
    void *Next;
    void *Children;
}__attribute__((packed)) IVolume_t, *IVolume_p;

IVolume_p GetIVolumeFile(IVolume_p vol, const char* file);
unsigned EnumIVolumeFiles(IVolume_p *vol);

#endif
