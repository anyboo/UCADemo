#ifndef __AlarmNotification_h__
#define __AlarmNotification_h__

typedef enum
{
	ePresence = 0,
	eEntered,
	eExited,
	eAppeared,
	eDisappeared,
	eStopped,
	eDwell,
	eDirection,
	eSpeed,
	eAbObj,
	eTailgating,
	eLineCounterA,
	eLineCounterB,
	eTamper,
	eRmObj,
	eColFilter,
	eSmoke,
	eFire,
//	eAutotracking,
	eUnknown,
	// yada yada
	eMaxNumAlarmTypes
}
AlarmType;

typedef enum
{
	eMaskEngineId	= 0x00000001,
	eMaskAlarmId	= 0x00000002,
	eMaskAlarmType	= 0x00000004,
	eMaskZoneName	= 0x00000008,
	eMaskZoneId		= 0x00000010,
	eMaskStartTime	= 0x00000020,
	eMaskStopTime	= 0x00000040,
	eMaskObjCls		= 0x00000080,
	eMaskAlarmName	= 0x00000100,
}
AlarmNotificationMask;

typedef struct
{
	DWORD		dwEngId;
	int			iAlarmId;
	AlarmType	eAlarmType;
	int			iZoneId;
	char		csZoneName[32];
	char		csObjectType[32];
	char		csRuleName[32];
	__int64		i64StartTime;
	__int64		i64StopTime;
	int			iReserved;
	int			iObjCls;
	int			fMask;
}
AlarmNotification;

#endif // __AlarmNotification_h__