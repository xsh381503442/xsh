#ifndef GEOMAG_H
#define GEOMAG_H

#include "WMM.h"
#include "stdint.h"

//	void CMD_init(){};
//    void CMD_deinit(){};

	char CMD_GeomagIntroduction(double epoch);
	int CMD_GetUserInput(WMMtype_CoordGeodetic *CoordGeodetic, WMMtype_Date *MagneticDate, double epoch);
	void CMD_PrintUserData(WMMtype_GeoMagneticElements GeomagElements, WMMtype_CoordGeodetic SpaceInput, WMMtype_Date TimeInput, double epoch, int SecularVariationUsed);
	void CMD_Error(int control);

//private:
	int CMD_ValidateDMSstringlat(char *input, char *Error);
	void CMD_DMSstringToDegree (char *DMSstring, double *DegreesOfArc);
	int CMD_Warnings(int control, double value, double epoch);
	int CMD_ValidateDMSstringlong(char *input, char *Error);
	int CMD_DateToYear (WMMtype_Date *CalendarDate, char *Error);
	void CMD_DegreeToDMSstring (double DegreesOfArc, int UnitDepth, char *DMSstring);

    double calc_geomag(uint32_t lon, uint32_t lat);

#endif
