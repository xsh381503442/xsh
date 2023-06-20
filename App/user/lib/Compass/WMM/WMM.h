#ifndef WMM_H
#define WMM_H

#define RAD2DEG(rad)    ((rad)*(180.0L/M_PI))
#define DEG2RAD(deg)    ((deg)*(M_PI/180.0L))
#define ATanH(x)	    (0.5 * log((1 + x) / (1 - x)))

#define WMM_MAX_MODEL_DEGREES	12
#define WMM_MAX_SECULAR_VARIATION_MODEL_DEGREES 12
#define NUMTERMS ( ( WMM_MAX_MODEL_DEGREES + 1 ) * ( WMM_MAX_MODEL_DEGREES + 2 ) / 2 )

#define WMM_PS_MIN_LAT_DEGREE  -55 /* Minimum Latitude for  Polar Stereographic projection in degrees   */
#define WMM_PS_MAX_LAT_DEGREE   55 /* Maximum Latitude for Polar Stereographic projection in degrees    */
#define WMM_UTM_MIN_LAT_DEGREE -80.5  /* Minimum Latitude for UTM projection in degrees   */
#define WMM_UTM_MAX_LAT_DEGREE  84.5  /* Maximum Latitude for UTM projection in degrees   */

#define WMM_GEO_POLE_TOLERANCE  1e-5
#define WMM_USE_GEOID	1    /* 1 Geoid - Ellipsoid difference should be corrected, 0 otherwise */

#define Schmidt 168

#define M_PI    3.141592653589793


#include "egm961500.h"

typedef struct {
			unsigned char GEOID;
			double lambda;// longitude
			double phi; // geodetic latitude
			double HeightAboveEllipsoid; // height above the ellipsoid (HaE)
			double HeightAboveGeoid;  /* (height above the EGM96 geoid model ) */
			} WMMtype_CoordGeodetic;

typedef struct {
			double EditionDate;
			double epoch;       //Base time of Geomagnetic model epoch (yrs)
			char  ModelName[20];
			float *Main_Field_Coeff_G;         // C - Gauss coefficients of main geomagnetic model (nT)
			float *Main_Field_Coeff_H;         // C - Gauss coefficients of main geomagnetic model (nT)
			float *Secular_Var_Coeff_G; // CD - Gauss coefficients of secular geomagnetic model (nT/yr)
			float *Secular_Var_Coeff_H; // CD - Gauss coefficients of secular geomagnetic model (nT/yr)
			int nMax; // Maximum degree of spherical harmonic model
			int nMaxSecVar;//Maxumum degree of spherical harmonic secular model
			int SecularVariationUsed; //Whether or not the magnetic secular variation vector will be needed by program
			} WMMtype_MagneticModel;

typedef struct {
			double a; /*semi-major axis of the ellipsoid*/
			double b; /*semi-minor axis of the ellipsoid*/
			double fla; /* flattening */
			double epssq; /*first eccentricity squared */
			double eps; /* first eccentricity */
			double re;/* mean radius of  ellipsoid*/
			} WMMtype_Ellipsoid;

typedef struct {
			double *Pcup;  /* Legendre Function */
			double *dPcup; /* Derivative of Lagendre fn */
				} WMMtype_LegendreFunction;

typedef struct {
			int NumbGeoidCols ;   /* 360 degrees of longitude at 15 minute spacing */
			int NumbGeoidRows ;   /* 180 degrees of latitude  at 15 minute spacing */
			int NumbHeaderItems ;    /* min, max lat, min, max long, lat, long spacing*/
			float *GeoidHeightBuffer;
			int Decimate;
			int NumbGeoidElevs;
			} WMMtype_Geoid;

typedef struct {
			double Decl; 	/* 1. Angle between the magnetic field vector and true north, positive east*/
			double Incl; 	/*2. Angle between the magnetic field vector and the horizontal plane, positive down*/
			double F; 		/*3. Magnetic Field Strength*/
			double H; 		/*4. Horizontal Magnetic Field Strength*/
			double X; 		/*5. Northern component of the magnetic field vector*/
			double Y; 		/*6. Eastern component of the magnetic field vector*/
			double Z; 		/*7. Downward component of the magnetic field vector*/
			double Decldot; /*9. Yearly Rate of change in declination*/
			double Incldot; /*10. Yearly Rate of change in inclination*/
			double Fdot; 	/*11. Yearly rate of change in Magnetic field strength*/
			double Hdot; 	/*12. Yearly rate of change in horizontal field strength*/
			double Xdot; 	/*13. Yearly rate of change in the northern component*/
			double Ydot; 	/*14. Yearly rate of change in the eastern component*/
			double Zdot; 	/*15. Yearly rate of change in the downward component*/
			} WMMtype_GeoMagneticElements;

