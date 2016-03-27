#include <stdio.h>
#include <string.h>
#include <iostream>
using namespace std;

#include "lua_scripts.h"
#include "lua.hpp"

#include "VolumeInterface.h"
#include "IFILES_Volume.h"
#include "FAT_Volume.h"

bool Lua_DoCall(lua_State *Lua, const char *func, const char *sig, ...);

#define l_PrepareStack(L) 	int pcount = lua_gettop(L), stack = 0
#define l_get_bool(L) 		((pcount > stack)?(lua_toboolean(L, ++stack)):(false))
#define l_get_ptr(L) 		((pcount > stack)?(lua_topointer(L, ++stack)):(0))
#define l_get_string(L) 	((char*)((pcount > stack)?(lua_tostring(L, ++stack)):("")))
#define l_get_int(L) 		((pcount > stack)?(lua_tointeger(L, ++stack)):(0))

/*
#include "global_defs.h"
#include "IFILES_Volume.h"
#include "MFS_Volume.h"
using namespace WIN;*/

//cVolume OpenImage(char* fname)
int l_OpenImage(lua_State* L){
	l_PrepareStack(L);

	cImage *Img = new cImage();
	Img->OpenFile(lua_tostring(L, ++stack), 512);

	VolumeInterface *Vol = 0;
	if(Img->IsReady()){
		Vol = new FAT_Volume(Img);
		if(Vol->LastError(true) > 1){
			delete Vol;
			delete Img;
			Vol = 0;
			Img = 0;
		}
	}
	Img->Volume = Vol;
	lua_pushlightuserdata(L, Img);
	return 1;
}

//DeleteFile(volume, pat)
int l_DeleteFile(lua_State* L){
	l_PrepareStack(L);
	VolumeInterface *vol = (VolumeInterface*)l_get_ptr(L);
	if(!vol)return 0;
	vol->DeleteFile(l_get_string(L));
	return 0;
}

//InsertFile(volume, sys_path, vol_path)
int l_InsertFile(lua_State* L){
	l_PrepareStack(L);

	cImage *Img = (cImage*)l_get_ptr(L);
	if(!Img)return 0;
	VolumeInterface *vol = Img->Volume;

	char* fn = l_get_string(L);
	char* vn = l_get_string(L);

	if(vol)vol->InsertFile(vn,fn);

	return 0;
}

//CreateDir(volume, dir)
int l_CreateDir(lua_State* L){
	l_PrepareStack(L);
	VolumeInterface *vol = (VolumeInterface*)l_get_ptr(L);
	if(!vol)return 0;
	vol->CreateDir(l_get_string(L));
	return 0;
}

//handle CreteFile(volume, file)(prawa dosêpu zawsze najwy¿sze)
int l_CreteFile(lua_State* L){
	l_PrepareStack(L);

	VolumeInterface *vol = (VolumeInterface*)l_get_ptr(L);
	if(vol){
		unsigned h;
		if((h = vol->CreateFile(l_get_string(L), FILE_ACCES_FULL | FILE_OPEN_ALLWAYS)) != 0){
			lua_pushinteger(L, h);
			return 1;
		}
		vol->CloseFile(h);
	}
	lua_pushinteger(L, 0);
	return 1;
}

//CloseFile(volume, handle)
int l_CloseFile(lua_State* L){
	l_PrepareStack(L);
	VolumeInterface *vol = (VolumeInterface*)l_get_ptr(L);
	if(!vol)return 0;
	vol->CloseFile(l_get_int(L));
	return 0;
}

//int GetFileInfo(volume, handle, parametr_pliku)(prawa dosêpu zawsze najwy¿sze)
int l_GetFileInfo(lua_State* L){
	l_PrepareStack(L);

	VolumeInterface *vol = (VolumeInterface*)l_get_ptr(L);
	if(!vol){
        lua_pushinteger(L, 0);
        return 1;
	}
    unsigned h, value = 0;

	if((h = vol->CreateFile(l_get_string(L), FILE_OPEN_EXISTING)) != 0){
		char* p = l_get_string(L);

		if(!strcmp(p, "date"))value = vol->GetFileDate(h);
        else if(!strcmp(p, "size"))value = vol->GetFileSize(h);
//printf("get: %s %d\n", p, value);
        vol->CloseFile(h);
	}
	if(vol->LastError() == 0){

		lua_pushinteger(L, value);
		return 1;
	}

	lua_pushinteger(L, 0);
	return 1;
}

