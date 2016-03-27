;SFS nasm headers

%define FilesPerSector 		8 	;ilosc struktur plikow w sektorze
%define FileNameLength 		18 	;maksymalna dlugosc nazwy pliku
%define FileStructureSize 	64 	;rozmiar struktury pliku
%define FileSegmentsOffset 	29	;offset pozycji segmentu 1
%define FileSegments 		5

STRUC	SFS_BIS
	.JumpCode			resb 3
	.FSName				resb 5
	.BytesPerSector		resw 1
	.SectorsPerCluster	resb 1
	.SectorsPerTrack	resw 1
	.NumberOfHeads		resw 1
	.PartitionID		resd 1
	.TotalSectors		resd 1
	.FileStructPos		resd 1
	.FileStructSize		resd 1
	.DriveNumber		resb 1
	.Version			resw 1
	.MediaType			resb 1
	.Code				resb 475
	.Signature			resw 1
ENDSTRUC

STRUC   SFS_file 
	.Name:			resb 18
	.ParentDir:		resw 1
	.Attributes:	resb 1
	.Date:			resd 1
	.Size:			resd 1
	.Segment1:		resb 7
	.Segment2:		resb 7
	.Segment3:		resb 7
	.Segment4:		resb 7
	.Segment5:		resb 7
ENDSTRUC
