#include "supernova.h"
#include "supernova/di.h"
#include "kbd_dfs.h"

char KBD_NAME[] = "keyboard";

uint32 DriverID;

sint32 KBDIrqCount;

uint8 *KeyMap[4];
uint16 kbd_Status;
static void outb(int x, int y){
	x = y = 0;
}

static int inb(int x){
	x = 0;
	return 0;
}

/*static sint32 read_kbd(void)
{
	unsigned long timeout;
	uint32 stat, data;
	for(timeout = 500000; timeout != 0; timeout--)
	{
		stat = inb(0x64);
	// czekaj gdy bufor klawiatury jest pe³ny
		if(stat & 0x01)
		{
			data = inb(0x60);
		// pêtla, gdt b³¹d parzystoœci, lub koniec czasu oczekiwania 
			if((stat & 0xC0) == 0) return data;
		}
	}
	return -1;
}*/

static void write_kbd(unsigned adr, unsigned data){
	unsigned long timeout;
	unsigned stat;
	for(timeout = 500000; timeout != 0; timeout--)
	{
		stat = inb(0x64);
		/* czekaj gdy bufor klawiatury nie zrobi siê pusty */
		if((stat & 0x02) == 0) break;
	}
	if(timeout == 0)
	{
		puts("write_kbd: timeout\n");
		return;
	}
	outb(adr, data);
}

void KBD_SetLeds(){
	uint8 temp = 0;	
	if(kbd_Status & KBD_META_SCRL)temp |= 1;
	if(kbd_Status & KBD_META_NUM) temp |= 2;
	if(kbd_Status & KBD_META_CAPS)temp |= 4;
	write_kbd(0x60, 0xED);	// komenda "set LEDS" 
	write_kbd(0x60, temp);	
}

sint16 TranslateScanCode(sint16 ScanCode){	
	if(ScanCode < 0)return ScanCode;
	uint16 temp;
/* sprawdŸ czy kod break (np. gdy klawisz zosta³ zwolniony) */
	if(ScanCode & 0x80){
		kbd_Status |= KBD_META_BRKCD;
		ScanCode &= ~0x80;
	}
	if(ScanCode == 0xE0){
		kbd_Status |= KBD_META_ESC;
		ScanCode ^= 0x80;
	}	
//kody escape
	if(kbd_Status & KBD_META_ESC){	
		kbd_Status &= ~KBD_META_ESC;
		ScanCode += 256;
	}
// kod break, które nas na razie interesuj¹ to Ctrl, Shift, Alt 
	if(kbd_Status & KBD_META_BRKCD)
	{
		if(ScanCode == RAW1_LEFT_ALT || ScanCode == RAW1_RIGHT_ALT)
			kbd_Status &= ~KBD_META_ALT;
		else if(ScanCode == RAW1_LEFT_CTRL || ScanCode == RAW1_RIGHT_CTRL)
			kbd_Status &= ~KBD_META_CTRL;
		else if(ScanCode == RAW1_LEFT_SHIFT || ScanCode == RAW1_RIGHT_SHIFT)
			kbd_Status &= ~KBD_META_SHIFT;
		
		kbd_Status &= ~KBD_META_BRKCD;		
		return -1;
	}
// jeœli to kod make: sprawdŸ klawisze "meta" podobnie jak powy¿ej 
	if(ScanCode == RAW1_LEFT_ALT || ScanCode == RAW1_RIGHT_ALT)	{
		kbd_Status |= KBD_META_ALT;
		return -1;
	}
	if(ScanCode == RAW1_LEFT_CTRL || ScanCode == RAW1_RIGHT_CTRL){
		kbd_Status |= KBD_META_CTRL;
		return -1;
	}
	if(ScanCode == RAW1_LEFT_SHIFT || ScanCode == RAW1_RIGHT_SHIFT){
		kbd_Status |= KBD_META_SHIFT;
		return -1;
	}
/* Scroll Lock, Num Lock, i Caps Lock ustawiaj¹ diody LED. */
	if(ScanCode == RAW1_SCROLL_LOCK){
		kbd_Status ^= KBD_META_SCRL;
		KBD_SetLeds();
		return -1;
	}
	if(ScanCode == RAW1_CAPS_LOCK){
		kbd_Status ^= KBD_META_CAPS;
		KBD_SetLeds();
		return -1;
	}	
	if(ScanCode == RAW1_NUM_LOCK){
		kbd_Status ^= KBD_META_NUM;
		KBD_SetLeds();
		return -1;
	}
/* konwertuj A-Z[\]^_ na kody sterowania */
	if(kbd_Status & KBD_META_CTRL)
	{
		if(ScanCode >= KeyMapLength)return -1;
		temp = KeyMap[0][ScanCode];
		if(temp >= 'a' && temp <= 'z')return temp - 'a';
		if(temp >= '[' && temp <= '_')return temp - '[' + 0x1B;
		return -1;
	}

/* ignoruj niepoprawne kody */
	if(ScanCode >= KeyMapLength) return -1;	 
//pobieramy nr pod strony kodowej zale¿y tylko od shift'a i alt'a
	uint8 CodeSubPage = kbd_Status & 0x3;
	
	temp = KeyMap[CodeSubPage][ScanCode];
	if(temp == 0) return -1;
	if(kbd_Status & KBD_META_CAPS){//jest capslock
		CodeSubPage ^= KBD_META_SHIFT; //negujemy shift'a
		if((CodeSubPage & KBD_META_SHIFT) && temp >= 'A' && temp <= 'Z')//by³ shift
				temp = KeyMap[CodeSubPage][ScanCode];			
		else // nie by³o shift'a
			if(temp >= 'a' && temp <= 'z')
				temp = KeyMap[CodeSubPage][ScanCode];			
	}
	return temp;
}

uint32 KBD_SendCommand(uint32 DevID, uint32 cmd, uint32 param){
	printf("KBD_SendCommand %d %d %d\n", DevID, cmd, param);
	return 0;
}

int main(/*int DID*/){
	while(1){};
	/*DriverID = DID;
	
	KeyMap[0] = (uint8*)KeyTable;
	KeyMap[1] = (uint8*)KeyTableShift;
	KeyMap[2] = (uint8*)KeyTable;
	KeyMap[3] = (uint8*)KeyTableShift;	
	
	RegDevice_t RegD;
	RegD.DevName = KBD_NAME;
	RegD.DevType = DEV_TYPE_KBD;
	RegD.SendCommand = &KBD_SendCommand;
	RegD.DeviceEntry = 0;	

	RegisterDevice(DriverID, &RegD);
	RegisterIRQ((uint32*)&KBDIrqCount, 1);
	KBDIrqCount = 0;

	message_t msg;
	while(1){
		Sleep(0);
		
		if(KBDIrqCount > 0){
			KBDIrqCount--;
			sint16 klawisz = inb(0x60);
			klawisz = TranslateScanCode(klawisz);
			if(klawisz > 0)SystemPushData(SYSTEMDATA_SCANCODE, klawisz);
		}
		
		if(GetMessage(&msg) == 0){
		}		
	}
	*/
	return 0;
}
