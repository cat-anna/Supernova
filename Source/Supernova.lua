SN_img = "../binary/Supernova.img"

function BuildImage(DoFormat)
	Img = OpenImage(SN_img);
	
	if CheckError(Img) > 0 then
		print("Formating...");
		Format(Img, "FAT12", "FDD:1.44M", "SNDisk");
	end;
	
	InsertFile(Img, "../Binary/SNLoader/SNLoader.bin", "/SNLoader.bin");
	InsertFile(Img, "../Binary/kernel/Supernova.elf", "/Supernova.elf");
	InsertFile(Img, "../Binary/shared/Supernova.lib", "/Supernova.lib");
	InsertFile(Img, "../Binary/Drivers/FDD/FDD.drv", "/FDD.drv");
	InsertFile(Img, "../Binary/Drivers/SYS/SYS.drv", "/SYS.drv");
	InsertFile(Img, "../Binary/Drivers/FAT/FAT.drv", "/FAT.drv");
	InsertFile(Img, "../Source/Image/System.cfg", "/System.cfg");
	InsertFile(Img, "../Binary/Apps/shell.elf", "/shell.elf");

--	CheckFile(vol, "image/system.cfg", "/system.cfg");	
--	CheckFile(vol, "apps/errapp.elf", "/errapp.elf");
--	CheckFile(vol, "apps/shell.elf", "/shell.elf");
	
--	if DoFormat == 0 and CheckError(vol) > 0 then
--		CloseVolume(vol);
--		return BuildImage(1);
--	end
	
	--List(vol, "/*");
	
	--return CheckError(vol);
	CloseImage(Img);
	return 0;
end
--[[
function CreateIVolume()
	int_f = CreateVolume("ifiles.i", "IFILES:ASM");

	InsertFile(int_f, "../drivers/fdd/fdd.drv");					
	InsertFile(int_f, "../drivers/SFS/SFS.drv");					
	InsertFile(int_f, "../drivers/serial/serial.drv");					
	InsertFile(int_f, "../sharedlibs/Supernova.lib");					
	--List(int_f);
	CloseImage(int_f);
	return 0;
end
]]