//GetSystemDate()
int l_GetSysDate(lua_State* L){
//	l_PrepareStack(L);
//	lua_pushinteger(L, GetSystemTime_mfs());
	return 0;//1;
}

//GetSysFileDate(sys_path)
/*int l_GetSysFileDate(lua_State* L){
    l_PrepareStack(L);
	lua_pushinteger(L, GetSysFileTime_SFS(l_get_string(L)));
	return 1;
}*/

int l_List(lua_State* L){
	l_PrepareStack(L);

	VolumeInterface *vol = (VolumeInterface*)l_get_ptr(L);
	if(!vol)return 0;
	vol->List(0);
	return 0;
}

//Format(BS, )
int l_Format(lua_State* L){
	l_PrepareStack(L);

	cImage *Img = (cImage*)l_get_ptr(L);
	VolumeInterface *Vol = Img->Volume;

	char *fstype = l_get_string(L);
	char *imgType = l_get_string(L);
	char *VolName = l_get_string(L);

	delete Vol;

	Vol = new FAT_Volume(Img);
	Vol->Format(VolName, &PredefinedGeometry[Geometry_Fdd144M]);

	Img->Volume = Vol;
	return 0;
}

int l_CheckError(lua_State* L){
	l_PrepareStack(L);

	cImage *Img = (cImage*)l_get_ptr(L);
	VolumeInterface *vol = Img->Volume;
	if(!vol)
		lua_pushinteger(L, 1);
	else
		lua_pushinteger(L, vol->LastError());
	return 1;
}

int l_CreateVolume(lua_State* L){
	l_PrepareStack(L);

	VolumeInterface *vol;
	char* fname = l_get_string(L);
	char* fsys = l_get_string(L);
	char* format = strchr(fsys, ':');
	if(format)
        *format++ = 0;

	if(!strcmp(fsys, "IFILES")){
	    if(!format){
            lua_pushlightuserdata(L, 0);
            return 1;
	    }
		vol = new IFILES_Volume(fname, format);
	} else
	/*if(!strcmp(fsys, "SFS")){
		vol = new SFS_Volume(fname);
	} else*/ return 0;

	lua_pushlightuserdata(L, vol);
	return 1;
}

int l_CloseImage(lua_State* L){
	l_PrepareStack(L);

	cImage *Img = (cImage*)l_get_ptr(L);
	VolumeInterface *Vol = Img->Volume;
	delete Vol;
	delete Img;
	return 0;
}

#define lua_registerI(V, N) lua_pushinteger(Lua, V); 		lua_setglobal(Lua, N)
#define lua_registerN(V, N) lua_pushnumber(Lua, V); 		lua_setglobal(Lua, N)
#define lua_registerB(V, N) lua_pushboolean(Lua, V); 		lua_setglobal(Lua, N)
#define lua_registerV(V, N) lua_pushlightuserdata(Lua, V);	lua_setglobal(Lua, N)
#define lua_registerF(V, N) lua_pushcfunction(Lua, V); 		lua_setglobal(Lua, N)
#define lua_registerS(V, N) lua_pushstring(Lua, V); 		lua_setglobal(Lua, N)

char* paramstr;

int l_GetNextParam(lua_State* Lua){
	//l_PrepareStack(Lua);
	char* Ret = 0;
	if(paramstr){
		Ret = paramstr;
		paramstr = strchr(paramstr, ' ');
		if(paramstr)*paramstr++ = 0;
	}

	lua_pushstring(Lua, Ret);
	return 1;
}

