
#ifndef bdldr_h
#define bdldr_h

typedef void * (*_CoreInit4)(char * src);
typedef void * (*_CoreNewInstance)();
typedef void * (*_CoreDeleteInstance)(void * instance);
typedef void * (*_CoreUninit)(void * core);
typedef void * (*_CoreGet)(void * instance, unsigned int cmd);
typedef void * (*_CoreSet)(void * instance, unsigned int cmd, void * dummy0, void * dummy1);

#define BD_CORE_PATH		"/opt/BitDefender-scanner/var/lib/scan/bdcore.so"
#define BD_PLUGINS_PATH		"/opt/BitDefender-scanner/var/lib/scan/"
#define BD_SCAN 		0x37
#define BD_ACTION 		0x1f
#define BD_HEURISTICS		0x21
#define BD_EXE_UNPACK		0x19
#define BD_ARCHIVE_UNPACK	0x1a
#define BD_EMAIL_UNPACK		0x1b

#define BD_ACTION_IGNORE	"ignore"

#define BD_ENABLE		(void *)0x1
#define BD_DISABLE		(void *)0x0

#endif
