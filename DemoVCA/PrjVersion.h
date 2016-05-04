#define VER_A	1	// Major
#define VER_B	4	// Minor
#define VER_C	2	// ?
#define VER_D	0	// ?

#define RC_VER_(A)					#A
#define RC_VER__(A)					RC_VER_(A)

#define APP_RC_VERSION				VER_A,VER_B,VER_C,VER_D
#if defined(_DEBUG)
  #define APP_RC_VERSION_STR		RC_VER__(VER_A) ", " RC_VER__(VER_B) ", " RC_VER__(VER_C) ", " RC_VER__(VER_D) " C\0"
#else
  #define APP_RC_VERSION_STR		RC_VER__(VER_A) ", " RC_VER__(VER_B) ", " RC_VER__(VER_C) ", " RC_VER__(VER_D) "\0"
#endif
#define APP_VERSION					((VER_A<<24)|(VER_B<<16)|(VER_C<<8)|(VER_D<<0))

