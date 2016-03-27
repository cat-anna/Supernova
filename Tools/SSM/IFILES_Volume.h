#ifndef IFILES_VOLUME_H
#define IFILES_VOLUME_H

#include "VolumeInterface.h"

class IFILES_Volume;

class IFILES_node {
    friend class IFILES_Volume;
public:
    IFILES_node(char *sys, char* vol, IFILES_Volume *Vol);
    IFILES_node(const char* lab, short flags, IFILES_Volume *Vol);
    ~IFILES_node();

    void AddFile(IFILES_node *f);
    void Clean();
    void WriteStructure(unsigned handle);
    void ListTree();
    unsigned FileSize;
    unsigned FileDate;
    int CheckSysFile();
    int WriteFileData(unsigned F);
protected:
    IFILES_Volume *Volume;
    char *label;
    char *name;
    char *path;
    char *sys_name;
    unsigned Status;
    short attributes;
    IFILES_node *next;
    IFILES_node *child;
    void SubListTree(IFILES_node *node, unsigned level);
    void DoGenLabel(char* src);
    void SetPaths(char *sys, char* vol);
    void InsertNext(IFILES_node *n);
    void InsertChild(IFILES_node *n);
};

class IFILES_Volume : public VolumeInterface {
friend class IFILES_node;
public:
    IFILES_Volume(char* fname, char* format);
    ~IFILES_Volume();

    void InsertFile(char* vname, char* sysname);
    void List();
protected:
    char* FName;
    unsigned type;
    unsigned WinH;
    IFILES_node *node_root;
    void WriteFileData(IFILES_node *file);
};

#endif // IFILES_VOLUME_H
