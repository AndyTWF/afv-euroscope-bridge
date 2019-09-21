// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the AFVEUROSCOPEBRIDGE_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// AFVEUROSCOPEBRIDGE_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef AFVEUROSCOPEBRIDGE_EXPORTS
#define AFVEUROSCOPEBRIDGE_API __declspec(dllexport)
#else
#define AFVEUROSCOPEBRIDGE_API __declspec(dllimport)
#endif

// This class is exported from the dll
class AFVEUROSCOPEBRIDGE_API Cafveuroscopebridge {
public:
	Cafveuroscopebridge(void);
	// TODO: add your methods here.
};

extern AFVEUROSCOPEBRIDGE_API int nafveuroscopebridge;

AFVEUROSCOPEBRIDGE_API int fnafveuroscopebridge(void);