typedef struct {
			double lambda;/* longitude*/
			double phig;/* geocentric latitude*/
			double r;  /* distance from the center of the ellipsoid*/
			} WMMtype_CoordSpherical;

typedef struct {
			int	Year;
			int	Month;
			int	Day;
			double DecimalYear;     /* decimal years */
			} WMMtype_Date;

typedef struct {
			double Bx;    /* North */
			double By;	  /* East */
			double Bz;    /* Down */
			} WMMtype_MagneticResults;

typedef struct {
			double RelativeRadiusPower[WMM_MAX_MODEL_DEGREES+1];  /* [earth_reference_radius_km / sph. radius ]^n  */
			double cos_mlambda[WMM_MAX_MODEL_DEGREES+1]; /*cp(m)  - cosine of (m*spherical coord. longitude)*/
			double sin_mlambda[WMM_MAX_MODEL_DEGREES+1]; /* sp(m)  - sine of (m*spherical coord. longitude) */
			}   WMMtype_SphericalHarmonicVariables;

typedef struct {
			char Longitude[40];
			char Latitude[40];
			} WMMtype_CoordGeodeticStr;

typedef struct 
{
//public:
//	WMM();
//   ~WMM();


//private:
	WMMtype_MagneticModel* MagneticModel;
	WMMtype_MagneticModel* TimedMagneticModel;
	WMMtype_Ellipsoid* Ellip;
	WMMtype_Geoid* Geoid;
	


}WMM_t;

void	WMM_init(WMM_t *wwm);
void	WMM_deinit(WMM_t *wwm);
void WMM_TimelyModifyMagneticModel(WMM_t *wmm, WMMtype_Date UserDate);
void WMM_GeodeticToSpherical(WMM_t *wmm, WMMtype_CoordGeodetic CoordGeodetic, WMMtype_CoordSpherical *CoordSpherical);
void WMM_Geomag(WMM_t *wmm,WMMtype_CoordSpherical CoordSpherical, WMMtype_CoordGeodetic CoordGeodetic, WMMtype_GeoMagneticElements  *GeoMagneticElements);
void WMM_ConvertGeoidToEllipsoidHeight (WMMtype_CoordGeodetic *CoordGeodetic);
//double WMM_GetEpoch(){return MagneticModel->epoch;}
//int WMM_GetSecularVariationUsed(){return MagneticModel->SecularVariationUsed;}

void WMM_readMagneticModel(WMM_t *wmm);
void WMM_InitializeGeoid(WMM_t *wmm);
void WMM_Summation(WMM_t *wmm, WMMtype_LegendreFunction *LegendreFunction, WMMtype_SphericalHarmonicVariables *SphVariables, WMMtype_CoordSpherical *CoordSpherical, WMMtype_MagneticResults *MagneticResults);
void WMM_SummationSpecial(WMM_t *wmm, WMMtype_SphericalHarmonicVariables *SphVariables, WMMtype_CoordSpherical *CoordSpherical, WMMtype_MagneticResults *MagneticResults);
void WMM_SecVarSummation(WMM_t *wmm, WMMtype_LegendreFunction *LegendreFunction, WMMtype_SphericalHarmonicVariables *SphVariables, WMMtype_CoordSpherical *CoordSpherical, WMMtype_MagneticResults *MagneticResults);
void WMM_SecVarSummationSpecial(WMM_t *wmm, WMMtype_SphericalHarmonicVariables *SphVariables, WMMtype_CoordSpherical *CoordSpherical, WMMtype_MagneticResults *MagneticResults);
void WMM_GetGeoidHeight (WMM_t *wmm, double Latitude, double Longitude,	double *DeltaHeight);
void WMM_ComputeSphericalHarmonicVariables(WMM_t *wmm, WMMtype_CoordSpherical CoordSpherical, int nMax, WMMtype_SphericalHarmonicVariables *SphVariables);
void WMM_AssociatedLegendreFunction(WMMtype_CoordSpherical CoordSpherical, int nMax, WMMtype_LegendreFunction *LegendreFunction);
void WMM_PcupLow( double *Pcup, double *dPcup, double x, int nMax);
void WMM_PcupHigh(double *Pcup, double *dPcup, double x, int nMax);
void WMM_RotateMagneticVector(WMMtype_CoordSpherical CoordSpherical, WMMtype_CoordGeodetic CoordGeodetic, WMMtype_MagneticResults MagneticResultsSph, WMMtype_MagneticResults *MagneticResultsGeo);
void WMM_CalculateGeoMagneticElements(WMMtype_MagneticResults *MagneticResultsGeo, WMMtype_GeoMagneticElements *GeoMagneticElements);
void WMM_CalculateSecularVariation(WMMtype_MagneticResults MagneticVariation, WMMtype_GeoMagneticElements *MagneticElements);

#endif
