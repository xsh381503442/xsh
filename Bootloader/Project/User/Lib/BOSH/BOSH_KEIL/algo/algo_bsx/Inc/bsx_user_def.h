#ifdef BOSH_KEIL_API 
/*
 * $license$
 */

/*!@addtogroup bsx_integrationsupport
 * @{*/

#ifndef __BSX_USER_DEF_H__
#define __BSX_USER_DEF_H__




/*!
 * @brief Contains the user specific definitions to be included with bsx_interface.h
 *
 * @note this shall typically be changeable and changed by a user, e.g. to include sensors.h from the Android headers
 */

#ifdef __cplusplus
extern "C"
{
#endif

//#include "sensors.h" // locate here e.g. sensors.h from Android

#ifndef BSX_TRACE_ENABLED
#define BSX_TRACE_ENTRY(object, event, id)
#else
#define BSX_TRACE_ENTRY(object, event, id)
#endif

#ifdef BSX_DEVELOPMENT
char_t bsx_artefact_identification;
#endif


#ifdef __cplusplus
}
#endif

#endif  /* __BSX_USER_DEF_H__ */
#endif //BOSH_KEIL_API
/*!@}*/