int DoScript(char *ScriptFileName, char* FunctionName, char* param){
//	printf("param %s\n", param);
	lua_State *Lua = lua_open();
	luaopen_base(Lua);

	lua_registerF(l_OpenImage, "OpenImage");
//	lua_registerF(l_CreateVolume, "CreateVolume");
	lua_registerF(l_CloseImage, "CloseImage");

//	lua_registerF(l_DeleteFile, "DeleteFile");
	lua_registerF(l_InsertFile, "InsertFile");
//	lua_registerF(l_CreteFile, "CreateFile");
//	lua_registerF(l_CloseFile, "CloseFile");
//	lua_registerF(l_GetFileInfo, "GetFileInfo");

//	lua_registerF(l_GetSysDate, "GetSysDate");
//	lua_registerF(l_GetSysFileDate, "GetSysFileDate");

	lua_registerF(l_Format, "Format");
	lua_registerF(l_CheckError, "CheckError");

//	lua_registerF(l_GetNextParam, "GetNextParam");

//	lua_registerF(l_List, "List");

	paramstr = param;
//	char* scrn = strrchr(ScriptFileName, '\\');
//	lua_registerS((scrn != 0)?(scrn):(ScriptFileName), "ScriptFile");

	if(luaL_loadfile(Lua, ScriptFileName) != 0){
		lua_close(Lua);
		return 1;
	}

	lua_pcall(Lua, 0, 0, 0);

	unsigned ret;
	if(!Lua_DoCall(Lua, FunctionName, "i>i", 0, &ret)){
		lua_close(Lua);
		return 2;
	}
	if(ret != 0)ret += 100;

	lua_close(Lua);
	return ret;

/*	else if(!strcmp(Cmd, "extract")){//extract co gdzie
			u32 H, S;
			Volume.CreateFile(Params[1], &H, famRead, fomOpenExisting);
			Volume.GetFileSize(H, &S);
			char *Dane = new char[S+1];
			ZeroMemory(Dane, S+1);
			u32 Read = 0;
			ResCode = Volume.ReadFileData(H, Dane, S, &Read);
			Volume.CloseFile(H);
			HANDLE F = CreateFileA(Params[2], GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
			if(F != INVALID_HANDLE_VALUE){
				DWORD W = 0;
				WriteFile(F, Dane, S, &W, NULL);
				if(W != S) ResCode = 1;
			} else ResCode = 2;
			CloseHandle(F);
			delete[] Dane;
		}	*/
}

bool Lua_DoCall(lua_State *Lua, const char *func, const char *sig, ...){
	va_list vl;
	int narg, nres;  /* number of arguments and results */

	va_start(vl, sig);
	lua_getglobal(Lua, func);  /* get function */

	/* push arguments */
	narg = 0;
	while (*sig) {  /* push arguments */
		switch (*sig++) {
		  case 'd':  /* double argument */
			lua_pushnumber(Lua, va_arg(vl, double));
			break;
		  case 'i':  /* int argument */
			lua_pushnumber(Lua, va_arg(vl, int));
			break;
		  case 's':  /* string argument */
			lua_pushstring(Lua, va_arg(vl, char *));
			break;
		  case '>':
			goto endwhile;
		  default:
		   return false;
	  }
	  narg++;
		luaL_checkstack(Lua, 1, "too many arguments");
	} endwhile:

	/* do the call */
	nres = strlen(sig);  /* number of expected results */
	if (lua_pcall(Lua, narg, nres, 0) != 0)  /* do the call */
	  return false;

	/* retrieve results */
	nres = -nres;  /* stack index of first result */
	while (*sig) {  /* get results */
		switch (*sig++) {
		  case 'd':  /* double result */
			if (!lua_isnumber(Lua, nres)) return false;
//			error(Lua, "wrong result type");
			*va_arg(vl, double *) = lua_tonumber(Lua, nres);
			break;
		  case 'i':  /* int result */
			if (!lua_isnumber(Lua, nres)) return false;
			 // error(Lua, "wrong result type");
			*va_arg(vl, int *) = (int)lua_tonumber(Lua, nres);
			break;
		  case 's':  /* string result */
			if (!lua_isstring(Lua, nres)) return false;
		   //	  error(Lua, "wrong result type");
			*va_arg(vl, const char **) = lua_tostring(Lua, nres);
			break;
		  default:
			return false;
		}
		nres++;
	}
	va_end(vl);
	return true;
}
