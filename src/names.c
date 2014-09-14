
#include<libimg.h>
#include<string.h>


struct name_struct {

	enum imgFormat fmt;
	const char * name;
};

#define ENTRY(nam) { IMG_FMT_ ## nam, #nam }

const struct name_struct name_struct[] = {

	ENTRY(UNKNOWN),

	ENTRY(RGB16),
	ENTRY(BGR16),

	ENTRY(RGB15),
	ENTRY(BGR15),

	ENTRY(RGB24),
	ENTRY(BGR24),

	/*** uncompressed formats with alpha channels ***/
	ENTRY(RGBA16),
	ENTRY(RGBA32),
	ENTRY(BGRA32),
	ENTRY(ARGB32),
	ENTRY(ABGR32),
	ENTRY(RGBA64),
	ENTRY(BGRA64),
	ENTRY(ARGB64),
	ENTRY(ABGR64),
	ENTRY(GREYA16),
	ENTRY(GREYA32),
	ENTRY(YUVA420P),

	/*** pre-multiplied alpha versions of above ***/
	ENTRY(RGBA16_PMA),
	ENTRY(RGBA32_PMA),
	ENTRY(BGRA32_PMA),
	ENTRY(ARGB32_PMA),
	ENTRY(ABGR32_PMA),
	ENTRY(RGBA64_PMA),
	ENTRY(BGRA64_PMA),
	ENTRY(ARGB64_PMA),
	ENTRY(ABGR64_PMA),
	ENTRY(GREYA16_PMA),
	ENTRY(GREYA32_PMA),
	ENTRY(YUVA420P_PMA),


	ENTRY(RGBX32),
	ENTRY(BGRX32),
	ENTRY(XRGB32),
	ENTRY(XBGR32),
	ENTRY(RGB48),
	ENTRY(BGR48),
	ENTRY(RGBX64),
	ENTRY(BGRX64),
	ENTRY(XRGB64),
	ENTRY(XBGR64),

	ENTRY(GREY8),
	ENTRY(GREY16),

	ENTRY(YUV420P),

	// compressed formats
	ENTRY(DXT1),
	ENTRY(DXT3),
	ENTRY(DXT5),
	ENTRY(ETC1),

	// compressed formats with pre-multiplied alpha
	ENTRY(DXT4),
	ENTRY(DXT2),

	// floating point image formats.
	ENTRY(FLOAT_RGB),
	ENTRY(FLOAT_RGBA),
	ENTRY(FLOAT_RGBX),
	ENTRY(FLOAT_ARGB),
	ENTRY(FLOAT_XRGB),
	ENTRY(FLOAT_BGR),
	ENTRY(FLOAT_BGRA),
	ENTRY(FLOAT_BGRX),
	ENTRY(FLOAT_ABGR),
	ENTRY(FLOAT_XBGR),
};

enum imgFormat imguGetFormatByName(const char * name) {

	int i;

	for(i=0; i< (sizeof(name_struct) / sizeof (name_struct[0])); i++ )
		if( strcasecmp(name_struct[i].name, name) == 0 )
			return name_struct[i].fmt;

	return IMG_FMT_UNKNOWN;
}

const char * imguGetFormatName(enum imgFormat fmt) {

	int i;

	for(i=0; i< (sizeof(name_struct) / sizeof (name_struct[0])); i++ )
		if( fmt == name_struct[i].fmt )
			return name_struct[i].name;

	return "UNKNOWN";
}

