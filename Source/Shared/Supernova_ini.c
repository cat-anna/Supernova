/*
	Supernova kernel shared library subfile
	This file can only by included by Supernova kernel library
	This file contain methods for ini files
*/

uint32 lib_ini_FastRead(const uint32* ini_string, const char* section, const char* ident, char* value, uint32 value_len){
	if(!ini_string || !ident || !value)return 1;
	if(!value_len)return 0;

	char *ini = (char*)ini_string;
	if(section)				//we don't request any section, that means ini don't have sections
		while(1){			//look for requested section
			char *s = strchr(ini, '[');
			if(!s) return 2;
			s++;
			if(!strcmp2delim(s, section, ']')){		//we have found requested section
				s = strchr(s, '\n');				//get the next line
				if(!s)return 3;
				ini = ++s;
				break;
			}
		}

	while(1){										//look for requested ident
		if(*ini == '\n')ini++;
		if(!strcmp2delim(ini, ident, '=')){			//we have found the requested ident
			ini = strchr(ini, '=');
			if(!ini)return 3;
			ini++;
			uint32 len = value_len;
			char* end = ini;
			for(;len && *end && *end != '\n';len--)*value++ = *end++;
			if(len > 0)*value = 0;
			return 0;
		}
	}

	return 3;
}
