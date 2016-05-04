// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the VLCEXTENDER_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// EXTENDER_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef EXTENDER_EXPORTS
#define EXTENDER_API __declspec(dllexport)
#else
#define EXTENDER_API __declspec(dllimport)
#endif



STDAPI DllGetClassObject( const IID &rclsid, const IID &riid, LPVOID *ppv );
STDAPI DllCanUnloadNow( );