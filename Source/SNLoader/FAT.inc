;FAT nasm headers


STRUC FAT_BPB
	.BootJmp 			resb 3
	.OEM				resb 8
	.BytesPerSector		resw 1
	.SectorsPerCluster	resb 1
	.ReservedSectors	resw 1
	.NumberOfFATs		resb 1
	.RootEntryCount		resw 1
	.TotalSecotrs16		resw 1
	.MediaType			resb 1
	.SectorsPerFAT		resw 1
	.SectorsPerTrack	resw 1
	.NumberOfHeads		resw 1
	.HiddenSectors		resd 1
	.TotalSectors32		resd 1
	.ThisSize			resb 1
ENDSTRUC

STRUC FAT_FileEntry
	.Name				resb 8
	.Ext				resb 3
	.Attributes			resb 1
	.NTReserved			resb 1
	.CreationTime_ms	resb 1
	.CreationTime		resw 1
	.CreationDate		resw 1
	.LastAccessedDate	resw 1
	.HighCluster		resw 1
	.ModificationDate	resw 1
	.ModificationTime	resw 1
	.LowCluster			resw 1
	.FileSize			resd 1
	.ThisSize			resb 1
ENDSTRUC

STRUC FAT12_EXT_BPB
	.BPB				resb 36
	.DriveNumber		resb 1
	.Reserved			resb 1
	.BootSignature		resb 1
	.VolumeId			resd 1
	.VolumeLabel		resb 11
	.FatTypeLabel		resb 8
ENDSTRUC


