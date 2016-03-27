
#define MakeFun(A)  \
int A (){			\
	return 0;		\
}

MakeFun(kprintf);

MakeFun(RegisterDriver);
MakeFun(RegisterDevice);
MakeFun(RegisterVolume);
MakeFun(RegisterIRQ);

MakeFun(kmalloc);
MakeFun(kfree);
MakeFun(Lowkmalloc);
MakeFun(Lowkfree);

MakeFun(ReleaseLock);
MakeFun(AcquireLock);

MakeFun(DirectOpen);
MakeFun(DirectClose);
MakeFun(DirectRead);
MakeFun(DirectWrite);
MakeFun(DirectSeek);
MakeFun(DirectCommand);

MakeFun(InternalMessage);
MakeFun(InternalGetMessage);

MakeFun(GetDeviceName);

//MakeFun();
//MakeFun();
//MakeFun();
//MakeFun();
//MakeFun();
//MakeFun();
//MakeFun();
//MakeFun();
//MakeFun();
//MakeFun();
//MakeFun();
//MakeFun();
